/*
  ==============================================================================

    OfflineLoudnessProcessor.cpp
    Created: 10 Sep 2018 11:50:05am
    Author:  Ken

  ==============================================================================
*/

#include "OfflineLoudnessProcessor.h"

OfflineLoudnessProcessor::~OfflineLoudnessProcessor()
{
}

void OfflineLoudnessProcessor::run(float _dBLufsTarget, FileListBoxModel _fileListModel, File _destinationFolder)
{
	dBLufsTarget = _dBLufsTarget;
	fileListModel = _fileListModel;
	destinationFolder = _destinationFolder;

	formatManager.registerBasicFormats();
	loadLimiterPlugin();

	preProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	postProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	timer = std::make_unique<PulseTimer>(this);

	isPostProcess = false;

	int numFiles = fileListModel.getNumRows();
	for (activeIndex = 0; activeIndex < numFiles; ++activeIndex)
	{
		if (activeIndex >= numFiles)
		{
			// completed
			return;
		}
		activeFile = inputFiles[activeIndex].file;

		//// must check this as often as possible, because this is
		//// how we know if the user's pressed 'cancel'
		//if (threadShouldExit())
		//	break;

		//// this will update the progress bar on the dialog box
		//setProgress(activeIndex / (double)numFiles);

		//   ... do the business here...
		processNextFile();
	}
}


#pragma region Run Batch Audio Processing
void OfflineLoudnessProcessor::processNextFile()
{
	if (loadFileFromDisk(activeFile))
	{
		//setStatusMessage(activeFile.getFileName());
		
		AudioFormatReader* fmtReader = readerSource->getAudioFormatReader();
		numSamples = fmtReader->lengthInSamples;
		numChannels = fmtReader->numChannels;

		audioBuffer = std::make_unique<AudioSampleBuffer>(numChannels, numSamples);
		fmtReader->read(audioBuffer.get(), 0, numSamples, 0, true, true);

		expectedRequestRate = 10;
		samplesPerBlock = fmtReader->sampleRate;

		preProcessLoudnessMeter->reset();
		preProcessLoudnessMeter->prepareToPlay(numSamples, 2, samplesPerBlock, expectedRequestRate);

		postProcessLoudnessMeter->reset();
		postProcessLoudnessMeter->prepareToPlay(numSamples, 2, samplesPerBlock, expectedRequestRate);

		isPostProcess = false;
		bufferPointer = 0;
		timer.get()->startTimerHz(100.0f);
	}
}

void OfflineLoudnessProcessor::handleTimerTick()
{
	if (bufferPointer >= numSamples - samplesPerBlock)
	{
		timer->stopTimer();

		int finalBufferSize = numSamples - bufferPointer;
		AudioSampleBuffer workBuffer(numChannels, finalBufferSize);
		workBuffer.setDataToReferTo((float**)audioBuffer.get()->getArrayOfReadPointers(), numChannels, bufferPointer, finalBufferSize);
		if (isPostProcess)
		{
			postProcessLoudnessMeter->processBlock(workBuffer);
		}
		else
		{
			preProcessLoudnessMeter->processBlock(workBuffer);
		}

		runPostProcess();

		activeIndex++;
		processNextFile();
	}

	AudioSampleBuffer workBuffer(numChannels, samplesPerBlock);
	workBuffer.setDataToReferTo((float**)audioBuffer.get()->getArrayOfReadPointers(), numChannels, bufferPointer, samplesPerBlock);
	if (isPostProcess)
	{
		postProcessLoudnessMeter->processBlock(workBuffer);
	}
	else
	{
		preProcessLoudnessMeter->processBlock(workBuffer);
	}
	bufferPointer += samplesPerBlock;
}

void OfflineLoudnessProcessor::runPostProcess()
{
	if (isPostProcess == true)
	{
		// finished
		isPostProcess = false;
	}

	isPostProcess = true;
	applyGain(audioBuffer.get());
	applyBrickwallLimiter(audioBuffer.get());
	readPostProcessLoudness();

	//writeOutputFile(audioBuffer.get());
}
void OfflineLoudnessProcessor::applyGain(AudioSampleBuffer* audioBuffer)
{
	float fileDbLufs = preProcessLoudnessMeter->getIntegratedLoudness();
	float dbDifference = (fileDbLufs * -1) - (dBLufsTarget * -1);
	float gainFactor = Decibels::decibelsToGain(dbDifference);
	audioBuffer->applyGain(gainFactor);
}
void OfflineLoudnessProcessor::applyBrickwallLimiter(AudioSampleBuffer* audioBuffer)
{
	int numSamples = audioBuffer->getNumSamples();
	float initialCeiling = dBLimiterCeiling;
	float initialMax = audioBuffer->getMagnitude(0, numSamples);

	float db_1 = Decibels::decibelsToGain(-1.0f);
	float dbFS = Decibels::decibelsToGain(initialCeiling);

	limiterPlugin->setNonRealtime(true);
	limiterPlugin->setParameter(0, dbFS);	// Threshold
	limiterPlugin->setParameter(1, dbFS);	// Ceiling
	limiterPlugin->setParameter(2, 200.0f);	// Release
	limiterPlugin->setParameter(3, false);	// Auto Release

	limiterPlugin->prepareToPlay(fileSampleRate, numSamples);
	limiterPlugin.get()->processBlock(*audioBuffer, midiBuffer);

	float max = audioBuffer->getMagnitude(0, numSamples);
}

void OfflineLoudnessProcessor::readPostProcessLoudness()
{
	bufferPointer = 0;
	timer.get()->startTimerHz(100.0f);
}

void OfflineLoudnessProcessor::writeOutputFile(AudioSampleBuffer* audioBuffer)
{
	File wavFile = destinationFolder.getChildFile(activeFile.getFileName());
	wavFile.deleteFile();

	FileOutputStream* fos = new FileOutputStream(wavFile);
	WavAudioFormat wavFormat;
	ScopedPointer<AudioFormatWriter> afw(wavFormat.createWriterFor(fos, fileSampleRate, audioBuffer->getNumChannels(), fileBitsPerSample, StringPairArray(), 0));
	afw->writeFromAudioSampleBuffer(*audioBuffer, 0, audioBuffer->getNumSamples());
}
#pragma endregion

#pragma region Load Resources
bool OfflineLoudnessProcessor::loadFileFromDisk(File srcFile)
{
	if (auto* reader = formatManager.createReaderFor(srcFile))
	{
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		readerSource.reset(newSource.release());
		readerSource->setLooping(false);

		fileTotalLength = readerSource->getTotalLength();
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
#pragma endregion
