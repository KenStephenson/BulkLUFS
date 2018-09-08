/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	initialiseUserInterface();

	ebuLoudnessMeter = std::make_unique<LUFSMeterAudioProcessor>();
	ebuLoudnessMeter.get()->setNonRealtime(true);

	setAudioChannels (2, 2);
	formatManager.registerBasicFormats();

	loadLimiterPlugin();

	setSize(600, 600);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

#pragma region Initialise Process
void MainComponent::runProcess()
{
	if (validateProcessorParameters() == false)
	{
		return;
	}

	// Collect the Process Parameters
	dBLufsTarget = mainPanel.sldLUFSTarget.getValue();
	dBLimiterCeiling = mainPanel.sldLimiterCeiling.getValue();
	inputFiles = inputListModel.data;
	outputFiles.clear();

	for (activeIndex = 0; activeIndex < inputFiles.size(); activeIndex++)
	{
		activeFile = inputFiles[activeIndex];
		if (loadFileFromDisk(activeFile))
		{
			prepareToPlay((int)((double)fileSampleRate / (double)100), fileSampleRate);
		}
	}
}
bool MainComponent::loadFileFromDisk(File srcFile)
{
	if (auto* reader = formatManager.createReaderFor(srcFile))
	{
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
		readerSource.reset(newSource.release());
		readerSource->setLooping(false);

		fileTotalLength = readerSource->getTotalLength();
		fileSampleRate = reader->sampleRate;
		bitsPerSample = reader->bitsPerSample;
		return true;
	}
	fileSampleRate = 0;
	bitsPerSample = 0;
	return false;
}
#pragma endregion

#pragma region Audio Processing
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	if (readerSource.get() == nullptr)
	{
		return;
	}
	leftPanel.setEnabled(false);
	mainPanel.setEnabled(false);
	mainPanel.progressValue = 0;
	fileGetNextReadPosition = 0;
	int samplesPerBlock = (int)((double)sampleRate / (double)100);

	ebuLoudnessMeter->reset();
	ebuLoudnessMeter->prepareToPlay(sampleRate, samplesPerBlock);
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
	readerSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
	transportSource.start();
}
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
	if (readerSource.get() == nullptr || transportSource.isPlaying() == false)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}
		
	transportSource.getNextAudioBlock(bufferToFill);
	fileGetNextReadPosition = readerSource->getNextReadPosition();
	updateProgressPercentage();

	auto* inBuffer = bufferToFill.buffer->getArrayOfReadPointers();
	AudioSampleBuffer sBuffer;
	sBuffer.setDataToReferTo((float**)inBuffer, bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
	ebuLoudnessMeter->processBlock(sBuffer, midiBuffer);
	
	if(transportSource.isPlaying() == false)
	{
		runPostProcess();
		releaseResources();
	}
}
void MainComponent::releaseResources()
{
	transportSource.releaseResources();
}
void MainComponent::runPostProcess()
{
	std::unique_ptr<AudioSampleBuffer> audioBuffer = std::make_unique<AudioSampleBuffer>(2, fileTotalLength);
	applyGain(audioBuffer.get());
	applyBrickwallLimiter(audioBuffer.get());
	writeOutputFile(audioBuffer.get());
}

void MainComponent::applyGain(AudioSampleBuffer* audioBuffer)
{
	float fileDbLufs = ebuLoudnessMeter->getIntegratedLoudness();
	float dbDifference = (fileDbLufs * -1) - (dBLufsTarget * -1);
	float gainFactor = Decibels::decibelsToGain(dbDifference);

	AudioFormatReader* fmtReader = readerSource->getAudioFormatReader();
	fmtReader->read(audioBuffer, 0, fileTotalLength, 0, true, true);
	audioBuffer->applyGain(gainFactor);
}
void MainComponent::applyBrickwallLimiter(AudioSampleBuffer* audioBuffer)
{
	int numSamples = audioBuffer->getNumSamples();
	float initialCeiling = dBLimiterCeiling;
	float initialMax = audioBuffer->getMagnitude(0, numSamples);
	
	float db_1 = Decibels::decibelsToGain(-1.0f);
	float dbFS = Decibels::decibelsToGain(initialCeiling);

	limiterPlugin->setNonRealtime(true);
	limiterPlugin->setParameter(0, dbFS);	// Threshold
	limiterPlugin->setParameter(1, dbFS);	// Ceiling
	limiterPlugin->setParameter(2, 200.0f);	// Release
	limiterPlugin->setParameter(3, false);	// Auto Release

	limiterPlugin->prepareToPlay(fileSampleRate, numSamples);
	limiterPlugin.get()->processBlock(*audioBuffer, midiBuffer);
	
	float max = audioBuffer->getMagnitude(0, numSamples);

}

