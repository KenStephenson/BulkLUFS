/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./View/Screen.h"
#include "./Model/SessionModel.h"
#include "./Model/TrackModel.h"
#include "./OfflineLoudnessProcessor/OfflineLoudnessScanThread.h"
#include "./VstHost/PluginListManager.h"

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
		AppLookAndFeel appLookAndFeel;

		const String limiterPluginName = "George Yohng's W1 Limiter";
		const String limiterPluginName64 = "George Yohng's W1 Limiter x64";
		void loadLimiterPlugin();
		std::unique_ptr<AudioProcessor> limiterPlugin;

		const String tagInputList = "INPUT";
		std::unique_ptr<SessionModel> filesToProcesstListModel = nullptr;
		std::unique_ptr<OfflineLoudnessScanThread> offlineLoudnessScanThread = nullptr;
		std::shared_ptr<TrackModel> activeOfflineLoudnessScanItem;
		float dBLufsTarget = -14.0f;
		bool writeFile = false;
		int activeScanIndex = 0;

		bool validateProcessorParameters();
		void runProcess();
		void stopProcess();
		void startNextLoudnessScan();
		void scanCompleted() override;
		#pragma endregion

		#pragma region User Interface Parameters
		HeaderPanel headerPanel;
		ControlsPanel controlPanel;
		FileListPanel fileTablePanel;

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
		void setPeakLimiterClicked();
		#pragma endregion

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
