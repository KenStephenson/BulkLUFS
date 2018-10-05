/*
  ==============================================================================

    OfflineLoudnessProcessor.cpp
    Created: 10 Sep 2018 11:50:05am
    Author:  Ken

  ==============================================================================
*/

#include "OfflineLoudnessProcessor.h"

OfflineLoudnessProcessor::OfflineLoudnessProcessor(std::shared_ptr<TrackModel> _offlineLoudnessScanData)
	: Thread("LUFSScan"), trackData(_offlineLoudnessScanData) 
{
	limiterPlugin = trackData->limiterPlugin;
};

OfflineLoudnessProcessor::~OfflineLoudnessProcessor() 
{
	shutDownThread();
};

void OfflineLoudnessProcessor::run()
{

	currentProcessStep = ProcessStep::Initialise;
	runProcessStep();

	// loop until it is called to stop
	while (!threadShouldExit()) 
	{
	}
}

void OfflineLoudnessProcessor::shutDownThread()
{
	if (isThreadRunning())
	{
		signalThreadShouldExit();
	}
}

void OfflineLoudnessProcessor::runProcessStep()
{
	switch (currentProcessStep)
	{
	case ProcessStep::Initialise:
		runProcessStepInitialise();
		break;

	case ProcessStep::PreProcessLoudness:
		runProcessStepPreLoudness();
		break;

	case ProcessStep::BrickwallLimiter:
		runProcessStepBrickwallLimiter();
		break;

	case ProcessStep::PostProcessLoudness:
		runProcessStepPostLoudness();
		break;

	case ProcessStep::Complete:
		signalThreadShouldExit();
		break;
	};

}

void OfflineLoudnessProcessor::runProcessStepInitialise()
{
	formatManager.registerBasicFormats();
	if (!loadFileFromDisk(trackData->file))
	{
		signalThreadShouldExit();
		return;
	}

	//loadLimiterPlugin();
	if (limiterPlugin == nullptr)
	{
		signalThreadShouldExit();
		return;
	}

	preProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	postProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	timer = std::make_unique<PulseTimer>(this);

	AudioFormatReader* fmtReader = readerSource->getAudioFormatReader();
	numSamples = fmtReader->lengthInSamples;
	numChannels = fmtReader->numChannels;
	samplePerBlock = (int)fmtReader->sampleRate;
	pulseTimerHz = 100;

	audioBuffer = std::make_unique<AudioSampleBuffer>((int)numChannels, (int)numSamples);
	fmtReader->read(audioBuffer.get(), 0, (int)numSamples, 0, true, true);

	preProcessLoudnessMeter->prepareToPlay((double)fileSampleRate, 2, samplePerBlock, pulseTimerHz);
	if (limiterPlugin != nullptr)
	{
		limiterPlugin->prepareToPlay(fileSampleRate, (int)samplePerBlock);
	}
	postProcessLoudnessMeter->prepareToPlay((double)fileSampleRate, 2, samplePerBlock, pulseTimerHz);

	initialiseTimer(ProcessStep::PreProcessLoudness);
}
void OfflineLoudnessProcessor::runProcessStepPreLoudness()
{
	// This called after the Initial Loudness scan
	trackData->preIntegratedLufs = preProcessLoudnessMeter->getIntegratedLoudness();
	trackData->prePeakDbfs = Decibels::gainToDecibels(audioBuffer->getMagnitude(0, (int)numSamples));

	// Apply the required Gain
	float fileDbLufs = preProcessLoudnessMeter->getIntegratedLoudness();
	float dbDifference = (fileDbLufs * -1) - (trackData->dBLufsTarget * -1);
	float gainFactor = Decibels::decibelsToGain(dbDifference);
	trackData->diffLufs = dbDifference;
	trackData->gain = gainFactor;
	audioBuffer->applyGain(gainFactor);

	initialiseTimer(ProcessStep::BrickwallLimiter);

}
void OfflineLoudnessProcessor::runProcessStepBrickwallLimiter()
{
	initialiseTimer(ProcessStep::PostProcessLoudness);
}
void OfflineLoudnessProcessor::runProcessStepPostLoudness()
{
	// This called after the Post Processing has been done
	// Finalise and call the next file
	trackData->postIntegratedLufs = postProcessLoudnessMeter->getIntegratedLoudness();
	trackData->postPeakDbfs = Decibels::gainToDecibels(audioBuffer->getMagnitude(0, (int)numSamples));
	trackData->postShortTermLoudness = postProcessLoudnessMeter->getShortTermLoudness();
	trackData->postMaximumShortTermLoudness = postProcessLoudnessMeter->getMaximumShortTermLoudness();
	trackData->postMomentaryLoudness = postProcessLoudnessMeter->getMomentaryLoudness();
	trackData->postMaximumMomentaryLoudness = postProcessLoudnessMeter->getMaximumMomentaryLoudness();
	trackData->postLoudnessRangeStart = postProcessLoudnessMeter->getLoudnessRangeStart();
	trackData->postLoudnessRangeEnd = postProcessLoudnessMeter->getLoudnessRangeEnd();
	trackData->postLoudnessRange = postProcessLoudnessMeter->getLoudnessRange();

	if (trackData->writeFile)
	{
		File audioFile = trackData->destinationFolder.getChildFile(trackData->file.getFileName());
		audioFile.deleteFile();
		FileOutputStream* fos = new FileOutputStream(audioFile);
		WavAudioFormat wavFormat;
		if (wavFormat.canHandleFile(audioFile))
		{
			ScopedPointer<AudioFormatWriter> afw(wavFormat.createWriterFor(fos, (double)fileSampleRate, audioBuffer->getNumChannels(), fileBitsPerSample, StringPairArray(), 0));
			afw->writeFromAudioSampleBuffer(*audioBuffer, 0, audioBuffer->getNumSamples());
		}
		else
		{
			AiffAudioFormat aiffFormat;
			if (aiffFormat.canHandleFile(audioFile))
			{
				ScopedPointer<AudioFormatWriter> afw(aiffFormat.createWriterFor(fos, (double)fileSampleRate, audioBuffer->getNumChannels(), fileBitsPerSample, StringPairArray(), 0));
				afw->writeFromAudioSampleBuffer(*audioBuffer, 0, audioBuffer->getNumSamples());
			}
			else
			{
				FlacAudioFormat flacFormat;
				if (flacFormat.canHandleFile(audioFile))
				{
					ScopedPointer<AudioFormatWriter> afw(flacFormat.createWriterFor(fos, (double)fileSampleRate, audioBuffer->getNumChannels(), fileBitsPerSample, StringPairArray(), 0));
					afw->writeFromAudioSampleBuffer(*audioBuffer, 0, audioBuffer->getNumSamples());
				}
			}
		}
	}
	bufferPointer = 0;
	currentProcessStep = ProcessStep::Complete;
	runProcessStep();
}


