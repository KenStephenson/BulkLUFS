/*
  ==============================================================================

    Screen.cpp
    Created: 13 Sep 2018 11:08:04am
    Author:  Ken

  ==============================================================================
*/

#include "Screen.h"
#include "../Model/SessionModel.h"
#include "../Model/TrackModel.h"

#pragma region HeaderPanel Panel
HeaderPanel::HeaderPanel()
{
	using theme = ColourFactory::ThemeComponent;
	backgroundColour = gridColour;

	btnAddFiles.setButtonText("1 - ADD FILES");
	btnAddFiles.setTooltip("Add the audio files to be processed");

	btnDestFolder.setButtonText("2 - SELECT OUTPUT FOLDER");
	btnDestFolder.setTooltip("If output files are required, select an output folder (other than the source folder). If no folder is selected, no files will be written.");

	lblDestFolder.setText(tagNoDestinationFolder, dontSendNotification);
	lblDestFolder.setJustificationType(Justification::centred);

	btnRunProcess.setButtonText("3 - START PROCESS");
	btnRunProcess.setTooltip("Start / Stop the analysis process");

	progressBar = std::make_unique<ProgressBar>(progressValue);
	progressBar->setPercentageDisplay(true);

	btnClearFiles.setButtonText("CLEAR FILES");
	btnClearFiles.setTooltip("Clear the files list.");

	btnResetFiles.setButtonText("RESET FILES");
	btnResetFiles.setTooltip("Reset the analysis values in the files list.");

	addAndMakeVisible(&btnAddFiles);
	addAndMakeVisible(&btnDestFolder);
	addAndMakeVisible(&lblDestFolder);
	addAndMakeVisible(&btnRunProcess);
	addAndMakeVisible(*progressBar.get());
	addAndMakeVisible(&btnClearFiles);
	addAndMakeVisible(&btnResetFiles);
}
HeaderPanel::~HeaderPanel()
{
}
void HeaderPanel::paint(Graphics& g)
{
	g.fillAll(Colours::slategrey);
}
void HeaderPanel::resized()
{
	Grid grid;
	using Track = Grid::TrackInfo;
	grid.setGap(6_px);
	grid.templateRows = { Track(1_fr) };
	grid.templateColumns = { Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr) };
	grid.items =
	{
		GridItem(btnAddFiles), 
		GridItem(btnDestFolder),
		GridItem(lblDestFolder),
		GridItem(btnRunProcess),
		GridItem(*progressBar.get()),
		GridItem(btnClearFiles),
		GridItem(btnResetFiles),
	};
	grid.performLayout(getLocalBounds());

	btnAddFiles.setBounds(btnAddFiles.getBounds().reduced(XMargin, YMargin));
	btnDestFolder.setBounds(btnDestFolder.getBounds().reduced(XMargin, YMargin));
	btnRunProcess.setBounds(btnRunProcess.getBounds().reduced(XMargin, YMargin));
	progressBar.get()->setBounds(progressBar.get()->getBounds().reduced(XMargin, YMargin));
	btnClearFiles.setBounds(btnClearFiles.getBounds().reduced(XMargin, YMargin));
	btnResetFiles.setBounds(btnResetFiles.getBounds().reduced(XMargin, YMargin));
}
void HeaderPanel::setEnableState(bool state)
{
	btnAddFiles.setEnabled(state);
	btnDestFolder.setEnabled(state);
	btnRunProcess.setButtonText(state ? tagProcessStart : tagProcessStop);
	btnClearFiles.setEnabled(state);
	btnResetFiles.setEnabled(state);
}
#pragma endregion

