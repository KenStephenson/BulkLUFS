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

	setSize(600, 600);

    // specify the number of input and output channels that we want to open
    setAudioChannels (2, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

#pragma region Audio Processing
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}
void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
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
	leftPanel.listInputFiles.setModel(&inputListModel);
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
		inputListModel.data = chooser.getResults();
		inputFolder = inputListModel.data[0].getParentDirectory();
	}
	else
	{
		inputListModel.data.clear();
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
	if (inputListModel.getNumRows() == 0)
	{
		AlertWindow dlg("Parameter Error", "No Input Files", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return;
	}
	if (leftPanel.lblDestFolder.getText() == leftPanel.tagNoDestinationFolder)
	{
		AlertWindow dlg("Parameter Error", "No Output Folder", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return;
	}
	if (destinationFolder == inputFolder)
	{
		AlertWindow dlg("Parameter Error", "Input and Output folders are the same", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return;
	}
	processParams.inputFiles = inputListModel.data;
	processParams.destinationFolder = destinationFolder;
	processParams.dBLufsTarget = mainPanel.sldLUFSTarget.getValue();
	processParams.dBLimiterCeiling = mainPanel.sldLimiterCeiling.getValue();
}
#pragma endregion
