/*
  ==============================================================================

    OfflineLoudnessProcessor.cpp
    Created: 10 Sep 2018 11:50:05am
    Author:  Ken

  ==============================================================================
*/

#include "OfflineLoudnessProcessor.h"

OfflineLoudnessProcessor::OfflineLoudnessProcessor(OfflineLoudnessScanDataPacket* _scanItem)
	: Thread("LUFSScan"), 
	scanItem(_scanItem), 
	dBLufsTarget(_scanItem->dBLufsTarget), 
	dBLimiterCeiling(_scanItem->dBLimiterCeiling), 
	destinationFolder(_scanItem->destinationFolder), 
	writeFile(_scanItem->writeFile)
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
	}

	//if (timer->isTimerRunning())
	//{
	//	timer->stopTimer();
	//}
}


void OfflineLoudnessProcessor::startProcess()
{
	if (loadFileFromDisk(scanItem->file))
	{
		AudioFormatReader* fmtReader = readerSource->getAudioFormatReader();
		numSamples = fmtReader->lengthInSamples;
		numChannels = fmtReader->numChannels;

		audioBuffer = std::make_unique<AudioSampleBuffer>((int)numChannels, (int)numSamples);
		fmtReader->read(audioBuffer.get(), 0, (int)numSamples, 0, true, true);

		samplesPerBlock = (int)fmtReader->sampleRate;
		preProcessLoudnessMeter->setFreezeLoudnessRangeOnSilence(true);
		preProcessLoudnessMeter->prepareToPlay((double)numSamples, 2, samplesPerBlock, pulseTimerHz);
		preProcessLoudnessMeter->reset();

		initialiseBrickwallLimiter();
		limiterPlugin->prepareToPlay(samplesPerBlock, (int)numSamples);

		postProcessLoudnessMeter->setFreezeLoudnessRangeOnSilence(true);
		postProcessLoudnessMeter->prepareToPlay((double)numSamples, 2, samplesPerBlock, pulseTimerHz);
		postProcessLoudnessMeter->reset();

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
		processAudioBuffer((int)numSamples - bufferPointer);
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
	AudioSampleBuffer workBuffer((int)numChannels, bufferSize);
	workBuffer.setDataToReferTo((float**)audioBuffer->getArrayOfReadPointers(), (int)numChannels, bufferPointer, bufferSize);
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
			scanItem->preIntegratedLufs = preProcessLoudnessMeter->getIntegratedLoudness();
			scanItem->prePeakDbfs = Decibels::gainToDecibels(audioBuffer->getMagnitude(0, (int)numSamples));

			applyGain();
			initialiseTimer(ProcessStep::BrickwallLimiter);
			break;

		case ProcessStep::BrickwallLimiter:
			initialiseTimer(ProcessStep::PostProcessLoudness);
			break;

		case ProcessStep::PostProcessLoudness:
			// This called after the Post Processing has been done
			// Finalise and call the next file
			scanItem->postIntegratedLufs = postProcessLoudnessMeter->getIntegratedLoudness();
			scanItem->postPeakDbfs = Decibels::gainToDecibels(audioBuffer->getMagnitude(0, (int)numSamples));
			scanItem->postShortTermLoudness = postProcessLoudnessMeter->getShortTermLoudness();
			scanItem->postMaximumShortTermLoudness = postProcessLoudnessMeter->getMaximumShortTermLoudness();
			scanItem->postMomentaryLoudness = postProcessLoudnessMeter->getMomentaryLoudness();
			scanItem->postMaximumMomentaryLoudness = postProcessLoudnessMeter->getMaximumMomentaryLoudness();
			scanItem->postLoudnessRangeStart = postProcessLoudnessMeter->getLoudnessRangeStart();
			scanItem->postLoudnessRangeEnd = postProcessLoudnessMeter->getLoudnessRangeEnd();
			scanItem->postLoudnessRange = postProcessLoudnessMeter->getLoudnessRange();

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
	scanItem->diffLufs = dbDifference;
	scanItem->gain = gainFactor;
	audioBuffer->applyGain(gainFactor);
}

void OfflineLoudnessProcessor::writeOutputFile()
{
	File wavFile = destinationFolder.getChildFile(scanItem->file.getFileName());
	wavFile.deleteFile();

	FileOutputStream* fos = new FileOutputStream(wavFile);
	WavAudioFormat wavFormat;
	ScopedPointer<AudioFormatWriter> afw(wavFormat.createWriterFor(fos, (int)fileSampleRate, audioBuffer->getNumChannels(), fileBitsPerSample, StringPairArray(), 0));
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
	std::unique_ptr<PluginDirectoryScanner> scanner = std::make_unique<PluginDirectoryScanner>(knownPluginList, *format, path, true, File(), false);

	String currentPlugBeingScanned = "DUMMY";
	while (currentPlugBeingScanned != "")
	{
		currentPlugBeingScanned = scanner->getNextPluginFileThatWillBeScanned();
		File f(currentPlugBeingScanned);
		String testStr = f.getFileNameWithoutExtension();
		if (testStr == limiterPluginName)
		{
			scanner->scanNextFile(true, currentPlugBeingScanned);
			break;
		}
		scanner->skipNextFile();
	}
	if (knownPluginList.getNumTypes() > 0)
	{
		PluginDescription* plugIn = knownPluginList.getType(0);
		AudioPluginFormatManager fm;
		fm.addDefaultFormats();
		String ignore;
		if (AudioPluginInstance* pluginInstance = fm.createPluginInstance(*plugIn, 44100.0, 512, ignore))
		{
			limiterPlugin = std::make_shared<PluginWrapperProcessor>(pluginInstance);
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
	limiterPlugin->setParameter(2, 20.0f);	// Release
	limiterPlugin->setParameter(3, false);	// Auto Release
}

#pragma endregion

