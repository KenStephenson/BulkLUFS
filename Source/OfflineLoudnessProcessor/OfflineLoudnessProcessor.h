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
#include "./OfflineLoudnessScanDataPacket.h"

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

class OfflineLoudnessProcessor : public Thread, public TimerListener
{
	public:
		OfflineLoudnessProcessor(OfflineLoudnessScanDataPacket* _fileDetails);
		~OfflineLoudnessProcessor();

		void run() override;

	private:
		AudioFormatManager formatManager;
		MidiBuffer midiBuffer;
		std::unique_ptr<AudioFormatReaderSource> readerSource;
		std::unique_ptr<AudioSampleBuffer> audioBuffer;
		std::unique_ptr<Ebu128LoudnessMeter> preProcessLoudnessMeter;
		std::unique_ptr<Ebu128LoudnessMeter> postProcessLoudnessMeter;
		std::shared_ptr<AudioProcessor> limiterPlugin;
		std::unique_ptr<PulseTimer> timer;

		const String limiterPluginName = "George Yohng's W1 Limiter";
		const String limiterPluginName64 = "George Yohng's W1 Limiter x64";

		OfflineLoudnessScanDataPacket* scanItem;
		float dBLufsTarget;
		float dBLimiterCeiling;
		double fileSampleRate;
		double fileBitsPerSample;
		File destinationFolder;
		bool writeFile = false;

		int pulseTimerHz = 200;
		int samplesPerBlock = 44100;
		int bufferPointer = 0;
		int64 numSamples = 0;
		int64 numChannels = 0;

		enum ProcessStep
		{
			None = 0,
			InitialLoudness = 1,
			BrickwallLimiter = 2,
			PostProcessLoudness = 3
		};
		ProcessStep currentProcessStep = ProcessStep::None;

		void startProcess();
		void initialiseTimer(ProcessStep _processStep);
		void handleTimerTick() override;
		void processAudioBuffer(int bufferSize);
		void scanComplete();
		void applyGain();
		void writeOutputFile();
		bool loadFileFromDisk(File srcFile);
		void loadLimiterPlugin();
		void initialiseBrickwallLimiter();
};

