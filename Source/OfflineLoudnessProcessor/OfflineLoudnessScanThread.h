/*
  ==============================================================================

    OfflineLoudnessScanThread.h
    Created: 11 Sep 2018 5:30:48pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "./OfflineLoudnessProcessor.h"

class OfflineLoudnessScanListener
{
public:
	OfflineLoudnessScanListener() {};
	~OfflineLoudnessScanListener() {};
	virtual void scanCompleted() {};
};

class OfflineLoudnessScanThread : public Thread::Listener, public OfflineLoudnessScanListener
{
	public:
		OfflineLoudnessScanThread() {};
		~OfflineLoudnessScanThread() {};

		void runScan(std::shared_ptr<TrackModel> _offlineLoudnessScanData, OfflineLoudnessScanListener* _viewListener)
		{
			viewListener = _viewListener;
			scanThread = std::make_unique<OfflineLoudnessProcessor>(_offlineLoudnessScanData);
			scanThread->addListener(this);
			scanThread->startThread();
		}
		void shutDownThread() { scanThread->shutDownThread(); }

	private:
		void exitSignalSent() override
		{
			scanThread->removeListener(this);
			if (scanThread->isThreadRunning())
			{
				scanThread->stopThread(100);
			}
			if (viewListener != nullptr)
			{
				viewListener->scanCompleted();
			}
		}

		std::unique_ptr<OfflineLoudnessProcessor> scanThread = nullptr;
		OfflineLoudnessScanListener* viewListener = nullptr;
};