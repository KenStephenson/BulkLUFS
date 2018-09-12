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
	g.drawRect(topPanel.getLocalBounds());
	g.drawRect(fileTable.getLocalBounds());
}
void MainComponent::resized()
{
	Grid grid;

	using Track = Grid::TrackInfo;
	grid.rowGap = 6_px;
	grid.columnGap = 6_px;
	grid.templateRows = { Track(1_fr), Track(6_fr) };
	grid.templateColumns = { Track(1_fr) };
	grid.items = {GridItem(topPanel), GridItem(fileTable) };
	grid.performLayout(getLocalBounds());
}
void MainComponent::initialiseUserInterface()
{
	inputListModel = std::make_unique<FileListBoxModel>();

	addAndMakeVisible(topPanel);
	addAndMakeVisible(fileTable);

	topPanel.btnAddFiles.onClick = [this] { addFilesButtonClicked(); };
	topPanel.btnDestFolder.onClick = [this] { destinationFolderButtonClicked(); };
	topPanel.btnRunProcess.onClick = [this] { runProcessButtonClicked(); };

	inputListModel.get()->setListener(this, tagInputList);
	fileTable.listInputFiles.setModel(inputListModel.get());
}
void MainComponent::refreshFileTableModel(String tag)
{
	if (tag == tagInputList)
	{
		fileTable.listInputFiles.updateContent();
	}
};
void MainComponent::addFilesButtonClicked()
{
	FileChooser chooser("Select files to process...", File(), "*.wav");
	if (chooser.browseForMultipleFilesToOpen())
	{
		for (int i = 0; i < chooser.getResults().size(); i++)
		{
			File f = chooser.getResults()[i];
			OfflineLoudnessScanDataPacket* scanData = new OfflineLoudnessScanDataPacket();
			scanData->rowNo = i;
			scanData->file = f;
			inputListModel->data.add(scanData);
		}
		inputFolder = inputListModel->data[0]->file.getParentDirectory();
	}
	fileTable.listInputFiles.updateContent();
}
void MainComponent::destinationFolderButtonClicked()
{
	FileChooser chooser("Select output folder...", inputFolder, "*.*");
	if (chooser.browseForDirectory())
	{
		destinationFolder = chooser.getResult();
		topPanel.lblDestFolder.setText(destinationFolder.getFileNameWithoutExtension(), dontSendNotification);
	}
	else
	{
		topPanel.lblDestFolder.setText(topPanel.tagNoDestinationFolder, dontSendNotification);
	}
}
void MainComponent::runProcessButtonClicked()
{
	runProcess();
}
bool MainComponent::validateProcessorParameters()
{
	dBLufsTarget = (float)topPanel.sldLUFSTarget.getValue();
	dBLimiterCeiling = (float)topPanel.sldLimiterCeiling.getValue() / 2;

	if (inputListModel->getNumRows() == 0)
	{
		AlertWindow dlg("Parameter Error", "No Input Files", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return false;
	}
	
	writeFile = topPanel.lblDestFolder.getText() != topPanel.tagNoDestinationFolder ? true : false;
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
		percent = ((double)activeScanIndex / (double)inputListModel->getNumRows());
	}
	topPanel.progressValue = percent;
}
#pragma endregion

#pragma region Run Batch Audio Processing
void MainComponent::runProcess()
{
	if (validateProcessorParameters() == false)
	{
		return;
	}
	topPanel.setEnableState(false);

	activeScanIndex = 0;
	startNextLoudnessScan();
}
void MainComponent::startNextLoudnessScan()
{
	if (activeScanIndex >= inputListModel->data.size())
	{
		// completed
		activeScanIndex = 0;
		updateProgressPercentage();
		scanMgr = nullptr;
		topPanel.setEnableState(true);
		return;
	}
	updateProgressPercentage();

	activeScanItem = inputListModel->data[activeScanIndex];
	activeScanItem->dBLufsTarget = dBLufsTarget;
	activeScanItem->dBLimiterCeiling = dBLimiterCeiling;
	activeScanItem->destinationFolder = destinationFolder;
	activeScanItem->writeFile = writeFile;

	scanMgr = std::make_unique<OfflineLoudnessScanManager>();
	scanMgr->runScan(activeScanItem, this);
}

void MainComponent::ScanCompleted()
{
	fileTable.listInputFiles.repaintRow(activeScanItem->rowNo);
	activeScanIndex++;
	startNextLoudnessScan();
}
#pragma endregion
