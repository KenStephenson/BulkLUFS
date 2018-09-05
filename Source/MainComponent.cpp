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
		transportSource.addChangeListener(this);

		fileTotalLength = readerSource->getTotalLength();
		fileSampleRate = reader->sampleRate;
		bitsPerSample = reader->bitsPerSample;
		return true;
	}
	fileSampleRate = 0;
	bitsPerSample = 0;
	return false;
}
void MainComponent::WriteBufferToFile(AudioSampleBuffer* gainBuffer)
{

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
	//if(readerSource->getNextReadPosition() >= readerSource->getTotalLength())
	{
		runPostProcess();
		releaseResources();
	}
}
void MainComponent::releaseResources()
{
	transportSource.releaseResources();
	readerSource->releaseResources();
}
void MainComponent::runPostProcess()
{
	float fileDbLufs = ebuLoudnessMeter->getIntegratedLoudness();
	float gainDB = ((fileDbLufs * -1) - (dBLufsTarget * -1));
	float gainFactor = Decibels::decibelsToGain(gainDB);

	AudioSampleBuffer gainBuffer(2, fileTotalLength);
	AudioFormatReader* fmtReader = readerSource->getAudioFormatReader();
	fmtReader->read(&gainBuffer, 0, fileTotalLength, 0, true, true);
	gainBuffer.applyGain(gainFactor);

	File wavFile = destinationFolder.getChildFile(activeFile.getFileName());
	wavFile.deleteFile();

	FileOutputStream* fos = new FileOutputStream(wavFile);
	WavAudioFormat wavFormat;
	ScopedPointer<AudioFormatWriter> afw(wavFormat.createWriterFor(fos, fileSampleRate, gainBuffer.getNumChannels(), bitsPerSample, StringPairArray(), 0));
	afw->writeFromAudioSampleBuffer(gainBuffer, 0, gainBuffer.getNumSamples());
}
void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == &transportSource)
	{
		//transportSourceChanged();
	}
}
void MainComponent::transportSourceChanged()
{
	if (transportSource.isPlaying())
	{
		//transportStateChanged(Playing);
	}
	else
	{
		//transportStateChanged(Stopped);
	}
}
void MainComponent::transportStateChanged(TransportState newState)
{
	if (state != newState)
	{
		state = newState;
		switch (state)
		{
		case Stopped:
			//screen->stopButton.setEnabled(false);
			//screen->pauseButton.setEnabled(false);
			//screen->playButton.setEnabled(true);
			//transportSource.setPosition(0.0);
			break;
		case Starting:
			//screen->playButton.setEnabled(false);
			//transportSource.start();
			break;
		case Playing:
			//screen->stopButton.setEnabled(true);
			//screen->pauseButton.setEnabled(true);
			break;
		case Stopping:
			//transportSource.stop();
			break;
		case Paused:
			//screen->stopButton.setEnabled(false);
			//screen->pauseButton.setEnabled(false);
			//screen->playButton.setEnabled(true);
			break;
		case Pausing:
			//transportSource.stop();
			//isPaused = true;
			break;
		default:
			jassertfalse;
			break;
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
