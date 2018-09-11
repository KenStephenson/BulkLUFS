/*
  ==============================================================================

    OfflineLoudnessProcessor.cpp
    Created: 10 Sep 2018 11:50:05am
    Author:  Ken

  ==============================================================================
*/

#include "OfflineLoudnessProcessor.h"

OfflineLoudnessProcessor::OfflineLoudnessProcessor(String threadName, 
													float _dBLufsTarget, 
													float _dbLimiterCeiling,
													FileLoudnessDetails* _fileDetails, 
													File _destinationFolder, 
													bool _writeFile)
	: Thread(threadName),
	dBLufsTarget(_dBLufsTarget),
	dBLimiterCeiling(_dbLimiterCeiling),
	fileDetails(_fileDetails),
	destinationFolder(_destinationFolder),
	writeFile(_writeFile)
{
};

OfflineLoudnessProcessor::~OfflineLoudnessProcessor() 
{
};


void OfflineLoudnessProcessor::run()
{
	formatManager.registerBasicFormats();
	loadLimiterPlugin();
	preProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	postProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	timer = std::make_unique<PulseTimer>(this);

	currentProcessStep = ProcessStep::None;
	startProcess();

	// loop until it is called to stop
	while (!threadShouldExit())
	{
		//sleep(5);
	}
	if (timer->isTimerRunning())
	{
		timer->stopTimer();
	}
}


void OfflineLoudnessProcessor::startProcess()
{
	if (loadFileFromDisk(fileDetails->file))
	{
		AudioFormatReader* fmtReader = readerSource->getAudioFormatReader();
		numSamples = fmtReader->lengthInSamples;
		numChannels = fmtReader->numChannels;

		audioBuffer = std::make_unique<AudioSampleBuffer>(numChannels, numSamples);
		fmtReader->read(audioBuffer.get(), 0, numSamples, 0, true, true);

		samplesPerBlock = fmtReader->sampleRate;
		preProcessLoudnessMeter->reset();
		preProcessLoudnessMeter->prepareToPlay(numSamples, 2, samplesPerBlock, pulseTimerHz);

		initialiseBrickwallLimiter();
		limiterPlugin->prepareToPlay(samplesPerBlock, numSamples);

		postProcessLoudnessMeter->reset();
		postProcessLoudnessMeter->prepareToPlay(numSamples, 2, samplesPerBlock, pulseTimerHz);

		initialiseTimer(ProcessStep::InitialLoudness);
	}
}
void OfflineLoudnessProcessor::initialiseTimer(ProcessStep _processStep)
{
	currentProcessStep = _processStep;
	bufferPointer = 0;
	timer.get()->startTimerHz(pulseTimerHz);
}

void OfflineLoudnessProcessor::handleTimerTick()
{
	if (bufferPointer > numSamples - samplesPerBlock)
	{
		timer->stopTimer();
		processAudioBuffer(numSamples - bufferPointer);
		scanComplete();
	}
	else
	{
		processAudioBuffer(samplesPerBlock);
		bufferPointer += samplesPerBlock;
	}
}

void OfflineLoudnessProcessor::processAudioBuffer(int bufferSize)
{
	AudioSampleBuffer workBuffer(numChannels, bufferSize);
	workBuffer.setDataToReferTo((float**)audioBuffer->getArrayOfReadPointers(), numChannels, bufferPointer, bufferSize);
	switch (currentProcessStep)
	{
		case ProcessStep::InitialLoudness:
			preProcessLoudnessMeter->processBlock(workBuffer);
			break;
		case ProcessStep::BrickwallLimiter:
			limiterPlugin->processBlock(workBuffer, midiBuffer);
			break;
		case ProcessStep::PostProcessLoudness:
			postProcessLoudnessMeter->processBlock(workBuffer);
			break;
	};
}

