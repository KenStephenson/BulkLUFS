/*
  ==============================================================================

    Screen.h
    Created: 1 Sep 2018 4:03:11pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

using namespace std;

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

const int XMargin = 6;
const int YMargin = 4;
const Colour gridColour = Colours::slategrey;

class AppLookAndFeel : public LookAndFeel_V4
{
	public:
		AppLookAndFeel()
		{
			btnFont = make_unique<Font>(14, Font::bold);

			ColourScheme& currentScheme = getCurrentColourScheme();
			currentScheme.setUIColour(LookAndFeel_V4::ColourScheme::UIColour::windowBackground, Colours::aliceblue);
			currentScheme.setUIColour(LookAndFeel_V4::ColourScheme::UIColour::outline, Colours::aliceblue);
			currentScheme.setUIColour(LookAndFeel_V4::ColourScheme::UIColour::defaultFill, Colours::aliceblue);
			setColourScheme(currentScheme);

			setColour(Label::textColourId, Colours::white);

			setColour(TextButton::buttonColourId, Colours::white);
			setColour(TextButton::textColourOnId, Colours::black);
			setColour(TextButton::textColourOffId, Colours::black);

		}
		~AppLookAndFeel()
		{
			btnFont = nullptr;
		}
		Font getTextButtonFont(TextButton& tbn, int buttonHeight) override
		{
			return *btnFont.get();
		}
		Font getLabelFont(Label& tbn) override
		{
			return *btnFont.get();
		}

	private:
		unique_ptr<Font> btnFont;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppLookAndFeel)
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
