/*
  ==============================================================================

    LoudnessTaskThread.h
    Created: 5 Sep 2018 10:06:36am
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "./LoudnessProcessor/Ebu128LoudnessMeter.h"
#include "./GainProcessor/GainProcessor.h"

class LoudnessTaskParameters
{
	public:
		float dBLufsTarget = -15.0;
		float dBLimiterCeiling = -1.0;
		File inputFile;
		File outputFolder;
		Component* parentComponent;
};

class LoudnessTaskThread : public ThreadWithProgressWindow
{
	public:
		LoudnessTaskThread(LoudnessTaskParameters params);
		~LoudnessTaskThread();

		void run();

	private:
		bool loadFileFromDisk(File srcFile);
		void updateProgressPercentage();

		LoudnessTaskParameters parameters;
		AudioFormatManager formatManager;
		//std::unique_ptr<MemoryAudioSource> memorySource;
		std::unique_ptr<AudioFormatReaderSource> readerSource;

		std::unique_ptr<Ebu128LoudnessMeter> ebuLoudnessMeter;
		std::unique_ptr<GainProcessor> gainProcessor;
		std::unique_ptr<FilterProcessor> filterProcessor;

		bool taskIsActive = false;

		double fileSampleRate;
		double bitsPerSample;
		int64 fileTotalLength;
		int64 fileGetNextReadPosition;
		float fileDbLufs;

		enum PassID
		{
			ebuLoudness = 0,
			gain = 1,
			limiter = 2,
		};
		PassID passID;

};