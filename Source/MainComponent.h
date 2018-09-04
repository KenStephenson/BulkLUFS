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
#include "./GainProcessor/GainProcessor.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public AudioAppComponent, public ListBoxModelListener
{
	public:
		//==============================================================================
		using Track = Grid::TrackInfo;
		using AudioGraphIOProcessor = AudioProcessorGraph::AudioGraphIOProcessor;
		using Node = AudioProcessorGraph::Node;

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
#pragma region User Interface Parameters
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
#pragma endregion

#pragma region Process Parameters
		enum PassID
		{
			ebuLoudness = 0,
			gain = 1,
			limiter = 2,
		};

		int activeIndex;
		float dBLufsTarget;
		float dBLimiterCeiling;
		double fileSampleRate;
		double bitsPerSample;
		Array<File> inputFiles;
		Array<File> outputFiles;
		int64 fileTotalLength;
		int64 fileGetNextReadPosition;
		float fileDbLufs;
		PassID passID;

		void runProcess();

		bool validateProcessorParameters();
		bool loadFileFromDisk(File srcFile);
#pragma endregion

#pragma region Audio Processor Rack
		AudioFormatManager formatManager;
		std::unique_ptr<AudioFormatReaderSource> readerSource;
		std::unique_ptr<MemoryAudioSource> memorySource;

		std::unique_ptr<Ebu128LoudnessMeter> ebuLoudnessMeter;
		std::unique_ptr<GainProcessor> gainProcessor;
		std::unique_ptr<FilterProcessor> filterProcessor;

		void CreateMemoryAudioSource();
#pragma endregion

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