void MainComponent::writeOutputFile(AudioSampleBuffer* audioBuffer)
{
	File wavFile = destinationFolder.getChildFile(activeFile.getFileName());
	wavFile.deleteFile();

	FileOutputStream* fos = new FileOutputStream(wavFile);
	WavAudioFormat wavFormat;
	ScopedPointer<AudioFormatWriter> afw(wavFormat.createWriterFor(fos, fileSampleRate, audioBuffer->getNumChannels(), bitsPerSample, StringPairArray(), 0));
	afw->writeFromAudioSampleBuffer(*audioBuffer, 0, audioBuffer->getNumSamples());
}

void MainComponent::loadLimiterPlugin()
{
	KnownPluginList knownPluginList;
	std::unique_ptr<AudioPluginFormat> format = std::make_unique<VSTPluginFormat>();
	FileSearchPath path = format->getDefaultLocationsToSearch();
	
	// Scan the directory for plugins
	std::unique_ptr<PluginDirectoryScanner> scanner = std::make_unique<PluginDirectoryScanner>(knownPluginList, *format, path,	true, File::nonexistent, false);
	String currentPlugBeingScanned;
	while (scanner->scanNextFile(true, currentPlugBeingScanned)) {}

	for (int i = 0; i < knownPluginList.getNumTypes(); i++)
	{
		PluginDescription* plugIn = knownPluginList.getType(i);
		if (plugIn->name == limiterPluginName)
		{
			AudioPluginFormatManager fm;
			fm.addDefaultFormats();

			String ignore;
			if (AudioPluginInstance* pluginInstance = fm.createPluginInstance(*plugIn, 44100.0, 512, ignore))
			{
				limiterPlugin = std::make_unique<PluginWrapperProcessor>(pluginInstance);
			}
		}
	}
}

#pragma endregion


#pragma region User Interface
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(Colours::lightgrey);

    // You can add your drawing code here!
	g.setColour(Colours::black);
	g.drawRect(leftPanel.getLocalBounds());
	g.drawRect(mainPanel.getLocalBounds());
}
void MainComponent::resized()
{
	Grid grid;
	using Track = Grid::TrackInfo;

	grid.templateRows = { Track(1_fr) };
	grid.templateColumns = { Track(1_fr), Track(1_fr) };
	grid.items = { GridItem(leftPanel), GridItem(mainPanel), };
	grid.performLayout(getLocalBounds());
}
void MainComponent::initialiseUserInterface()
{
	addAndMakeVisible(leftPanel);
	addAndMakeVisible(mainPanel);

	leftPanel.btnAddFiles.onClick = [this] { addFilesButtonClicked(); };
	leftPanel.btnDestFolder.onClick = [this] { destinationFolderButtonClicked(); };
	mainPanel.btnRunProcess.onClick = [this] { runProcessButtonClicked(); };

	inputListModel.setListener(this, tagInputList);
	outputListModel.setListener(this, tagOutputList);

	leftPanel.listInputFiles.setModel(&inputListModel);
	mainPanel.listOutputFiles.setModel(&outputListModel);
}
void MainComponent::ModelRefresh(String tag)
{
	if (tag == tagInputList)
	{
		leftPanel.listInputFiles.updateContent();
	}
};
void MainComponent::addFilesButtonClicked()
{
	FileChooser chooser("Select files to process...", File::nonexistent, "*.wav");
	if (chooser.browseForMultipleFilesToOpen())
	{
		inputListModel.data.addArray(chooser.getResults());
		inputFolder = inputListModel.data[0].getParentDirectory();
	}
	leftPanel.listInputFiles.updateContent();
}
void MainComponent::destinationFolderButtonClicked()
{
	FileChooser chooser("Select output folder...", inputFolder, "*.*");
	if (chooser.browseForDirectory())
	{
		destinationFolder = chooser.getResult();
		leftPanel.lblDestFolder.setText(destinationFolder.getFileNameWithoutExtension(), dontSendNotification);
	}
	else
	{
		leftPanel.lblDestFolder.setText(leftPanel.tagNoDestinationFolder, dontSendNotification);
	}
}
void MainComponent::runProcessButtonClicked()
{
	runProcess();
}
bool MainComponent::validateProcessorParameters()
{
	if (inputListModel.getNumRows() == 0)
	{
		AlertWindow dlg("Parameter Error", "No Input Files", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return false;
	}
	if (leftPanel.lblDestFolder.getText() == leftPanel.tagNoDestinationFolder)
	{
		AlertWindow dlg("Parameter Error", "No Output Folder", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return false;
	}
	if (destinationFolder == inputFolder)
	{
		AlertWindow dlg("Parameter Error", "Input and Output folders are the same", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return false;
	}
	return true;
}
void MainComponent::updateProgressPercentage()
{
	double percent = 0;
	if (fileTotalLength > 0)
	{
		percent = ((double)fileGetNextReadPosition / (double)fileTotalLength);
	}
	mainPanel.progressValue = percent;
}
#pragma endregion