void OfflineLoudnessProcessor::scanComplete()
{
	bufferPointer = 0;

	switch (currentProcessStep)
	{
		case ProcessStep::InitialLoudness:
			// This called after the Initial Loudness scan
			// Start the Post Processing
			// the Post Process Loudness scan will restart the timer and will finalise in this runPostProcessing
			fileDetails->preIntegratedLufs = preProcessLoudnessMeter->getIntegratedLoudness();
			fileDetails->prePeakDbfs = Decibels::gainToDecibels(audioBuffer->getMagnitude(0, numSamples));

			applyGain();
			initialiseTimer(ProcessStep::BrickwallLimiter);
			break;

		case ProcessStep::BrickwallLimiter:
			initialiseTimer(ProcessStep::PostProcessLoudness);
			break;

		case ProcessStep::PostProcessLoudness:
			// This called after the Post Processing has been done
			// Finalise and call the next file
			fileDetails->postIntegratedLufs = postProcessLoudnessMeter->getIntegratedLoudness();
			fileDetails->postPeakDbfs = Decibels::gainToDecibels(audioBuffer->getMagnitude(0, numSamples));

			if (writeFile)
			{
				writeOutputFile();
			}
			currentProcessStep = ProcessStep::None;
			signalThreadShouldExit();
			break;
	};

}
void OfflineLoudnessProcessor::applyGain()
{
	float fileDbLufs = preProcessLoudnessMeter->getIntegratedLoudness();
	float dbDifference = (fileDbLufs * -1) - (dBLufsTarget * -1);
	float gainFactor = Decibels::decibelsToGain(dbDifference);
	fileDetails->diffLufs = dbDifference;
	fileDetails->gain = gainFactor;
	audioBuffer->applyGain(gainFactor);
}

void OfflineLoudnessProcessor::writeOutputFile()
{
	File wavFile = destinationFolder.getChildFile(fileDetails->file.getFileName());
	wavFile.deleteFile();

	FileOutputStream* fos = new FileOutputStream(wavFile);
	WavAudioFormat wavFormat;
	ScopedPointer<AudioFormatWriter> afw(wavFormat.createWriterFor(fos, fileSampleRate, audioBuffer->getNumChannels(), fileBitsPerSample, StringPairArray(), 0));
	afw->writeFromAudioSampleBuffer(*audioBuffer, 0, audioBuffer->getNumSamples());
}

#pragma region Load Resources
bool OfflineLoudnessProcessor::loadFileFromDisk(File srcFile)
{
	if (auto* reader = formatManager.createReaderFor(srcFile))
	{
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		readerSource.reset(newSource.release());
		readerSource->setLooping(false);

		fileSampleRate = reader->sampleRate;
		fileBitsPerSample = reader->bitsPerSample;
		return true;
	}
	fileSampleRate = 0;
	fileBitsPerSample = 0;
	return false;
}
void OfflineLoudnessProcessor::loadLimiterPlugin()
{
	KnownPluginList knownPluginList;
	std::unique_ptr<AudioPluginFormat> format = std::make_unique<VSTPluginFormat>();
	FileSearchPath path = format->getDefaultLocationsToSearch();

	// Scan the directory for plugins
	std::unique_ptr<PluginDirectoryScanner> scanner = std::make_unique<PluginDirectoryScanner>(knownPluginList, *format, path, true, File::nonexistent, false);
	String currentPlugBeingScanned;
	while (scanner->scanNextFile(true, currentPlugBeingScanned)) {}

	for (int i = 0; i < knownPluginList.getNumTypes(); i++)
	{
		PluginDescription* plugIn = knownPluginList.getType(i);
		if (plugIn->name == limiterPluginName)
		{
			AudioPluginFormatManager fm;
			fm.addDefaultFormats();

			String ignore;
			if (AudioPluginInstance* pluginInstance = fm.createPluginInstance(*plugIn, 44100.0, 512, ignore))
			{
				limiterPlugin = std::make_shared<PluginWrapperProcessor>(pluginInstance);
			}
		}
	}
}
void OfflineLoudnessProcessor::initialiseBrickwallLimiter()
{
	float db_1 = Decibels::decibelsToGain(-1.0f);
	float dbFS = Decibels::decibelsToGain(dBLimiterCeiling);

	limiterPlugin->setNonRealtime(true);
	limiterPlugin->setParameter(0, dbFS);	// Threshold
	limiterPlugin->setParameter(1, dbFS);	// Ceiling
	limiterPlugin->setParameter(2, 200.0f);	// Release
	limiterPlugin->setParameter(3, false);	// Auto Release
}

#pragma endregion

