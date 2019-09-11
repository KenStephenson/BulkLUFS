/*
  ==============================================================================

    PluginListManager.cpp
    Created: 16 Sep 2018 12:54:41pm
    Author:  Ken

  ==============================================================================
*/

#include "PluginListManager.h"
class PluginListManager::Window : public DocumentWindow
{
public:
	Window(PluginListManager* manager)
		: DocumentWindow(
			"Available Plugins", Colours::white,
			DocumentWindow::minimiseButton | DocumentWindow::closeButton)
		, manager_(manager)
	{
		const File deadMansPedalFile(
			manager_->props_->getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList"));

		setContentOwned(
			new PluginListComponent(manager_->formatManager, *manager_->list_, deadMansPedalFile, manager_->props_->getUserSettings(), true), true);

		setResizable(true, false);
		setResizeLimits(300, 400, 800, 1500);
		setTopLeftPosition(60, 60);

		restoreWindowStateFromString(manager_->props_->getUserSettings()->getValue("listWindowPos"));
		setVisible(true);
	}

	~Window()
	{
		manager_->props_->getUserSettings()->setValue("listWindowPos", getWindowStateAsString());

		clearContentComponent();
	}

	void closeButtonPressed() override
	{
		manager_->window_ = nullptr;
	}

private:
	PluginListManager* manager_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Window)
};

PluginListManager::PluginListManager(KnownPluginList* list, ApplicationProperties* props)
	: list_(list), props_(props)
{
	{
		std::unique_ptr<XmlElement> savedPluginList(props->getUserSettings()->getXmlValue("pluginList"));
		if (savedPluginList != nullptr)
		{
			list->recreateFromXml(*savedPluginList);
		}
	}

	formatManager.addDefaultFormats();
	list_->addChangeListener(this);
}

PluginListManager::~PluginListManager()
{
	list_->removeChangeListener(this);
}

void PluginListManager::changeListenerCallback(ChangeBroadcaster* source)
{
	jassert(source == list_);
	std::unique_ptr<XmlElement> savedPluginList(list_->createXml());
	if (savedPluginList != nullptr)
	{
		PropertiesFile* userSettings = props_->getUserSettings();
		String str = String("pluginList");
		userSettings->setValue(str, savedPluginList.get());
		props_->saveIfNeeded();
	}
}

void PluginListManager::showWindow()
{
	if (DocumentWindow* win = window_)
		win->toFront(true);
	else
		window_ = new Window(this);
}
