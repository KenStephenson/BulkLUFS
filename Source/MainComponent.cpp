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

	loadLimiterPlugin();

	preProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	formatManager.registerBasicFormats();

	setSize(1000, 600);
}

MainComponent::~MainComponent()
{
}

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
	grid.templateColumns = { Track(3_fr), Track(1_fr) };
	grid.items = { GridItem(leftPanel), GridItem(mainPanel), };
	grid.performLayout(getLocalBounds());
}
void MainComponent::initialiseUserInterface()
{
	inputListModel = std::make_unique<FileListBoxModel>();

	addAndMakeVisible(leftPanel);
	addAndMakeVisible(mainPanel);

	leftPanel.btnAddFiles.onClick = [this] { addFilesButtonClicked(); };
	mainPanel.btnDestFolder.onClick = [this] { destinationFolderButtonClicked(); };
	mainPanel.btnRunProcess.onClick = [this] { runProcessButtonClicked(); };

	inputListModel.get()->setListener(this, tagInputList);
	leftPanel.listInputFiles.setModel(inputListModel.get());
}
void MainComponent::refreshFileTableModel(String tag)
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
		for (int i = 0; i < chooser.getResults().size(); i++)
		{
			File f = chooser.getResults()[i];
			inputListModel->data.add(new FileLoudnessDetails(i, f));
		}
		inputFolder = inputListModel->data[0]->file.getParentDirectory();
	}
	leftPanel.listInputFiles.updateContent();
}
void MainComponent::destinationFolderButtonClicked()
{
	FileChooser chooser("Select output folder...", inputFolder, "*.*");
	if (chooser.browseForDirectory())
	{
		destinationFolder = chooser.getResult();
		mainPanel.lblDestFolder.setText(destinationFolder.getFileNameWithoutExtension(), dontSendNotification);
	}
	else
	{
		mainPanel.lblDestFolder.setText(mainPanel.tagNoDestinationFolder, dontSendNotification);
	}
}
void MainComponent::runProcessButtonClicked()
{
	runProcess();

	leftPanel.listInputFiles.updateContent();
}
bool MainComponent::validateProcessorParameters()
{
	if (inputListModel->getNumRows() == 0)
	{
		AlertWindow dlg("Parameter Error", "No Input Files", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return false;
	}
	
	writeFile = mainPanel.lblDestFolder.getText() != mainPanel.tagNoDestinationFolder ? true : false;
	if (writeFile && destinationFolder == inputFolder)
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
	if (inputListModel->getNumRows() > 0)
	{
		percent = ((double)activeIndex / (double)inputListModel->getNumRows());
	}
	mainPanel.progressValue = percent;
}
#pragma endregion

#pragma region Run Batch Audio Processing
void MainComponent::runProcess()
{
	if (validateProcessorParameters() == false)
	{
		return;
	}

	// Collect the Process Parameters
	dBLufsTarget = mainPanel.sldLUFSTarget.getValue();
	dBLimiterCeiling = mainPanel.sldLimiterCeiling.getValue();

	preProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	postProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	timer = std::make_unique<PulseTimer>(this);

	activeIndex = 0;
	processNextFile();
}
void MainComponent::processNextFile()
{
	if (activeIndex >= inputListModel->data.size())
	{
		// completed
		updateProgressPercentage();
		return;
	}
	activeFile = inputListModel->data[activeIndex];
	updateProgressPercentage();

	if (loadFileFromDisk(activeFile->file))
	{
		AudioFormatReader* fmtReader = readerSource->getAudioFormatReader();
		numSamples = fmtReader->lengthInSamples;
		numChannels = fmtReader->numChannels;

		audioBuffer = std::make_unique<AudioSampleBuffer>(numChannels, numSamples);
		fmtReader->read(audioBuffer.get(), 0, numSamples, 0, true, true);

		expectedRequestRate = 10;
		samplesPerBlock = fmtReader->sampleRate;

		preProcessLoudnessMeter->reset();
		preProcessLoudnessMeter->prepareToPlay(numSamples, 2, samplesPerBlock, expectedRequestRate);

		postProcessLoudnessMeter->reset();
		postProcessLoudnessMeter->prepareToPlay(numSamples, 2, samplesPerBlock, expectedRequestRate);

		isPostProcess = false;
		bufferPointer = 0;
		timer.get()->startTimerHz(1000.0f);
	}
}

void MainComponent::handleTimerTick()
{
	if (bufferPointer > numSamples - samplesPerBlock)
	{
		timer->stopTimer();
		bufferPointer = 0;

		analyseBufferLoudness(numSamples - bufferPointer);
		
		loudnessScanComplete();
	}
	else
	{
		analyseBufferLoudness(samplesPerBlock);
		bufferPointer += samplesPerBlock;
	}
}

void MainComponent::analyseBufferLoudness(int bufferSize)
{
	AudioSampleBuffer workBuffer(numChannels, bufferSize);
	workBuffer.setDataToReferTo((float**)audioBuffer->getArrayOfReadPointers(), numChannels, bufferPointer, bufferSize);
	if (isPostProcess)
	{
		postProcessLoudnessMeter->processBlock(workBuffer);
	}
	else
	{
		preProcessLoudnessMeter->processBlock(workBuffer);
	}
}

void MainComponent::loudnessScanComplete()
{
	if (isPostProcess == false)
	{
		// This called after the Initial Loudness scan
		// Start the Post Processing
		// the Post Process Loudness scan will restart the timer and will finalise in this runPostProcessing
		isPostProcess = true;

		activeFile->preIntegratedLufs = preProcessLoudnessMeter->getIntegratedLoudness();
		activeFile->prePeakDbfs = Decibels::gainToDecibels(audioBuffer->getMagnitude(0, numSamples));

		applyGain();
		applyBrickwallLimiter();
		readPostProcessLoudness(); 
	}
	else
	{
		// This called after the Post Processing has been done
		// Finalise and call the next file
		activeFile->postIntegratedLufs = postProcessLoudnessMeter->getIntegratedLoudness();
		activeFile->postPeakDbfs = Decibels::gainToDecibels(audioBuffer->getMagnitude(0, numSamples));

		if (writeFile)
		{
			writeOutputFile();
		}
		isPostProcess = false;

		activeIndex++;
		processNextFile();
	}
}
void MainComponent::applyGain()
{
	float fileDbLufs = preProcessLoudnessMeter->getIntegratedLoudness();
	float dbDifference = (fileDbLufs * -1) - (dBLufsTarget * -1);
	float gainFactor = Decibels::decibelsToGain(dbDifference);
	activeFile->diffLufs = dbDifference;
	activeFile->gain = gainFactor;
	audioBuffer->applyGain(gainFactor);
}
void MainComponent::applyBrickwallLimiter()
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
	limiterPlugin->processBlock(*audioBuffer, midiBuffer);

	float max = audioBuffer->getMagnitude(0, numSamples);
}
void MainComponent::readPostProcessLoudness()
{
	bufferPointer = 0;
	timer.get()->startTimerHz(1000.0f);
}
void MainComponent::writeOutputFile()
{
	File wavFile = destinationFolder.getChildFile(activeFile->file.getFileName());
	wavFile.deleteFile();

	FileOutputStream* fos = new FileOutputStream(wavFile);
	WavAudioFormat wavFormat;
	ScopedPointer<AudioFormatWriter> afw(wavFormat.createWriterFor(fos, fileSampleRate, audioBuffer->getNumChannels(), fileBitsPerSample, StringPairArray(), 0));
	afw->writeFromAudioSampleBuffer(*audioBuffer, 0, audioBuffer->getNumSamples());
}

#pragma region Load Resources
bool MainComponent::loadFileFromDisk(File srcFile)
{
	if (auto* reader = formatManager.createReaderFor(srcFile))
	{
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		readerSource.reset(newSource.release());
		readerSource->setLooping(false);

		fileSampleRate = reader->sampleRate;
		fileBitsPerSample = reader->fileBitsPerSample;
		return true;
	}
	fileSampleRate = 0;
	fileBitsPerSample = 0;
	return false;
}
void MainComponent::loadLimiterPlugin()
{
	KnownPluginList knownPluginList;
	std::unique_ptr<AudioPluginFormat> format = std::make_unique<VSTPluginFormat>();
	FileSearchPath path = format->getDefaultLocationsToSearch();

	// Scan the directory for plugins
	std::unique_ptr<PluginDirectoryScanner> scanner = std::make_unique<PluginDirectoryScanner>(knownPluginList, *format, path, true, File::nonexistent, false);
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
				limiterPlugin = std::make_shared<PluginWrapperProcessor>(pluginInstance);
			}
		}
	}
}
#pragma endregion

#pragma endregion

