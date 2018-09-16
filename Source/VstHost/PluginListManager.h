/*
  ==============================================================================

    PluginListManager.h
    Created: 16 Sep 2018 12:55:01pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#ifndef PLUGINLISTWINDOW_H_INCLUDED
#define PLUGINLISTWINDOW_H_INCLUDED

#include "JuceHeader.h"

class PluginListManager : private ChangeListener
{
public:
	// The constructor loads the saved list,
	// relying on the ApplicationProperties to be initialized.
	PluginListManager(KnownPluginList*, ApplicationProperties*);
	~PluginListManager();

	void showWindow();

	AudioPluginFormatManager formatManager;

private:
	class Window;

	void changeListenerCallback(ChangeBroadcaster*) override;

	KnownPluginList* list_;
	ApplicationProperties* props_;
	ScopedPointer<DocumentWindow> window_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListManager)
};

#endif  // PLUGINLISTWINDOW_H_INCLUDED
