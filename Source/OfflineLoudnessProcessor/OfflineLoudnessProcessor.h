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
		void timerCallback() { if (listener != nullptr)	{ listener->handleTimerTick(); }}
		TimerListener* listener;
};

class OfflineLoudnessProcessor : public Thread, public TimerListener
{
	public:
		OfflineLoudnessProcessor(std::shared_ptr<OfflineLoudnessScanDataPacket> _offlineLoudnessScanData);
		~OfflineLoudnessProcessor();

		void run() override;
		void shutDownThread();

	private:
		enum ProcessStep
		{
			Initialise = 0,
			PreProcessLoudness = 1,
			BrickwallLimiter = 2,
			PostProcessLoudness = 3,
			Complete = 4,
		};
		ProcessStep currentProcessStep = ProcessStep::Initialise;
		void runProcessStep();
		void runProcessStepInitialise();
		void runProcessStepPreLoudness();
		void runProcessStepBrickwallLimiter();
		void runProcessStepPostLoudness();

		bool loadFileFromDisk(File srcFile);
		void loadLimiterPlugin();
		void initialiseTimer(ProcessStep _processStep);
		void handleTimerTick() override;
		void processAudioBuffer(int bufferSize);

		AudioFormatManager formatManager;
		MidiBuffer midiBuffer;
		std::unique_ptr<AudioFormatReaderSource> readerSource;
		std::unique_ptr<AudioSampleBuffer> audioBuffer;
		std::unique_ptr<Ebu128LoudnessMeter> preProcessLoudnessMeter;
		std::unique_ptr<Ebu128LoudnessMeter> postProcessLoudnessMeter;
		std::unique_ptr<AudioProcessor> limiterPlugin;
		std::unique_ptr<PulseTimer> timer;
		std::shared_ptr<OfflineLoudnessScanDataPacket> offlineLoudnessScanData;
		double fileSampleRate;
		double fileBitsPerSample;

		int pulseTimerHz = 200;
		int samplesPerBlock = 44100;
		int bufferPointer = 0;
		int64 numSamples = 0;
		int64 numChannels = 0;

		const String limiterPluginName = "George Yohng's W1 Limiter";
		const String limiterPluginName64 = "George Yohng's W1 Limiter x64";
};