#pragma region Control Panel
ControlsPanel::ControlsPanel()
{
	using theme = ColourFactory::ThemeComponent;

	backgroundColour = gridColour;

	lblLUFSTarget.setText("TARGET LOUDNESS", dontSendNotification);
	lblLUFSTarget.setJustificationType(Justification::centred);

	sldLUFSTarget.setRange(-23.0f, -10.0f);
	sldLUFSTarget.setValue(-14.0f);
	sldLUFSTarget.setTextValueSuffix(" LUFS");
	sldLUFSTarget.setNumDecimalPlacesToDisplay(1);
	sldLUFSTarget.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxRight, true, 120, 20);
	sldLUFSTarget.setColour(Slider::textBoxTextColourId, ColourFactory::getColour(theme::LABEL_TEXT_COLOUR));
	sldLUFSTarget.setTooltip("Set the required output loudness - default -14LUFS");

	btnLimiterCeiling.setButtonText("PEAK LIMITER");
	btnLimiterCeiling.setTooltip("Select and Set a Peak Limiter.");

	addAndMakeVisible(&lblLUFSTarget);
	addAndMakeVisible(&sldLUFSTarget);
	addAndMakeVisible(&btnLimiterCeiling);
}
ControlsPanel::~ControlsPanel()
{
}
void ControlsPanel::paint(Graphics& g)
{
	g.fillAll(Colours::slategrey);
}
void ControlsPanel::resized()
{
	Grid grid;
	using Track = Grid::TrackInfo;
	grid.setGap(6_px);

	grid.templateRows = { Track(1_fr) };
	grid.templateColumns = { Track(1_fr), Track(1_fr), Track(2_fr), Track(1_fr), Track(1_fr), Track(1_fr), };
	grid.items =
	{
		GridItem(nullptr), GridItem(lblLUFSTarget) , GridItem(sldLUFSTarget), GridItem(nullptr), GridItem(btnLimiterCeiling), GridItem(nullptr),
	};
	grid.performLayout(getLocalBounds());

	btnLimiterCeiling.setBounds(btnLimiterCeiling.getBounds().reduced(XMargin, YMargin));
}
void ControlsPanel::setEnableState(bool state)
{
	sldLUFSTarget.setEnabled(state);
	btnLimiterCeiling, setEnabled(state);
}
#pragma endregion

#pragma region File List Panel
FileListPanel::FileListPanel()
{
	using theme = ColourFactory::ThemeComponent;
	using colID = SessionModel::ColumnID;

	backgroundColour = gridColour;

	const int colWIdth = 98;
	listInputFiles.getHeader().addColumn("File", colID::File, 200, TableHeaderComponent::notSortable);
	listInputFiles.getHeader().addColumn("IN [LUFS]", colID::InLufs, colWIdth, TableHeaderComponent::notSortable);
	listInputFiles.getHeader().addColumn("OUT [LUFS]", colID::OutLufs, colWIdth, TableHeaderComponent::notSortable);
	listInputFiles.getHeader().addColumn("Range [LUFS]", colID::Range, colWIdth, TableHeaderComponent::notSortable);
	listInputFiles.getHeader().addColumn("MaxShortTerm", colID::MaxShortTerm, colWIdth, TableHeaderComponent::notSortable);
	listInputFiles.getHeader().addColumn("Diff [LUFS]", colID::Diff, colWIdth, TableHeaderComponent::notSortable);
	listInputFiles.getHeader().addColumn("Gain [1=0dB]", colID::Gain, colWIdth, TableHeaderComponent::notSortable);
	listInputFiles.getHeader().addColumn("PEAK IN [dBFS]", colID::InDbfs, colWIdth, TableHeaderComponent::notSortable);
	listInputFiles.getHeader().addColumn("PEAK OUT [dBFS]", colID::OutDbfs, colWIdth, TableHeaderComponent::notSortable);

	addAndMakeVisible(&listInputFiles);
}
FileListPanel::~FileListPanel()
{
}
void FileListPanel::paint(Graphics& g) 
{
	g.fillAll(Colours::slategrey);
}

void FileListPanel::resized() 
{
	//==============================================================================
	FlexBox fbLeftPanel;
	fbLeftPanel.flexWrap = FlexBox::Wrap::wrap;
	fbLeftPanel.justifyContent = FlexBox::JustifyContent::spaceBetween;
	fbLeftPanel.flexDirection = FlexBox::Direction::column;

	fbLeftPanel.items.add(FlexItem(listInputFiles).withMinHeight(250.0f).withMinWidth(50.0f).withFlex(1));

	//==============================================================================
	FlexBox fb;
	fb.flexWrap = FlexBox::Wrap::noWrap;
	fb.items.add(FlexItem(fbLeftPanel).withFlex(2.5));
	fb.performLayout(getLocalBounds().toFloat());
}



#pragma endregion
