/*
  ==============================================================================

    PluginWrapperProcessor.cpp
    Created: 13 Sep 2018 11:45:52am
    Author:  Ken

  ==============================================================================
*/

#include "PluginWrapperProcessor.h"

#pragma region AudioParameterWrapper
AudioParameterWrapper::AudioParameterWrapper(AudioProcessorParameter& paramToWrap) : wrap(paramToWrap) {}
float AudioParameterWrapper::getValue() const { return wrap.getValue(); }
void AudioParameterWrapper::setValue(float newValue) { wrap.setValue(newValue); }
float AudioParameterWrapper::getDefaultValue() const { return wrap.getDefaultValue(); }
String AudioParameterWrapper::getName(int maxLen) const { return wrap.getName(maxLen); }
String AudioParameterWrapper::getLabel() const { return wrap.getLabel(); }
int AudioParameterWrapper::getNumSteps() const { return wrap.getNumSteps(); }
bool AudioParameterWrapper::isDiscrete() const { return wrap.isDiscrete(); }

String AudioParameterWrapper::getText(float v, int len) const { return wrap.getText(v, len); }
float AudioParameterWrapper::getValueForText(const String& t) const { return wrap.getValueForText(t); }
String AudioParameterWrapper::getCurrentValueAsText() const { return wrap.getCurrentValueAsText(); }
bool AudioParameterWrapper::isOrientationInverted() const { return wrap.isOrientationInverted(); }
bool AudioParameterWrapper::isAutomatable() const { return wrap.isAutomatable(); }
bool AudioParameterWrapper::isMetaParameter() const { return wrap.isMetaParameter(); }
#pragma endregion

#pragma region PluginWrapperProcessor
void PluginWrapperProcessor::updateParameter()
{
	auto& params = plugin->getParameters();

	for (auto* param : params)
	{
		AudioParameterWrapper* pWrapper = new AudioParameterWrapper(*param);
		addParameter(pWrapper);
	}
}
const String PluginWrapperProcessor::getName() const { return plugin->getName(); }
bool PluginWrapperProcessor::canAddBus(bool inputBus) const { return plugin->canAddBus(inputBus); }
bool PluginWrapperProcessor::canRemoveBus(bool inputBus) const { return plugin->canRemoveBus(inputBus); }
bool PluginWrapperProcessor::supportsDoublePrecisionProcessing() const { return plugin->supportsDoublePrecisionProcessing(); }
double PluginWrapperProcessor::getTailLengthSeconds() const { return plugin->getTailLengthSeconds(); }
bool PluginWrapperProcessor::acceptsMidi() const { return plugin->acceptsMidi(); }
bool PluginWrapperProcessor::producesMidi() const { return plugin->producesMidi(); }
bool PluginWrapperProcessor::supportsMPE() const { return plugin->supportsMPE(); }
bool PluginWrapperProcessor::isMidiEffect() const { return plugin->isMidiEffect(); }
void PluginWrapperProcessor::reset()  { plugin->reset(); }
AudioProcessorEditor* PluginWrapperProcessor::createEditor()  { return plugin->createEditor(); }
bool PluginWrapperProcessor::hasEditor() const { return plugin->hasEditor(); }

int PluginWrapperProcessor::getNumParameters()  { return plugin->getNumParameters(); }

int PluginWrapperProcessor::getNumPrograms()  { return plugin->getNumPrograms(); }
int PluginWrapperProcessor::getCurrentProgram()  { return plugin->getCurrentProgram(); }
void PluginWrapperProcessor::setCurrentProgram(int index)  { plugin->setCurrentProgram(index); }
const String PluginWrapperProcessor::getProgramName(int index)  { return plugin->getProgramName(index); }
void PluginWrapperProcessor::changeProgramName(int index, const String& newName)  { plugin->changeProgramName(index, newName); }
void PluginWrapperProcessor::getStateInformation(juce::MemoryBlock& destData)  { plugin->getStateInformation(destData); }
void PluginWrapperProcessor::getCurrentProgramStateInformation(juce::MemoryBlock& destData)  { plugin->getCurrentProgramStateInformation(destData); }
void PluginWrapperProcessor::setStateInformation(const void* data, int sizeInBytes)  { plugin->setStateInformation(data, sizeInBytes); }
void PluginWrapperProcessor::setCurrentProgramStateInformation(const void* data, int bytes)  { plugin->setCurrentProgramStateInformation(data, bytes); }

//==============================================================================
void PluginWrapperProcessor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
	plugin->releaseResources();

	plugin->setRateAndBufferSizeDetails(sampleRate, maximumExpectedSamplesPerBlock);

	// sync number of buses
	for (int dir = 0; dir < 2; ++dir)
	{
		const bool isInput = (dir == 0);
		int expectedNumBuses = getBusCount(isInput);
		int requiredNumBuses = plugin->getBusCount(isInput);

		for (; expectedNumBuses < requiredNumBuses; expectedNumBuses++)
			plugin->addBus(isInput);

		for (; requiredNumBuses < expectedNumBuses; requiredNumBuses++)
			plugin->removeBus(isInput);
	}

	plugin->setBusesLayout(getBusesLayout());
	plugin->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void PluginWrapperProcessor::releaseResources() { return plugin->releaseResources(); }

//==============================================================================
void PluginWrapperProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)  { plugin->processBlock(buffer, midiMessages); }
void PluginWrapperProcessor::processBlock(AudioBuffer<double>& buffer, MidiBuffer& midiMessages)  { plugin->processBlock(buffer, midiMessages); }
void PluginWrapperProcessor::processBlockBypassed(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)  { plugin->processBlockBypassed(buffer, midiMessages); }
void PluginWrapperProcessor::processBlockBypassed(AudioBuffer<double>& buffer, MidiBuffer& midiMessages)  { plugin->processBlockBypassed(buffer, midiMessages); }

//==============================================================================
bool PluginWrapperProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const { return plugin->checkBusesLayoutSupported(layouts); }
bool PluginWrapperProcessor::canApplyBusesLayout(const BusesLayout& layouts) const { return plugin->setBusesLayout(layouts); }
bool PluginWrapperProcessor::canApplyBusCountChange(bool isInput, bool isAddingBuses, BusProperties& outNewBusProperties)
{
	if (isAddingBuses)
	{
		int busIdx = plugin->getBusCount(isInput);

		if (!plugin->addBus(isInput))
			return false;

		if (Bus* bus = plugin->getBus(isInput, busIdx))
		{
			outNewBusProperties.busName = bus->getName();
			outNewBusProperties.defaultLayout = bus->getDefaultLayout();
			outNewBusProperties.isActivatedByDefault = bus->isEnabledByDefault();

			return true;
		}
		else
		{
			jassertfalse;
			return false;
		}
	}
	else
		return plugin->removeBus(isInput);
}

#pragma endregion


