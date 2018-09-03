/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./View/Screen.h"
#include "./View/FileListBoxModel.h"
#include "./Ebu128Loudness/Ebu128LoudnessMeter.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public AudioAppComponent, public ListBoxModelListener
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
		// User Interface Parameters
		InputPanel leftPanel;
		ControlsPanel mainPanel;
		File inputFolder;
		File destinationFolder;
		FileListBoxModel inputListModel;
		FileListBoxModel outputListModel;
		const String tagInputList = "INPUT";
		const String tagOutputList = "OUTPUT";

		void initialiseUserInterface();
		void addFilesButtonClicked();
		void destinationFolderButtonClicked();
		void runProcessButtonClicked();
		void updateProgressPercentage();


		// Loudness Processing Parameters
		int activeIndex;
		float dBLufsTarget;
		float dBLimiterCeiling;
		double fileSampleRate;
		double bitsPerSample;
		Array<File> inputFiles;
		Array<File> outputFiles;

		AudioFormatManager formatManager;
		std::unique_ptr<AudioFormatReaderSource> readerSource;
		Ebu128LoudnessMeter ebuLoudnessMeter;
		int64 fileTotalLength;
		int64 fileGetNextReadPosition;
		float fileDbLufs;

		bool isInitialAnalysis;
		void runProcess();
		bool validateProcessorParameters();
		bool loadFileFromDisk(File srcFile);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
