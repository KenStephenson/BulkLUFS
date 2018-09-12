/*
  ==============================================================================

    OfflineLoudnessScanManager.h
    Created: 11 Sep 2018 5:30:48pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "./OfflineLoudnessProcessor.h"

class ScanListener
{
public:
	ScanListener() {};
	~ScanListener() {};
	virtual void ScanCompleted() {};
};

class OfflineLoudnessScanManager : public Thread::Listener, public ScanListener
{
	public:
		OfflineLoudnessScanManager() {};
		~OfflineLoudnessScanManager() {};

		void runScan(OfflineLoudnessScanDataPacket* _scanData, ScanListener* _viewListener)
		{
			viewListener = _viewListener;
			scanThread = std::make_unique<OfflineLoudnessProcessor>(_scanData);
			scanThread->addListener(this);
			scanThread->startThread();
		}
	
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
				viewListener->ScanCompleted();
			}
		}

		std::unique_ptr<OfflineLoudnessProcessor> scanThread = nullptr;
		ScanListener* viewListener = nullptr;
};