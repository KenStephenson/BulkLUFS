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

	setSize(1000, 600);
}
MainComponent::~MainComponent()
{
}
void MainComponent::closeApp()
{
	if (offlineLoudnessScanThread != nullptr)
	{
		offlineLoudnessScanThread->shutDownThread();
	}
}

#pragma region User Interface
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(Colours::lightgrey);

    // You can add your drawing code here!
	g.setColour(Colours::black);
	g.drawRect(topPanel.getLocalBounds());
	g.drawRect(fileTablePanel.getLocalBounds());
}
void MainComponent::resized()
{
	Grid grid;

	using Track = Grid::TrackInfo;
	grid.rowGap = 6_px;
	grid.columnGap = 6_px;
	grid.templateRows = { Track(2_fr), Track(12_fr), Track(1_fr) };
	grid.templateColumns = { Track(1_fr) };
	grid.items = { GridItem(topPanel), GridItem(fileTablePanel), GridItem(footerPanel) };
	grid.performLayout(getLocalBounds());
}
void MainComponent::initialiseUserInterface()
{
	filesToProcesstListModel = std::make_unique<FileListBoxModel>();

	addAndMakeVisible(topPanel);
	addAndMakeVisible(fileTablePanel);
	addAndMakeVisible(footerPanel);

	topPanel.btnAddFiles.onClick = [this] { addFilesButtonClicked(); };
	topPanel.btnDestFolder.onClick = [this] { destinationFolderButtonClicked(); };
	topPanel.btnRunProcess.onClick = [this] { runProcessButtonClicked(); };
	footerPanel.btnClearFiles.onClick = [this] { clearFilesButtonClicked(); };
	footerPanel.btnResetFiles.onClick = [this] { resetFilesButtonClicked(); };

	filesToProcesstListModel.get()->setListener(this, tagInputList);
	fileTablePanel.listInputFiles.setModel(filesToProcesstListModel.get());
}
void MainComponent::addFilesButtonClicked()
{
	FileChooser chooser("Select files to process...", File(), "*.wav");
	if (chooser.browseForMultipleFilesToOpen())
	{
		for (int i = 0; i < chooser.getResults().size(); i++)
		{
			File f = chooser.getResults()[i];
			std::shared_ptr<OfflineLoudnessScanDataPacket> scanData = std::make_shared<OfflineLoudnessScanDataPacket>();
			scanData->rowNo = i;
			scanData->file = f;
			filesToProcesstListModel->addFile(scanData);
		}
		inputFolder = filesToProcesstListModel->getParentDirectory();
	}
	fileTablePanel.listInputFiles.updateContent();
}
void MainComponent::destinationFolderButtonClicked()
{
	FileChooser chooser("Select output folder...", inputFolder, "*.wav");
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
	if (offlineLoudnessScanThread == nullptr)
	{
		runProcess();
	}
	else
	{
		stopProcess();
	}
}

void MainComponent::clearFilesButtonClicked()
{
	filesToProcesstListModel->clearFiles();
	fileTablePanel.listInputFiles.updateContent();
}

void MainComponent::resetFilesButtonClicked()
{
	filesToProcesstListModel->resetFiles(fileTablePanel.listInputFiles);
}

bool MainComponent::validateProcessorParameters()
{
	dBLufsTarget = (float)topPanel.sldLUFSTarget.getValue();
	dBLimiterCeiling = (float)topPanel.sldLimiterCeiling.getValue() / 2;

	if (filesToProcesstListModel->getNumRows() == 0)
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
	if (filesToProcesstListModel->getNumRows() > 0)
	{
		percent = ((double)activeScanIndex / (double)filesToProcesstListModel->getNumRows());
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
	footerPanel.setEnableState(false);

	cancelRequest = false;
	activeScanIndex = 0;
	startNextLoudnessScan();
}

void MainComponent::stopProcess()
{
	cancelRequest = true;
	offlineLoudnessScanThread->shutDownThread();
}

void MainComponent::startNextLoudnessScan()
{
	if (activeScanIndex >= filesToProcesstListModel->getNumRows())
	{
		activeScanIndex = 0;
		updateProgressPercentage();
		offlineLoudnessScanThread = nullptr;
		topPanel.setEnableState(true);
		footerPanel.setEnableState(true);
		return;
	}
	updateProgressPercentage();

	activeOfflineLoudnessScanItem = filesToProcesstListModel->getFile(activeScanIndex);
	activeOfflineLoudnessScanItem->dBLufsTarget = dBLufsTarget;
	activeOfflineLoudnessScanItem->dBLimiterCeiling = dBLimiterCeiling;
	activeOfflineLoudnessScanItem->destinationFolder = destinationFolder;
	activeOfflineLoudnessScanItem->writeFile = writeFile;

	offlineLoudnessScanThread = std::make_unique<OfflineLoudnessScanThread>();
	offlineLoudnessScanThread->runScan(activeOfflineLoudnessScanItem, this);
}

void MainComponent::ScanCompleted()
{
	if (cancelRequest)
	{
		cancelRequest = false;
		filesToProcesstListModel->resetFiles(fileTablePanel.listInputFiles);

		activeScanIndex = 0;
		updateProgressPercentage();
		offlineLoudnessScanThread = nullptr;
		topPanel.setEnableState(true);
		footerPanel.setEnableState(true);
		return;
	}

	fileTablePanel.listInputFiles.repaintRow(activeOfflineLoudnessScanItem->rowNo);
	activeScanIndex++;
	startNextLoudnessScan();
}
#pragma endregion
