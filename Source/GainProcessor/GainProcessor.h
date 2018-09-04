/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioProcessorGraphTutorial
 version:          2.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Explores the audio processor graph.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 type:             Component
 mainClass:        MainComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/
#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class ProcessorBase  : public AudioProcessor
{
public:
    //==============================================================================
    ProcessorBase()  {}
    ~ProcessorBase() {}
    
    //==============================================================================
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (AudioSampleBuffer&, MidiBuffer&) override {}
    
    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return nullptr; }
    bool hasEditor() const override                        { return false; }
    
    //==============================================================================
    const String getName() const override                  { return {}; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }
    
    //==============================================================================
    int getNumPrograms() override                          { return 0; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const String&) override   {}
    
    //==============================================================================
    void getStateInformation (MemoryBlock&) override       {}
    void setStateInformation (const void*, int) override   {}
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorBase)
};

//==============================================================================
class GainProcessor  : public ProcessorBase
{
public:
    GainProcessor()
    {
        gain.setGainDecibels (-6.0f);
    }
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
        gain.prepare (spec);
    }
    
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        dsp::AudioBlock<float> block (buffer);
        dsp::ProcessContextReplacing<float> context (block);
        gain.process (context);
    }
    
    void reset() override
    {
        gain.reset();
    }
    
    const String getName() const override { return "Gain"; }

private:
    dsp::Gain<float> gain;
};
//==============================================================================
class FilterProcessor : public ProcessorBase
{
public:
	FilterProcessor() {}

	void prepareToPlay(double sampleRate, int samplesPerBlock) override
	{
		*filter.state = *dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 1000.0f);

		dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
		filter.prepare(spec);
	}

	void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override
	{
		dsp::AudioBlock<float> block(buffer);
		dsp::ProcessContextReplacing<float> context(block);
		filter.process(context);
	}

	void reset() override
	{
		filter.reset();
	}

	const String getName() const override { return "Filter"; }

private:
	dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> filter;
};
