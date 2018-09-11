/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./View/Screen.h"
#include "./View/FileListBoxModel.h"
#include "./OfflineLoudnessProcessor/OfflineLoudnessProcessor.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MainComponent : public Component, public ListBoxModelListener, public Thread::Listener
{
	public:
		using Track = Grid::TrackInfo;

		MainComponent();
		~MainComponent();

		void paint (Graphics& g) override;
		void resized() override;

		void refreshFileTableModel(String tag) override;

	private:
#pragma region Process Methods and Parameters
		const String tagInputList = "INPUT";
		std::unique_ptr<FileListBoxModel> inputListModel = nullptr;

		std::unique_ptr<OfflineLoudnessProcessor> scanThread = nullptr;
		int activeIndex;
		FileLoudnessDetails* activeFileDetail;
		float dBLufsTarget;
		float dBLimiterCeiling;
		bool writeFile = false;

		bool validateProcessorParameters();
		void runProcess();
		void startProcess();
		void exitSignalSent() override;
#pragma endregion

#pragma region User Interface Parameters
		InputPanel leftPanel;
		ControlsPanel mainPanel;
		File inputFolder;
		File destinationFolder;


		void initialiseUserInterface();
		void addFilesButtonClicked();
		void destinationFolderButtonClicked();
		void runProcessButtonClicked();
		void updateProgressPercentage();
#pragma endregion

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