void OfflineLoudnessProcessor::initialiseTimer(ProcessStep _processStep)
{
	currentProcessStep = _processStep;
	bufferPointer = 0;
	timer.get()->startTimerHz(pulseTimerHz);
}
void OfflineLoudnessProcessor::handleTimerTick()
{
	if (bufferPointer > numSamples - samplePerBlock)
	{
		timer->stopTimer();
		processAudioBuffer((int)numSamples - bufferPointer);
		runProcessStep();
	}
	else
	{
		processAudioBuffer(fileSampleRate);
		bufferPointer += fileSampleRate;
	}
}
void OfflineLoudnessProcessor::processAudioBuffer(int bufferSize)
{
	AudioSampleBuffer workBuffer((int)numChannels, bufferSize);
	workBuffer.setDataToReferTo((float**)audioBuffer->getArrayOfWritePointers(), (int)numChannels, bufferPointer, bufferSize);
	switch (currentProcessStep)
	{
		case ProcessStep::PreProcessLoudness:
			preProcessLoudnessMeter->processBlock(workBuffer);
			break;
		case ProcessStep::BrickwallLimiter:
			if (limiterPlugin != nullptr)
			{
				limiterPlugin->processBlock(workBuffer, midiBuffer);
			}
			break;
		case ProcessStep::PostProcessLoudness:
			postProcessLoudnessMeter->processBlock(workBuffer);
			break;
	};
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
//void OfflineLoudnessProcessor::loadLimiterPlugin()
//{
//	limiterPlugin = nullptr;
//
//	KnownPluginList knownPluginList;
//	std::unique_ptr<AudioPluginFormat> format = std::make_unique<VSTPluginFormat>();
//	FileSearchPath path = format->getDefaultLocationsToSearch();
//
//	// Scan the directory for plugins
//	std::unique_ptr<PluginDirectoryScanner> scanner = std::make_unique<PluginDirectoryScanner>(knownPluginList, *format, path, true, File(), false);
//
//	String currentPlugBeingScanned = "----";
//	while (currentPlugBeingScanned != "")
//	{
//		currentPlugBeingScanned = scanner->getNextPluginFileThatWillBeScanned();
//		File f(currentPlugBeingScanned);
//		String plugName = f.getFileNameWithoutExtension();
//		if (plugName == limiterPluginName || plugName == limiterPluginName64)
//		{
//			scanner->scanNextFile(true, currentPlugBeingScanned);
//		}
//		else
//		{
//			scanner->skipNextFile();
//		}
//	}
//	int numTypes = knownPluginList.getNumTypes();
//	PluginDescription* plugIn = nullptr;
//	switch (numTypes)
//	{
//		case 1:
//			plugIn = knownPluginList.getType(0);
//			break;
//		case 2:
//			PluginDescription* plugIn1 = knownPluginList.getType(0);
//			PluginDescription* plugIn2 = knownPluginList.getType(1);
//			plugIn = plugIn1->name.contains("x64") ? plugIn1 : plugIn2;
//			break;
//	}
//	if (plugIn != nullptr)
//	{
//		AudioPluginFormatManager fm;
//		fm.addDefaultFormats();
//		String ignore;
//		if (AudioPluginInstance* pluginInstance = fm.createPluginInstance(*plugIn, 44100.0, 512, ignore))
//		{
//			float dbFS = Decibels::decibelsToGain(trackData->dBLimiterCeiling);
//
//			limiterPlugin = std::make_unique<PluginWrapperProcessor>(pluginInstance);
//			limiterPlugin->setNonRealtime(true);
//			limiterPlugin->setParameter(0, dbFS);	// Threshold
//			limiterPlugin->setParameter(1, dbFS);	// Ceiling
//			limiterPlugin->setParameter(2, 20.0f);	// Release
//			limiterPlugin->setParameter(3, false);	// Auto Release
//		}
//	}
//}
#pragma endregion

