/*
  ==============================================================================

    Screen.h
    Created: 1 Sep 2018 4:03:11pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
class ColourFactory
{
	public:
		enum ThemeComponent
		{
			PANEL_BK_COLOUR = 0,
			LABEL_BK_COLOUR = 1,
			LABEL_TEXT_COLOUR = 2,
			BUTTON_BK_COLOUR = 3,
			BUTTON_TEXT_COLOUR = 4,
			LIST_INPUTS_COLOUR = 5,
			LIST_OUTLINE_COLOUR = 6,
		};
		static Colour getColour(ThemeComponent item)
		{
			Colour c;
			switch (item)
			{
			case ThemeComponent::PANEL_BK_COLOUR:
				c = Colour(0xffd6eaf8);
				break;
			case ThemeComponent::LABEL_BK_COLOUR:
				c = Colour(0xffd6eaf8);
				break;
			case ThemeComponent::LABEL_TEXT_COLOUR:
				c = Colours::black;
				break;
			case ThemeComponent::BUTTON_BK_COLOUR:
				c = Colour(0xffebf5fb);
				break;
			case ThemeComponent::BUTTON_TEXT_COLOUR:
				c = Colours::black;
				break;
			case ThemeComponent::LIST_INPUTS_COLOUR:
				c = Colours::aliceblue;
				break;
			case ThemeComponent::LIST_OUTLINE_COLOUR:
				c = Colours::black;
				break;
			}
			return c;
		}
};


class HeaderPanel : public Component
{
	public:

		HeaderPanel();
		~HeaderPanel();
		void paint(Graphics& g) override;
		void resized() override;
		void setEnableState(bool state);

		TextButton btnAddFiles;
		TextButton btnDestFolder;
		TextButton btnRunProcess;
		Label lblDestFolder;
		std::unique_ptr<ProgressBar> progressBar;
		TextButton btnClearFiles;
		TextButton btnResetFiles;

		const String tagNoDestinationFolder = "No Folder Selected...";
		const String tagProcessStart = "3 - START PROCESS";
		const String tagProcessStop = "3 - STOP PROCESS";

		double progressValue;

	private:
		Colour backgroundColour;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderPanel)
};
class ControlsPanel : public Component
{
	public:

		ControlsPanel();
		~ControlsPanel();
		void paint(Graphics& g) override;
		void resized() override;
		void setEnableState(bool state);

		Slider sldLUFSTarget;
		Slider sldLimiterCeiling;
		TextButton btnLimiterCeiling;

	private:
		Colour backgroundColour;
		Label lblLUFSTarget;

		const String tagSliderIdLUFS = "LUFS";
		const String tagSliderIdCeiling = "CEILING";
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlsPanel)
};
class FileListPanel : public Component
{
	public:
		FileListPanel();
		~FileListPanel();
		void paint(Graphics& g) override;
		void resized() override;

		TableListBox listInputFiles;

private:
	Colour backgroundColour;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileListPanel)
};
#pragma endregion
