/*
  ==============================================================================

    Screen.cpp
    Created: 13 Sep 2018 11:08:04am
    Author:  Ken

  ==============================================================================
*/

#include "Screen.h"

#pragma region Control Panel

ControlsPanel::ControlsPanel()
{
	using theme = ColourFactory::ThemeComponent;

	backgroundColour = ColourFactory::getColour(theme::PANEL_BK_COLOUR);

	btnAddFiles.setButtonText("1 - ADD FILES");
	btnAddFiles.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
	btnAddFiles.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnAddFiles.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnAddFiles.setTooltip("Add the audio files to be processed");

	btnDestFolder.setButtonText("2 - SELECT OUTPUT FOLDER");
	btnDestFolder.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
	btnDestFolder.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnDestFolder.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnDestFolder.setTooltip("If output files are required, select an output folder (other than the source folder). If no folder is selected, no files will be written.");
	
	lblDestFolder.setText(tagNoDestinationFolder, dontSendNotification);
	lblDestFolder.setColour(Label::textColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));
	lblDestFolder.setColour(Label::backgroundColourId, ColourFactory::getColour(theme::LABEL_BK_COLOUR));
	lblDestFolder.setJustificationType(Justification::centred);

	btnRunProcess.setButtonText("3 - START PROCESS");
	btnRunProcess.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
	btnRunProcess.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnRunProcess.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnRunProcess.setTooltip("Start / Stop the analysis process");

	lblLUFSTarget.setText("TARGET LOUDNESS", dontSendNotification);
	lblLUFSTarget.setColour(Label::textColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));
	lblLUFSTarget.setColour(Label::backgroundColourId, ColourFactory::getColour(theme::LABEL_BK_COLOUR));
	lblLUFSTarget.setJustificationType(Justification::centred);
	sldLUFSTarget.setRange(-23.0f, -10.0f);
	sldLUFSTarget.setValue(-14.0f);
	sldLUFSTarget.setTextValueSuffix(" LUFS");
	sldLUFSTarget.setNumDecimalPlacesToDisplay(1);
	sldLUFSTarget.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxRight, true, 120, 20);
	sldLUFSTarget.setColour(Slider::textBoxTextColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));
	sldLUFSTarget.setTooltip("Set the required output loudness - default -14LUFS");

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
	sldLimiterCeiling.setTooltip("Set the limiter ceiling - default -1dbFS");

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
ControlsPanel::~ControlsPanel()
{
}
void ControlsPanel::paint(Graphics& g)
{
	g.fillAll(backgroundColour);
}

void ControlsPanel::resized()
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
void ControlsPanel::setEnableState(bool state)
{
	btnAddFiles.setEnabled(state);
	btnDestFolder.setEnabled(state);
	//btnRunProcess.setEnabled(state);
	sldLUFSTarget.setEnabled(state);
	sldLimiterCeiling.setEnabled(state);

	btnRunProcess.setButtonText(state ? tagProcessStart : tagProcessStop);
}

#pragma endregion

#pragma region File List Panel
FileListPanel::FileListPanel()
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
FileListPanel::~FileListPanel()
{
}
void FileListPanel::paint(Graphics& g) 
{
	g.fillAll(backgroundColour);
}

void FileListPanel::resized() 
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



#pragma endregion

#pragma region Footer Panel
FooterPanel::FooterPanel()
{
	using theme = ColourFactory::ThemeComponent;

	backgroundColour = ColourFactory::getColour(theme::PANEL_BK_COLOUR);

	btnClearFiles.setButtonText("CLEAR FILES");
	btnClearFiles.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
	btnClearFiles.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnClearFiles.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnClearFiles.setTooltip("Clear the files list.");

	btnResetFiles.setButtonText("RESET FILES");
	btnResetFiles.setColour(TextButton::buttonColourId, ColourFactory::getColour(theme::BUTTON_BK_COLOUR));
	btnResetFiles.setColour(TextButton::textColourOnId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnResetFiles.setColour(TextButton::textColourOffId, ColourFactory::getColour(theme::BUTTON_TEXT_COLOUR));
	btnResetFiles.setTooltip("Reset the analysis values in the files list.");

	addAndMakeVisible(&btnClearFiles);
	addAndMakeVisible(&btnResetFiles);

}
FooterPanel::~FooterPanel()
{
}

void FooterPanel::paint(Graphics& g) 
{
	g.fillAll(backgroundColour);
}
void FooterPanel::resized() 
{
	Grid grid;
	using Track = Grid::TrackInfo;
	grid.rowGap = 6_px;
	grid.columnGap = 6_px;

	grid.templateRows = { Track(1_fr) };
	grid.templateColumns = { Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr) };
	grid.items =
	{
		GridItem(btnClearFiles), GridItem(btnResetFiles), GridItem(nullptr), GridItem(nullptr), 
		GridItem(nullptr), GridItem(nullptr), GridItem(nullptr),
	};
	grid.performLayout(getLocalBounds());
}
void FooterPanel::setEnableState(bool state)
{
	btnClearFiles.setEnabled(state);
	btnResetFiles.setEnabled(state);
}
#pragma endregion
