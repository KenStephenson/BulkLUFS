/*
  ==============================================================================

    OfflineLoudnessProcessor.h
    Created: 10 Sep 2018 11:50:05am
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "../View/FileListBoxModel.h"
#include "../VstHost/PluginWrapperProcessor.h"
#include"../EBU-R128/Ebu128LoudnessMeter.h"

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

//class OfflineLoudnessProcessor : public ThreadWithProgressWindow, public TimerListener
class OfflineLoudnessProcessor : public TimerListener
{
	public:
		OfflineLoudnessProcessor()	/* : ThreadWithProgressWindow("Processing...", true, true)*/
		{
		}
		~OfflineLoudnessProcessor();
		void run(float dBLufsTarget, FileListBoxModel fileList, File destinationFolder);

private:
#pragma region Process Methods and Parameters
		void processNextFile();
		void handleTimerTick() override;
		void runPostProcess();
		void applyGain(AudioSampleBuffer* audioBuffer);
		void applyBrickwallLimiter(AudioSampleBuffer* audioBuffer);
		void readPostProcessLoudness();
		void writeOutputFile(AudioSampleBuffer* audioBuffer);
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
		bool isPostProcess = false;

		FileListBoxModel fileListModel;
		Array<FileLoudnessDetails> inputFiles;
		int activeIndex;
		File activeFile;
		File destinationFolder;

#pragma endregion
};