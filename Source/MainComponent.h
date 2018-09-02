/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./View/Screen.h"
#include "./View/FileListBoxModel.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class ProcessParameters
{
	public:
		Array<File> inputFiles;
		Array<File> outputFiles;
		File destinationFolder;
		float dBLufsTarget;
		float dBLimiterCeiling;
};

class MainComponent   : public AudioAppComponent, public ListBoxModelListener
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

		void ModelRefresh(String tag) override;

	private:

		void initialiseUserInterface();

		void addFilesButtonClicked();
		void destinationFolderButtonClicked();
		void runProcessButtonClicked();
		bool validateProcessorParameters();

		InputPanel leftPanel;
		ControlsPanel mainPanel;
		File inputFolder;
		File destinationFolder;

		FileListBoxModel inputListModel;
		FileListBoxModel outputListModel;
		ProcessParameters processParams;

		const String tagInputList = "INPUT";
		const String tagOutputList = "OUTPUT";

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
