/*
  ==============================================================================

    OfflineLoudnessProcessor.h
    Created: 10 Sep 2018 11:50:05am
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "../View/FileListBoxModel.h"

class OfflineLoudnessProcessor : public ThreadWithProgressWindow
{
	public:
		OfflineLoudnessProcessor();
		~OfflineLoudnessProcessor();

		void run(FileListBoxModel fileList);
};