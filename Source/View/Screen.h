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

			listInputFiles.setColour(ListBox::backgroundColourId, ColourFactory::getColour(theme::LIST_INPUTS_COLOUR));
			listInputFiles.setColour(ListBox::outlineColourId, ColourFactory::getColour(theme::LIST_OUTLINE_COLOUR));

			addAndMakeVisible(&listInputFiles);
			const int colWIdth = 98;
			listInputFiles.getHeader().addColumn("File", 1, 200, TableHeaderComponent::notSortable);
			
			listInputFiles.getHeader().addColumn("LUFS: In", 2, colWIdth, TableHeaderComponent::notSortable);
			listInputFiles.getHeader().addColumn("Out", 3, colWIdth, TableHeaderComponent::notSortable);
			listInputFiles.getHeader().addColumn("Difference", 4, colWIdth, TableHeaderComponent::notSortable);
			listInputFiles.getHeader().addColumn("Range", 5, colWIdth, TableHeaderComponent::notSortable);
			listInputFiles.getHeader().addColumn("Max Short Term", 6, colWIdth, TableHeaderComponent::notSortable);
			listInputFiles.getHeader().addColumn("Gain [1=0dB]", 7, colWIdth, TableHeaderComponent::notSortable);
			listInputFiles.getHeader().addColumn("Peak dBFS: In", 8, colWIdth, TableHeaderComponent::notSortable);
			listInputFiles.getHeader().addColumn("Out", 9, colWIdth, TableHeaderComponent::notSortable);
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

			fbLeftPanel.items.add(FlexItem(listInputFiles).withMinHeight(550.0f).withMinWidth(50.0f).withFlex(1));

			//==============================================================================
			FlexBox fb;
			fb.flexWrap = FlexBox::Wrap::noWrap;
			fb.items.add(FlexItem(fbLeftPanel).withFlex(2.5));
			fb.performLayout(getLocalBounds().toFloat());
		}

		Colour backgroundColour;
		TableListBox listInputFiles;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputPanel)
};
class ControlsPanel : public Component
{
	public:

		ControlsPanel()
		{
			using theme = ColourFactory::ThemeComponent;

			backgroundColour = ColourFactory::getColour(theme::PANEL_BK_COLOUR);

			btnAddFiles.setButtonText("1 - ADD FILES");
			btnAddFiles.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
			btnAddFiles.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
			btnAddFiles.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));

			btnDestFolder.setButtonText("2 - SELECT OUTPUT FOLDER");
			btnDestFolder.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
			btnDestFolder.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
			btnDestFolder.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));

			lblDestFolder.setText(tagNoDestinationFolder, dontSendNotification);
			lblDestFolder.setColour(Label::textColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));
			lblDestFolder.setColour(Label::backgroundColourId, ColourFactory::getColour(theme::LABEL_BK_COLOUR));
			lblDestFolder.setJustificationType(Justification::centred);

			btnRunProcess.setButtonText("3 - RUN PROCESS");
			btnRunProcess.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
			btnRunProcess.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
			btnRunProcess.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));

			lblLUFSTarget.setText("TARGET LOUDNESS", dontSendNotification);
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
			sldLimiterCeiling.setTextValueSuffix(" dBFS");
			sldLimiterCeiling.setNumDecimalPlacesToDisplay(1);
			sldLimiterCeiling.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxRight, true, 120, 20);
			sldLimiterCeiling.setColour(Slider::textBoxTextColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));

			progressBar = std::make_unique<ProgressBar>(progressValue);
			progressBar->setPercentageDisplay(true);

			addAndMakeVisible(&btnAddFiles);
			addAndMakeVisible(&btnDestFolder);
			addAndMakeVisible(&lblDestFolder);
			addAndMakeVisible(&btnRunProcess);
			addAndMakeVisible(&lblLUFSTarget);
			addAndMakeVisible(&sldLUFSTarget);
			addAndMakeVisible(&lblLimiterCeiling);
			addAndMakeVisible(&sldLimiterCeiling);
			addAndMakeVisible(*progressBar.get());
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

			Grid grid;
			using Track = Grid::TrackInfo;
			grid.rowGap = 6_px;
			grid.columnGap = 6_px;

			grid.templateRows = { Track(1_fr), Track(1_fr) };
			grid.templateColumns = { Track(1_fr), Track(2_fr), Track(1_fr), Track(2_fr), Track(1_fr) };
			grid.items = 
			{ 
				GridItem(lblLUFSTarget) , GridItem(sldLUFSTarget) , GridItem(lblLimiterCeiling) , GridItem(sldLimiterCeiling) , GridItem(*progressBar.get()),
				GridItem(btnAddFiles), GridItem(nullptr), GridItem(btnDestFolder), GridItem(lblDestFolder), GridItem(btnRunProcess),
			};
			grid.performLayout(getLocalBounds());
		}
		void setEnableState(bool state)
		{
			btnAddFiles.setEnabled(state);
			btnDestFolder.setEnabled(state);
			btnRunProcess.setEnabled(state);
			sldLUFSTarget.setEnabled(state);
			sldLimiterCeiling.setEnabled(state);
		}

		Colour backgroundColour;
		TextButton btnAddFiles;
		TextButton btnDestFolder;
		Label lblDestFolder;
		TextButton btnRunProcess;
		Label lblLUFSTarget;
		Slider sldLUFSTarget;
		Label lblLimiterCeiling;
		Slider sldLimiterCeiling;

		double progressValue;
		std::unique_ptr<ProgressBar> progressBar;

		const String tagNoDestinationFolder = "No Folder Selected...";
		const String tagSliderIdLUFS = "LUFS";
		const String tagSliderIdCeiling = "CEILING";
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlsPanel)
};

#pragma endregion
