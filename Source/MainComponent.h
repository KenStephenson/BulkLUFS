/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./View/Screen.h"
#include "./View/FileListBoxModel.h"
#include "./EBU-R128/Ebu128LoudnessMeter.h"
#include "./VstHost/PluginWrapperProcessor.h"
#include "./OfflineLoudnessProcessor/OfflineLoudnessProcessor.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class TimerListener
{
public:
	TimerListener() {};
	~TimerListener() {};
	virtual void handleTimerTick() {};
};
class PulseTimer : public Timer
{
public:
	PulseTimer() { listener = nullptr; }
	PulseTimer(TimerListener* lstnr) { listener = lstnr; }
	~PulseTimer() {};

private:
	void timerCallback()
	{
		if (listener != nullptr)
		{
			listener->handleTimerTick();
		}
	}
	TimerListener* listener;
};

class MainComponent : public Component, public ListBoxModelListener, public TimerListener
{
	public:
		//==============================================================================
		using Track = Grid::TrackInfo;

		//==============================================================================
		MainComponent();
		~MainComponent();

		//==============================================================================
		void paint (Graphics& g) override;
		void resized() override;

		void ModelRefresh(String tag) override;

	private:

#pragma region Process Methods and Parameters
		void runProcess();
		void processNextFile();
		void handleTimerTick() override;
		void runPostProcess();
		void applyGain(AudioSampleBuffer* audioBuffer);
		void applyBrickwallLimiter(AudioSampleBuffer* audioBuffer);
		void writeOutputFile(AudioSampleBuffer* audioBuffer);
		bool validateProcessorParameters();
		bool loadFileFromDisk(File srcFile);
		void loadLimiterPlugin();

		AudioFormatManager formatManager;
		MidiBuffer midiBuffer;
		std::unique_ptr<AudioFormatReaderSource> readerSource;
		std::unique_ptr<Ebu128LoudnessMeter> preProcessLoudnessMeter;
		std::unique_ptr<Ebu128LoudnessMeter> postProcessLoudnessMeter;
		std::unique_ptr<AudioSampleBuffer> audioBuffer;
		std::unique_ptr<PulseTimer> timer;

		std::shared_ptr<AudioProcessor> limiterPlugin;
		const String limiterPluginName = "George Yohng's W1 Limiter x64";

		float dBLufsTarget;
		float dBLimiterCeiling;
		double fileSampleRate;
		double bitsPerSample;
		int64 fileTotalLength;
		int64 fileGetNextReadPosition;
		int expectedRequestRate = 10;
		int samplesPerBlock = 44100;
		int bufferPointer = 0;
		int64 numSamples = 0;
		int64 numChannels = 0;

		Array<File> inputFiles;
		Array<File> outputFiles;
		int activeIndex;
		File activeFile;
#pragma endregion

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

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
