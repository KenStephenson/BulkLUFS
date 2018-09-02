/*
  ==============================================================================

    Screen.h
    Created: 1 Sep 2018 4:03:11pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

static class ColourFactory
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
			LIST_OUTPUTS_COLOUR = 7,
		};
		static Colour getColour(ThemeComponent item)
		{
			Colour c;
			switch (item)
			{
			case ThemeComponent::PANEL_BK_COLOUR:
				c = Colours::lightgrey;
				break;
			case ThemeComponent::LABEL_BK_COLOUR:
				c = Colours::lightgrey;
				break;
			case ThemeComponent::LABEL_TEXT_COLOUR:
				c = Colours::black;
				break;
			case ThemeComponent::BUTTON_BK_COLOUR:
				c = Colours::slategrey;
				break;
			case ThemeComponent::BUTTON_TEXT_COLOUR:
				c = Colours::white;
				break;
			case ThemeComponent::LIST_INPUTS_COLOUR:
				c = Colours::aliceblue;
				break;
			case ThemeComponent::LIST_OUTPUTS_COLOUR:
				c = Colours::lightgoldenrodyellow;
				break;
			case ThemeComponent::LIST_OUTLINE_COLOUR:
				c = Colours::black;
				break;
			}
			return c;
		}
};

class InputPanel : public Component
{
	public:

		InputPanel()
		{
			using theme = ColourFactory::ThemeComponent;

			backgroundColour = ColourFactory::getColour(theme::PANEL_BK_COLOUR);

			btnAddFiles.setButtonText("1 - Add Files");
			btnAddFiles.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
			btnAddFiles.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
			btnAddFiles.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));

			listInputFiles.setColour(ListBox::backgroundColourId, ColourFactory::getColour(theme::LIST_INPUTS_COLOUR));
			listInputFiles.setColour(ListBox::outlineColourId, ColourFactory::getColour(theme::LIST_OUTLINE_COLOUR));

			btnDestFolder.setButtonText("2 - Select Output Folder");
			btnDestFolder.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
			btnDestFolder.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
			btnDestFolder.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));

			lblDestFolder.setText(tagNoDestinationFolder, dontSendNotification);
			lblDestFolder.setColour(Label::textColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));
			lblDestFolder.setColour(Label::backgroundColourId, ColourFactory::getColour(theme::LABEL_BK_COLOUR));
			lblDestFolder.setJustificationType(Justification::centred);

			addAndMakeVisible(&btnAddFiles);
			addAndMakeVisible(&btnDestFolder);
			addAndMakeVisible(&lblDestFolder);
			addAndMakeVisible(&listInputFiles);
		}
		~InputPanel()
		{
		}
		void paint(Graphics& g) override
		{
			g.fillAll(backgroundColour);
		}

		void resized() override
		{
			//==============================================================================
			FlexBox fbLeftPanel;
			fbLeftPanel.flexWrap = FlexBox::Wrap::wrap;
			fbLeftPanel.justifyContent = FlexBox::JustifyContent::spaceBetween;
			fbLeftPanel.flexDirection = FlexBox::Direction::column;

			fbLeftPanel.items.add(FlexItem(btnAddFiles).withMinHeight(50.0f).withMaxHeight(50.0f).withMinWidth(50.0f).withFlex(1));
			fbLeftPanel.items.add(FlexItem(listInputFiles).withMinHeight(450.0f).withMinWidth(50.0f).withFlex(1));
			fbLeftPanel.items.add(FlexItem(btnDestFolder).withMinHeight(50.0f).withMaxHeight(50.0f).withMinWidth(100.0f).withFlex(1));
			fbLeftPanel.items.add(FlexItem(lblDestFolder).withMinHeight(50.0f).withMaxHeight(50.0f).withMinWidth(100.0f).withFlex(1));

			//==============================================================================
			FlexBox fb;
			fb.flexWrap = FlexBox::Wrap::noWrap;
			fb.items.add(FlexItem(fbLeftPanel).withFlex(2.5));
			fb.performLayout(getLocalBounds().toFloat());
		}


		const String tagNoDestinationFolder = "No Folder Selected...";
		Colour backgroundColour;
		TextButton btnAddFiles;
		TextButton btnDestFolder;
		Label lblDestFolder;
		ListBox listInputFiles;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputPanel)
};
class ControlsPanel : public Component
{
	public:

		ControlsPanel()
		{
			using theme = ColourFactory::ThemeComponent;

			backgroundColour = ColourFactory::getColour(theme::PANEL_BK_COLOUR);

			btnRunProcess.setButtonText("3 - RUN PROCESS");
			btnRunProcess.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
			btnRunProcess.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
			btnRunProcess.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));

			lblLUFSTarget.setText("TARGET LUFS", dontSendNotification);
			lblLUFSTarget.setColour(Label::textColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));
			lblLUFSTarget.setColour(Label::backgroundColourId, ColourFactory::getColour(theme::LABEL_BK_COLOUR));
			lblLUFSTarget.setJustificationType(Justification::centred);
			sldLUFSTarget.setRange(-23.0f, -10.0f);

			sldLUFSTarget.setValue(-15.0f);
			sldLUFSTarget.setTextValueSuffix(" LUFS");
			sldLUFSTarget.setNumDecimalPlacesToDisplay(1);
			sldLUFSTarget.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxRight, true, 120, 20);
			sldLUFSTarget.setColour(Slider::textBoxTextColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));

			lblLimiterCeiling.setText("LIMITER CEILING", dontSendNotification);
			lblLimiterCeiling.setColour(Label::textColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));
			lblLimiterCeiling.setColour(Label::backgroundColourId, ColourFactory::getColour(theme::LABEL_BK_COLOUR));
			lblLimiterCeiling.setJustificationType(Justification::centred);

			sldLimiterCeiling.setRange(-3.0f, 0.0f);
			sldLimiterCeiling.setValue(-1.0f);
			sldLimiterCeiling.setTextValueSuffix(" dB");
			sldLimiterCeiling.setNumDecimalPlacesToDisplay(1);
			sldLimiterCeiling.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxRight, true, 120, 20);
			sldLimiterCeiling.setColour(Slider::textBoxTextColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));

			progressBar = std::make_unique<ProgressBar>(progressValue);
			progressBar->setPercentageDisplay(true);

			listOutputFiles.setColour(ListBox::backgroundColourId, ColourFactory::getColour(theme::LIST_OUTPUTS_COLOUR));
			listOutputFiles.setColour(ListBox::outlineColourId, ColourFactory::getColour(theme::LIST_OUTLINE_COLOUR));

			addAndMakeVisible(&btnRunProcess);
			addAndMakeVisible(&lblLUFSTarget);
			addAndMakeVisible(&sldLUFSTarget);
			addAndMakeVisible(&lblLimiterCeiling);
			addAndMakeVisible(&sldLimiterCeiling);
			addAndMakeVisible(*progressBar.get());
		
			addAndMakeVisible(&listOutputFiles);
		}
		~ControlsPanel()
		{
		}
		void paint(Graphics& g) override
		{
			g.fillAll(backgroundColour);
		}

		void resized() override
		{
			//==============================================================================
			FlexBox fbCentrePanel;
			fbCentrePanel.flexWrap = FlexBox::Wrap::wrap;
			fbCentrePanel.justifyContent = FlexBox::JustifyContent::center;
			fbCentrePanel.flexDirection = FlexBox::Direction::column;

			fbCentrePanel.items.add(FlexItem(btnRunProcess).withMinHeight(50.0f).withMaxHeight(50.0f).withMinWidth(100.0f).withFlex(1));
			fbCentrePanel.items.add(FlexItem(lblLUFSTarget).withMinHeight(30.0f).withMaxHeight(30.0f).withMinWidth(100.0f).withFlex(1));
			fbCentrePanel.items.add(FlexItem(sldLUFSTarget).withMinHeight(50.0f).withMaxHeight(50.0f).withMinWidth(100.0f).withFlex(1));
			fbCentrePanel.items.add(FlexItem(lblLimiterCeiling).withMinHeight(30.0f).withMaxHeight(30.0f).withMinWidth(100.0f).withFlex(1));
			fbCentrePanel.items.add(FlexItem(sldLimiterCeiling).withMinHeight(50.0f).withMaxHeight(50.0f).withMinWidth(100.0f).withFlex(1));
			fbCentrePanel.items.add(FlexItem(*progressBar.get()).withMinHeight(12.0f).withMaxHeight(12.0f).withMinWidth(100.0f).withFlex(1));
			fbCentrePanel.items.add(FlexItem(listOutputFiles).withMinHeight(200.0f).withMinWidth(100.0f).withFlex(1));

			//==============================================================================
			FlexBox fb;
			fb.flexWrap = FlexBox::Wrap::noWrap;
			fb.items.add(FlexItem(fbCentrePanel).withFlex(2.5));
			fb.performLayout(getLocalBounds().toFloat());
		}

		Colour backgroundColour;
		TextButton btnRunProcess;
		Label lblLUFSTarget;
		Slider sldLUFSTarget;
		Label lblLimiterCeiling;
		Slider sldLimiterCeiling;
		ListBox listOutputFiles;

		double progressValue;
		std::unique_ptr<ProgressBar> progressBar;

		const String tagSliderIdLUFS = "LUFS";
		const String tagSliderIdCeiling = "CEILING";
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlsPanel)
};

#pragma endregion
