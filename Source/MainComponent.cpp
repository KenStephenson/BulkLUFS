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

//==============================================================================
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

//==============================================================================
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
}

void MainComponent::addFilesButtonClicked()
{
}

void MainComponent::destinationFolderButtonClicked()
{
}
