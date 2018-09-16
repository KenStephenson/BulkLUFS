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
#include "./OfflineLoudnessProcessor/OfflineLoudnessScanThread.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MainComponent : public Component, public ListBoxModelListener, public OfflineLoudnessScanListener
{
	public:
		using Track = Grid::TrackInfo;

		MainComponent();
		~MainComponent();

		void paint (Graphics& g) override;
		void resized() override;

		void closeApp();

	private:
		#pragma region Process Methods and Parameters
		const String tagInputList = "INPUT";
		std::unique_ptr<FileListBoxModel> filesToProcesstListModel = nullptr;
		std::unique_ptr<OfflineLoudnessScanThread> offlineLoudnessScanThread = nullptr;
		std::shared_ptr<OfflineLoudnessScanDataPacket> activeOfflineLoudnessScanItem;
		float dBLufsTarget;
		float dBLimiterCeiling;
		bool writeFile = false;
		int activeScanIndex;

		bool validateProcessorParameters();
		void runProcess();
		void stopProcess();
		void startNextLoudnessScan();
		void ScanCompleted() override;
		#pragma endregion

		#pragma region User Interface Parameters
		ControlsPanel topPanel;
		FileListPanel fileTablePanel;
		FooterPanel footerPanel;
		File inputFolder;
		File destinationFolder;
		bool cancelRequest;
		void initialiseUserInterface();
		void updateProgressPercentage();
		void addFilesButtonClicked();
		void destinationFolderButtonClicked();
		void runProcessButtonClicked();
		void clearFilesButtonClicked();
		void resetFilesButtonClicked();
		#pragma endregion

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
