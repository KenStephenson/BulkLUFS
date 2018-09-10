/*
  ==============================================================================

    OfflineLoudnessProcessor.cpp
    Created: 10 Sep 2018 11:50:05am
    Author:  Ken

  ==============================================================================
*/

#include "OfflineLoudnessProcessor.h"

OfflineLoudnessProcessor::OfflineLoudnessProcessor()
	: ThreadWithProgressWindow("Processing...", true, true)
{
}

OfflineLoudnessProcessor::~OfflineLoudnessProcessor()
{
}

void OfflineLoudnessProcessor::run(FileListBoxModel fileList)
{
	//for (int i = 0; i < thingsToDo; ++i)
	//{
	//	// must check this as often as possible, because this is
	//	// how we know if the user's pressed 'cancel'
	//	if (threadShouldExit())
	//		break;
	//	// this will update the progress bar on the dialog box
	//	setProgress(i / (double)thingsToDo);
	//	//   ... do the business here...

	//}
}