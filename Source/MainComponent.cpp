/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	initialiseUserInterface();

	//loadLimiterPlugin();

	//preProcessLoudnessMeter = std::make_unique<Ebu128LoudnessMeter>();
	//formatManager.registerBasicFormats();

	setSize(1000, 600);
}

MainComponent::~MainComponent()
{
}

#pragma region User Interface
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(Colours::lightgrey);

    // You can add your drawing code here!
	g.setColour(Colours::black);
	g.drawRect(leftPanel.getLocalBounds());
	g.drawRect(mainPanel.getLocalBounds());
}
void MainComponent::resized()
{
	Grid grid;
	using Track = Grid::TrackInfo;

	grid.templateRows = { Track(1_fr) };
	grid.templateColumns = { Track(3_fr), Track(1_fr) };
	grid.items = { GridItem(leftPanel), GridItem(mainPanel), };
	grid.performLayout(getLocalBounds());
}
void MainComponent::initialiseUserInterface()
{
	inputListModel = std::make_unique<FileListBoxModel>();

	addAndMakeVisible(leftPanel);
	addAndMakeVisible(mainPanel);

	leftPanel.btnAddFiles.onClick = [this] { addFilesButtonClicked(); };
	mainPanel.btnDestFolder.onClick = [this] { destinationFolderButtonClicked(); };
	mainPanel.btnRunProcess.onClick = [this] { runProcessButtonClicked(); };

	inputListModel.get()->setListener(this, tagInputList);
	leftPanel.listInputFiles.setModel(inputListModel.get());
}
void MainComponent::refreshFileTableModel(String tag)
{
	if (tag == tagInputList)
	{
		leftPanel.listInputFiles.updateContent();
	}
};
void MainComponent::addFilesButtonClicked()
{
	FileChooser chooser("Select files to process...", File::nonexistent, "*.wav");
	if (chooser.browseForMultipleFilesToOpen())
	{
		for (int i = 0; i < chooser.getResults().size(); i++)
		{
			File f = chooser.getResults()[i];
			inputListModel->data.add(new FileLoudnessDetails(i, f));
		}
		inputFolder = inputListModel->data[0]->file.getParentDirectory();
	}
	leftPanel.listInputFiles.updateContent();
}
void MainComponent::destinationFolderButtonClicked()
{
	FileChooser chooser("Select output folder...", inputFolder, "*.*");
	if (chooser.browseForDirectory())
	{
		destinationFolder = chooser.getResult();
		mainPanel.lblDestFolder.setText(destinationFolder.getFileNameWithoutExtension(), dontSendNotification);
	}
	else
	{
		mainPanel.lblDestFolder.setText(mainPanel.tagNoDestinationFolder, dontSendNotification);
	}
}
void MainComponent::runProcessButtonClicked()
{
	runProcess();
}
bool MainComponent::validateProcessorParameters()
{
	if (inputListModel->getNumRows() == 0)
	{
		AlertWindow dlg("Parameter Error", "No Input Files", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return false;
	}
	
	writeFile = mainPanel.lblDestFolder.getText() != mainPanel.tagNoDestinationFolder ? true : false;
	if (writeFile && destinationFolder == inputFolder)
	{
		AlertWindow dlg("Parameter Error", "Input and Output folders are the same", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return false;
	}
	return true;
}
void MainComponent::updateProgressPercentage()
{
	double percent = 0;
	if (inputListModel->getNumRows() > 0)
	{
		percent = ((double)activeIndex / (double)inputListModel->getNumRows());
	}
	mainPanel.progressValue = percent;
}
#pragma endregion

//#pragma region Run Batch Audio Processing
void MainComponent::runProcess()
{
	if (validateProcessorParameters() == false)
	{
		return;
	}
	dBLufsTarget = mainPanel.sldLUFSTarget.getValue();
	dBLimiterCeiling = mainPanel.sldLimiterCeiling.getValue();
	leftPanel.setEnableState(false);
	mainPanel.setEnableState(false);
	activeIndex = 0;
	startProcess();
}
void MainComponent::startProcess()
{
	if (activeIndex >= inputListModel->data.size())
	{
		// completed
		updateProgressPercentage();
		leftPanel.setEnableState(true);
		mainPanel.setEnableState(true);
		if (scanThread != nullptr && scanThread->isThreadRunning())
		{
			scanThread->stopThread(100);
		}
		return;
	}
	activeFileDetail = inputListModel->data[activeIndex];
	activeFileDetail->dBLufsTarget = dBLufsTarget;
	activeFileDetail->dBLimiterCeiling = dBLimiterCeiling;
	activeFileDetail->destinationFolder = destinationFolder;
	activeFileDetail->writeFile = writeFile;

	updateProgressPercentage();

	scanThread = std::make_unique<OfflineLoudnessProcessor>(activeFileDetail);
	scanThread->addListener(this);
	scanThread->startThread();
}

void MainComponent::exitSignalSent()
{
	leftPanel.listInputFiles.repaintRow(activeFileDetail->rowNo);
	scanThread->removeListener(this);
	activeIndex++;
	startProcess();
}
