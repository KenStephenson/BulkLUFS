/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./View/Screen.h"
#include "./View/FileListBoxModel.h"
#include "./LoudnessProcessor/Ebu128LoudnessMeter.h"
#include "./LoudnessTaskThread.h"
#include "./BrickwallLimiter/PeakLevelDetector.h"
#include "./BrickwallLimiter/GainDynamics.h"
#include "./VstHost/PluginWrapperProcessor.h"

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
		Array<File> inputFiles;
		Array<File> outputFiles;
		int activeIndex;
		File activeFile;
		float dBLufsTarget;
		float dBLimiterCeiling;
		double fileSampleRate;
		double bitsPerSample;
		int64 fileTotalLength;
		int64 fileGetNextReadPosition;

		void runProcess();
		void runPostProcess();
		void applyGain(AudioSampleBuffer* audioBuffer);
		void applyBrickwallLimiter(AudioSampleBuffer* audioBuffer);
		void writeOutputFile(AudioSampleBuffer* audioBuffer);

		bool validateProcessorParameters();
		bool loadFileFromDisk(File srcFile);

#pragma endregion

#pragma region Audio Processor Rack
		AudioFormatManager formatManager;
		std::unique_ptr<AudioFormatReaderSource> readerSource;
		AudioTransportSource transportSource;
		enum TransportState
		{
			Stopped,
			Starting,
			Playing,
			Stopping,
			Paused,
			Pausing,
		};
		TransportState state;

		std::unique_ptr<Ebu128LoudnessMeter> ebuLoudnessMeter;


		std::unique_ptr<AudioProcessor> limiterPlugin;
		const String limiterPluginName = "SPAN";	// "George Yohng's W1 Limiter x64";
		void loadLimiterPlugin();


		float thresholdDb = 1.0f;
		ScopedPointer<PeakLevelDetector> leftLevelDetector, rightLevelDetector;
		ScopedPointer<GainDynamics> gainDymanics;
		float aRatio = 10.0f;
		float attackTime = 0.010f;
		float releaseTime = 0.50f;

		float peakOutL, peakOutR, peakSum, peakSumDb;
		float gain, gainDb;

#pragma endregion

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
