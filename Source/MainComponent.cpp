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

	ebuLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
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
	int samplesPerBlock = (int)((double)fileSampleRate / (double)100);

	//if (leftLevelDetector == nullptr && rightLevelDetector == nullptr)
	//{
	//	leftLevelDetector = new PeakLevelDetector(fileSampleRate);
	//	rightLevelDetector = new PeakLevelDetector(fileSampleRate);
	//}
	//else
	//{
	//	leftLevelDetector->setDetector(fileSampleRate);
	//	rightLevelDetector->setDetector(fileSampleRate);
	//}

	//if (gainDymanics == nullptr) 
	//{
	//	gainDymanics = new GainDynamics(sampleRate, attackTime, releaseTime);
	//}
	//else 
	//{
	//	gainDymanics->setDetector(sampleRate);
	//}

	ebuLoudnessMeter->reset();
	ebuLoudnessMeter->prepareToPlay(fileSampleRate, 2, samplesPerBlock, 20);
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
	ebuLoudnessMeter->processBlock(sBuffer);
	
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
	//applyBrickwallLimiter(audioBuffer.get());
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

#define dB(x) 20.0 * ((x) > 0.00001 ? log10(x) : -5.0)
#define dB2mag(x) pow(10.0, (x) / 20.0)
void MainComponent::applyBrickwallLimiter(AudioSampleBuffer* audioBuffer)
{
	float* leftChannelData = audioBuffer->getWritePointer(0);
	float* rightChannelData = audioBuffer->getWritePointer(1);
	for (int i = 0; i < audioBuffer->getNumSamples(); i++)
	{
		// Peak detector
		peakOutL = leftLevelDetector->tick(leftChannelData[i]);
		peakOutR = rightLevelDetector->tick(rightChannelData[i]);
		peakSum = (peakOutL + peakOutR) * 0.5f;

		// Convert to db
		peakSumDb = dB(peakSum);

		// Calculate gain
		if (peakSumDb < thresholdDb)
		{
			gainDb = 0.f;
		}
		else
		{
			gainDb = -(peakSumDb - thresholdDb) * (1.f - 1.f / aRatio);
		}

		// Gain dynamics (attack and release)
		gainDb = gainDymanics->tick(gainDb);

		// Convert to Linear
		gain = dB2mag(gainDb);

		// Apply gain
		leftChannelData[i] *= gain;
		rightChannelData[i] *= gain;
	}
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
				pluginInstance->setNonRealtime(true);
				limiterPlugin = std::make_unique<PluginWrapperProcessor>(pluginInstance);
				
				int numParams = pluginInstance->getNumParameters();
				for (int parameterIndex = 0; parameterIndex < numParams; parameterIndex++)
				{
					auto p = limiterPlugin.get()->getParameter(parameterIndex);
					int bp = 0;

					//String pName = limiterPlugin.get()->getParameterName(parameterIndex);
					//	String pID = limiterPlugin.get()->getParameterID(parameterIndex);
					//	float pIndex = limiterPlugin.get()->getParameter(parameterIndex);
					//	String pText = limiterPlugin.get()->getParameterText(parameterIndex);
					//	int pNumSteps = limiterPlugin.get()->getParameterNumSteps(parameterIndex);
					//	float pDefVal = limiterPlugin.get()->getParameterDefaultValue(parameterIndex);
					//	String pLabel = limiterPlugin.get()->getParameterLabel(parameterIndex);

				}
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
