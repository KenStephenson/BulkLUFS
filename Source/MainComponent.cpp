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

    // specify the number of input and output channels that we want to open
    setAudioChannels (2, 2);
	formatManager.registerBasicFormats();

	setSize(600, 600);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

#pragma region Audio Processing

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
		File activeFile = inputFiles[activeIndex];
		isInitialAnalysis = false;
		if (loadFileFromDisk(activeFile))
		{
			isInitialAnalysis = true;
			prepareToPlay((int)((double)fileSampleRate / (double)100), fileSampleRate);
		}
	}
}
bool MainComponent::loadFileFromDisk(File srcFile)
{
	if (auto* reader = formatManager.createReaderFor(srcFile))
	{
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		readerSource.reset(newSource.release());
		fileSampleRate = reader->sampleRate;
		bitsPerSample = reader->bitsPerSample;
		return true;
	}
	fileSampleRate = 0;
	bitsPerSample = 0;
	return false;
}
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	if (readerSource.get() != nullptr)
	{
		mainPanel.progressValue = 0;

		readerSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
		fileTotalLength = readerSource->getTotalLength();

		ebuLoudnessMeter.reset();
		ebuLoudnessMeter.prepareToPlay(sampleRate, 2, samplesPerBlockExpected, 20);
	}
}
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
	if (readerSource.get() == nullptr)
	{
		bufferToFill.clearActiveBufferRegion();
	}
	else
	{
		auto maxOutputChannels = 2;
		
		readerSource->getNextAudioBlock(bufferToFill);

		fileGetNextReadPosition = readerSource->getNextReadPosition();
		updateProgressPercentage();

		if (isInitialAnalysis)	
		{
			// calculate the integrated LUFS value of the file as it is loaded
			auto* inBuffer = bufferToFill.buffer->getArrayOfReadPointers();
			AudioSampleBuffer sBuffer;
			sBuffer.setDataToReferTo((float**)inBuffer, bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
			ebuLoudnessMeter.processBlock(sBuffer);
			if (fileGetNextReadPosition >= fileTotalLength)
			{
				fileDbLufs = ebuLoudnessMeter.getIntegratedLoudness();
				isInitialAnalysis = false;
			}
		}
		else
		{
			//auto* inBuffer = bufferToFill.buffer->getReadPointer(channel, bufferToFill.startSample);
			//auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
			//for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
			//{
			//	outBuffer[sample] = inBuffer[sample] * level;
			//}
		}
	}
}
void MainComponent::releaseResources()
{
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
	//if (leftPanel.lblDestFolder.getText() == leftPanel.tagNoDestinationFolder)
	//{
	//	AlertWindow dlg("Parameter Error", "No Output Folder", AlertWindow::AlertIconType::WarningIcon);
	//	dlg.addButton("OK", 1);
	//	dlg.setUsingNativeTitleBar(true);
	//	dlg.runModalLoop();
	//	return false;
	//}
	//if (destinationFolder == inputFolder)
	//{
	//	AlertWindow dlg("Parameter Error", "Input and Output folders are the same", AlertWindow::AlertIconType::WarningIcon);
	//	dlg.addButton("OK", 1);
	//	dlg.setUsingNativeTitleBar(true);
	//	dlg.runModalLoop();
	//	return false;
	//}
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
