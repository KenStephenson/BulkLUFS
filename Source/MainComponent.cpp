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
	
	loadLimiterPlugin();

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
	using theme = ColourFactory::ThemeComponent;
	g.fillAll(ColourFactory::getColour(theme::PANEL_BK_COLOUR));

    // You can add your drawing code here!
	g.setColour(Colours::black);
	g.drawRect(controlPanel.getLocalBounds());
	g.drawRect(fileTablePanel.getLocalBounds());
}
void MainComponent::resized()
{
	Grid grid;

	using Track = Grid::TrackInfo;
	grid.setGap(6_px);
	grid.templateRows = { Track(1_fr), Track(12_fr), Track(1_fr), };
	grid.templateColumns = { Track(1_fr) };
	grid.items = { GridItem(headerPanel), GridItem(fileTablePanel), GridItem(controlPanel), };
	grid.performLayout(getLocalBounds());
}
void MainComponent::initialiseUserInterface()
{
	filesToProcesstListModel = std::make_unique<FileListBoxModel>();

	addAndMakeVisible(headerPanel);
	addAndMakeVisible(controlPanel);
	addAndMakeVisible(fileTablePanel);

	headerPanel.btnAddFiles.onClick = [this] { addFilesButtonClicked(); };
	headerPanel.btnDestFolder.onClick = [this] { destinationFolderButtonClicked(); };
	headerPanel.btnRunProcess.onClick = [this] { runProcessButtonClicked(); };
	headerPanel.btnClearFiles.onClick = [this] { clearFilesButtonClicked(); };
	headerPanel.btnResetFiles.onClick = [this] { resetFilesButtonClicked(); };
	controlPanel.btnLimiterCeiling.onClick = [this] { setPeakLimiterClicked(); };

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
		headerPanel.lblDestFolder.setText(destinationFolder.getFileNameWithoutExtension(), dontSendNotification);
	}
	else
	{
		headerPanel.lblDestFolder.setText(headerPanel.tagNoDestinationFolder, dontSendNotification);
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

void MainComponent::setPeakLimiterClicked()
{
    AudioProcessorEditor* editor = limiterPlugin->createEditor();

	DialogWindow::LaunchOptions launchOptions;
	launchOptions.dialogTitle = "Peak Limiter";
	launchOptions.escapeKeyTriggersCloseButton = true;
	launchOptions.resizable = false;
	launchOptions.useNativeTitleBar = true;
	launchOptions.useBottomRightCornerResizer = false;
	launchOptions.componentToCentreAround = this;
	launchOptions.content.set(editor, false);
	launchOptions.runModal();
	delete editor;
}
void MainComponent::loadLimiterPlugin()
{
	limiterPlugin = nullptr;

	KnownPluginList knownPluginList;
	std::unique_ptr<AudioPluginFormat> format = std::make_unique<VSTPluginFormat>();
	FileSearchPath path = format->getDefaultLocationsToSearch();

	// Scan the directory for plugins
	std::unique_ptr<PluginDirectoryScanner> scanner = std::make_unique<PluginDirectoryScanner>(knownPluginList, *format, path, true, File(), false);

	String currentPlugBeingScanned = "----";
	while (currentPlugBeingScanned != "")
	{
		currentPlugBeingScanned = scanner->getNextPluginFileThatWillBeScanned();
		File f(currentPlugBeingScanned);
		String plugName = f.getFileNameWithoutExtension();
		if (plugName == limiterPluginName || plugName == limiterPluginName64)
		{
			scanner->scanNextFile(true, currentPlugBeingScanned);
		}
		else
		{
			scanner->skipNextFile();
		}
	}
	int numTypes = knownPluginList.getNumTypes();
	PluginDescription* plugIn = nullptr;
	switch (numTypes)
	{
	case 1:
		plugIn = knownPluginList.getType(0);
		break;
	case 2:
		PluginDescription* plugIn1 = knownPluginList.getType(0);
		PluginDescription* plugIn2 = knownPluginList.getType(1);
		plugIn = plugIn1->name.contains("x64") ? plugIn1 : plugIn2;
		break;
	}
	if (plugIn != nullptr)
	{
		AudioPluginFormatManager fm;
		fm.addDefaultFormats();
		String ignore;
		if (AudioPluginInstance* pluginInstance = fm.createPluginInstance(*plugIn, 44100.0, 512, ignore))
		{
			limiterPlugin = std::make_unique<PluginWrapperProcessor>(pluginInstance);

			limiterPlugin->setNonRealtime(true);
			limiterPlugin->setParameter(0, 0.94f);		// Threshold -1dbFS
			limiterPlugin->setParameter(1, 0.94f);		// Ceiling -1dbFS
			limiterPlugin->setParameter(2, 0.2f);		// Release  5.5ms
			limiterPlugin->setParameter(3, 1.0f);		// Auto Release - ON
		}
	}
}
bool MainComponent::validateProcessorParameters()
{
	dBLufsTarget = (float)controlPanel.sldLUFSTarget.getValue();

	if (filesToProcesstListModel->getNumRows() == 0)
	{
		AlertWindow dlg("Parameter Error", "No Input Files", AlertWindow::AlertIconType::WarningIcon);
		dlg.addButton("OK", 1);
		dlg.setUsingNativeTitleBar(true);
		dlg.runModalLoop();
		return false;
	}
	
	writeFile = headerPanel.lblDestFolder.getText() != headerPanel.tagNoDestinationFolder ? true : false;
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
	headerPanel.progressValue = percent;
}
#pragma endregion

#pragma region Run Batch Audio Processing
void MainComponent::runProcess()
{
	if (validateProcessorParameters() == false)
	{
		return;
	}

	headerPanel.setEnableState(false);
	controlPanel.setEnableState(false);

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

		headerPanel.setEnableState(true);
		controlPanel.setEnableState(true);
		return;
	}
	updateProgressPercentage();

	activeOfflineLoudnessScanItem = filesToProcesstListModel->getFile(activeScanIndex);
	activeOfflineLoudnessScanItem->dBLufsTarget = dBLufsTarget;
	activeOfflineLoudnessScanItem->destinationFolder = destinationFolder;
	activeOfflineLoudnessScanItem->writeFile = writeFile;
	activeOfflineLoudnessScanItem->limiterPlugin = limiterPlugin.get();

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

		headerPanel.setEnableState(true);
		controlPanel.setEnableState(true);
		return;
	}

	fileTablePanel.listInputFiles.repaintRow(activeOfflineLoudnessScanItem->rowNo);
	activeScanIndex++;
	startNextLoudnessScan();
}
#pragma endregion
