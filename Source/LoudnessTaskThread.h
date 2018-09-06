/*
  ==============================================================================

    LoudnessTaskThread.h
    Created: 5 Sep 2018 10:06:36am
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
//#include "./GainProcessor/GainProcessor.h"

class LoudnessTaskParameters
{
	public:
		File inputFile;
		File outputFolder;
		Component* parentComponent;

		float fileDbLufs;
		float gainToApply;
		float dBLufsTarget;
		float dBLimiterCeiling;
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
		std::unique_ptr<MemoryAudioSource> memorySource;
		//std::unique_ptr<AudioFormatReaderSource> readerSource;

		//std::unique_ptr<GainProcessor> gainProcessor;

		bool taskIsActive = false;

		double fileSampleRate;
		double bitsPerSample;
		int64 fileTotalLength;
		int64 fileGetNextReadPosition;

		//enum PassID
		//{
		//	ebuLoudness = 0,
		//	gain = 1,
		//	limiter = 2,
		//};
		//PassID passID;

};