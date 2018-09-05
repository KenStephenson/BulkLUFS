/*
  ==============================================================================

    LoudnessTaskThread.cpp
    Created: 5 Sep 2018 10:06:36am
    Author:  Ken

  ==============================================================================
*/

#include "LoudnessTaskThread.h"

LoudnessTaskThread::LoudnessTaskThread(LoudnessTaskParameters params)
	: ThreadWithProgressWindow("Processing " + params.inputFile.getFileName(), true, true, 10000, "Cancel", params.parentComponent)
{
	parameters = params;

	gainProcessor = std::make_unique<GainProcessor>();
	gainProcessor->setGainDecibels(parameters.gainToApply);

	formatManager.registerBasicFormats();
}

LoudnessTaskThread::~LoudnessTaskThread()
{
}

void LoudnessTaskThread::run()
{
	if (loadFileFromDisk(parameters.inputFile) == false)
	{
		return;
	}
	if (memorySource.get() == nullptr)
	{
		return;
	}
	
	taskIsActive = true;
	fileGetNextReadPosition = 0;
	int samplesPerBlock = (int)((double)fileSampleRate / (double)100);
	updateProgressPercentage();

	gainProcessor->prepareToPlay(fileSampleRate, samplesPerBlock);
	memorySource->prepareToPlay(samplesPerBlock, fileSampleRate);

	MidiBuffer midiBuffer;
	while (taskIsActive)
	{
		AudioBuffer<float> audioBuffer(2, samplesPerBlock);
		AudioSourceChannelInfo bufferToFill(&audioBuffer, 0, samplesPerBlock);
		memorySource->getNextAudioBlock(bufferToFill);

		fileGetNextReadPosition += bufferToFill.numSamples;	// readerSource->getNextReadPosition();
		updateProgressPercentage();

		auto* inBuffer = bufferToFill.buffer->getArrayOfReadPointers();
		AudioSampleBuffer sBuffer;
		sBuffer.setDataToReferTo((float**)inBuffer, bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);

		gainProcessor->processBlock(sBuffer, midiBuffer);

		if (fileGetNextReadPosition >= fileTotalLength)
		{
			taskIsActive = false;
		}
		if (threadShouldExit())
		{
			break;
		}
	}
}

bool LoudnessTaskThread::loadFileFromDisk(File srcFile)
{
	if (auto* reader = formatManager.createReaderFor(srcFile))
	{
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		//readerSource.reset(newSource.release());
		
		fileTotalLength = newSource->getTotalLength();
		fileSampleRate = reader->sampleRate;
		bitsPerSample = reader->bitsPerSample;

		// Move the data into a Memory Audio Source
		AudioBuffer<float> audioBuffer(2, fileTotalLength);
		AudioFormatReader* fmtReader = newSource->getAudioFormatReader();
		fmtReader->read(&audioBuffer, 0, fileTotalLength, 0, true, true);
		memorySource = std::make_unique<MemoryAudioSource>(audioBuffer, true, false);
		newSource.release();
		return true;
	}
	fileSampleRate = 0;
	bitsPerSample = 0;
	return false;
}

//void LoudnessTaskThread::CreateMemoryAudioSource()
//{
//	//AudioFormatReader* fmtReader = readerSource->getAudioFormatReader();
//	//AudioBuffer<float> audioBuffer(2, fileTotalLength);
//	//fmtReader->read(&audioBuffer, 0, fileTotalLength, 0, true, true);
//	//memorySource = std::make_unique<MemoryAudioSource>(audioBuffer, true, false);
//}
//void LoudnessTaskThread::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
//{
//	if (memorySource.get() != nullptr)
//	{
//
//		//switch (passID)
//		//{
//		//case LoudnessTaskThread::ebuLoudness:
//		//	ebuLoudnessMeter->reset();
//		//	ebuLoudnessMeter->prepareToPlay(fileSampleRate, 2, samplesPerBlock, 10);
//		//	break;
//		//case LoudnessTaskThread::gain:
//		//	gainProcessor->prepareToPlay(fileSampleRate, samplesPerBlock);
//		//	break;
//		//case LoudnessTaskThread::limiter:
//		//	return;
//		//default:
//		//	return;
//		//}
//
//		memorySource->prepareToPlay(samplesPerBlockExpected, sampleRate);
//	}
//}
//void LoudnessTaskThread::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
//{
//	if (memorySource.get() == nullptr)
//	{
//		bufferToFill.clearActiveBufferRegion();
//		return;
//	}
//
//	memorySource->getNextAudioBlock(bufferToFill);
//
//	//fileGetNextReadPosition = readerSource->getNextReadPosition();
//	//updateProgressPercentage();
//
//	//auto* inBuffer = bufferToFill.buffer->getArrayOfReadPointers();
//	//AudioSampleBuffer sBuffer;
//	//sBuffer.setDataToReferTo((float**)inBuffer, bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
//
//	//switch (passID)
//	//{
//	//case LoudnessTaskThread::ebuLoudness:
//	//	ebuLoudnessMeter->processBlock(sBuffer);
//	//	break;
//	//case LoudnessTaskThread::gain:
//	//	break;
//	//case LoudnessTaskThread::limiter:
//	//	break;
//	//default:
//	//	break;
//	//}
//
//	//if (fileGetNextReadPosition >= fileTotalLength)
//	//{
//	//	switch (passID)
//	//	{
//	//	case LoudnessTaskThread::ebuLoudness:
//	//		fileDbLufs = ebuLoudnessMeter->getIntegratedLoudness();
//
//	//		// Initiate the second pass
//	//		passID = PassID::gain;
//	//		prepareToPlay((int)((double)fileSampleRate / (double)100), fileSampleRate);
//	//		break;
//	//	case LoudnessTaskThread::gain:
//
//	//		break;
//	//	case LoudnessTaskThread::limiter:
//	//		break;
//	//	default:
//	//		break;
//	//	}
//	//}
//}
//void LoudnessTaskThread::releaseResources()
//{
//}

void LoudnessTaskThread::updateProgressPercentage()
{
	double percent = 0;
	if (fileTotalLength > 0)
	{
		percent = ((double)fileGetNextReadPosition / (double)fileTotalLength);
	}
	setProgress(percent);
}

