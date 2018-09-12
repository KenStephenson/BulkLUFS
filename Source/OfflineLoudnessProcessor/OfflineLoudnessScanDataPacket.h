/*
  ==============================================================================

    OfflineLoudnessScanItem.h
    Created: 12 Sep 2018 12:41:21pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
struct OfflineLoudnessScanDataPacket
{
public:
	// Input Parameters
	int rowNo = 0;
	File file;
	File destinationFolder;
	float dBLufsTarget;
	float dBLimiterCeiling;
	bool writeFile = false;

	// Output Results
	float preIntegratedLufs = 0;
	float prePeakDbfs = 0;
	float diffLufs = 0;
	float gain = 0;
	float postPeakDbfs = 0;
	float postIntegratedLufs = 0;
	float postShortTermLoudness = 0;
	float postMaximumShortTermLoudness = 0;
	float postMomentaryLoudness = 0;
	float postMaximumMomentaryLoudness = 0;
	float postLoudnessRangeStart = 0;
	float postLoudnessRangeEnd = 0;
	float postLoudnessRange = 0;
};