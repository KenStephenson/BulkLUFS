/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./View/Screen.h"
#include "./View/FileListBoxModel.h"
#include "./OfflineLoudnessProcessor/OfflineLoudnessScanDataPacket.h"
#include "./OfflineLoudnessProcessor/OfflineLoudnessScanManager.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MainComponent : public Component, public ListBoxModelListener, public ScanListener
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
		std::unique_ptr<OfflineLoudnessScanManager> scanMgr = nullptr;
		OfflineLoudnessScanDataPacket* activeScanItem;
		
		float dBLufsTarget;
		float dBLimiterCeiling;
		bool writeFile = false;
		int activeScanIndex;

		bool validateProcessorParameters();
		void runProcess();
		void startNextLoudnessScan();
		void ScanCompleted() override;
		#pragma endregion

		#pragma region User Interface Parameters
		ControlsPanel topPanel;
		InputPanel fileTable;
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
