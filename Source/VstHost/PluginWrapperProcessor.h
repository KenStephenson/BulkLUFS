/*
  ==============================================================================

    VstWrapper.h
    Created: 7 Sep 2018 12:59:12pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class AudioParameterWrapper : public AudioProcessorParameter
{
	public:
		AudioParameterWrapper(AudioProcessorParameter& paramToWrap);
		float getValue() const override;
		void setValue(float newValue) override;
		float getDefaultValue() const override;
		String getName(int maxLen) const override;
		String getLabel() const override;
		int getNumSteps() const override;
		bool isDiscrete() const override;

		String getText(float v, int len) const override;
		float getValueForText(const String& t) const override;
		String getCurrentValueAsText() const override;
		bool isOrientationInverted() const override;
		bool isAutomatable() const override;
		bool isMetaParameter() const override;

		Category getCategory() const override { return wrap.getCategory(); }

	private:
		AudioProcessorParameter& wrap;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioParameterWrapper)
};

class PluginWrapperProcessor : public AudioProcessor
{
public:
	PluginWrapperProcessor(AudioPluginInstance* processorToUse)
		: AudioProcessor(getBusesPropertiesFromProcessor(processorToUse)), plugin(processorToUse)
	{
		updateParameter();
	}
	void updateParameter();
	//==============================================================================
	const String getName() const override;
	bool canAddBus(bool inputBus) const override;
	bool canRemoveBus(bool inputBus) const override;
	bool supportsDoublePrecisionProcessing() const override;
	double getTailLengthSeconds() const override;
	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool supportsMPE() const override;
	bool isMidiEffect() const override;
	void reset() override;
	AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;
	
	int getNumParameters() override;
	
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const String getProgramName(int index) override;
	void changeProgramName(int index, const String& newName) override;
	void getStateInformation(juce::MemoryBlock& destData) override;
	void getCurrentProgramStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;
	void setCurrentProgramStateInformation(const void* data, int bytes) override;

	void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
	void releaseResources() override;

	void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;
	void processBlock(AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override;
	void processBlockBypassed(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;
	void processBlockBypassed(AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override;

protected:
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
	bool canApplyBusesLayout(const BusesLayout& layouts) const override;
	bool canApplyBusCountChange(bool isInput, bool isAddingBuses, BusProperties& outNewBusProperties) override;

private:
	static BusesProperties getBusesPropertiesFromProcessor(AudioProcessor* processor)
	{
		BusesProperties retval;

		for (int dir = 0; dir < 2; ++dir)
		{
			const bool isInput = (dir == 0);
			const int n = processor->getBusCount(isInput);

			for (int i = 0; i < n; ++i)
			{
				if (AudioProcessor::Bus* bus = processor->getBus(isInput, i))
				{
					retval.addBus(isInput, bus->getName(), bus->getDefaultLayout(), bus->isEnabledByDefault());
				}
			}
		}

		return retval;
	}
	ScopedPointer<AudioProcessor> plugin;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWrapperProcessor)
};


