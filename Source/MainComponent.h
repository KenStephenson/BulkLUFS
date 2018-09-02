/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./View/Screen.h"
#include "./View/InputFileListBoxModel.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MainComponent   : public AudioAppComponent
{
	public:
		using Track = Grid::TrackInfo;
	
		//==============================================================================
		MainComponent();
		~MainComponent();

		//==============================================================================
		void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
		void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
		void releaseResources() override;

		//==============================================================================
		void paint (Graphics& g) override;
		void resized() override;

	private:

		void initialiseUserInterface();

		void addFilesButtonClicked();
		void destinationFolderButtonClicked();
		void runProcessButtonClicked();

		InputPanel leftPanel;
		ControlsPanel mainPanel;
		InputFileListBoxModel inputListModel;
		File inputFolder;
		File destinationFolder;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
