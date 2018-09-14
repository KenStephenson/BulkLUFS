/*
  ==============================================================================

    FileListBoxModel.cpp
    Created: 13 Sep 2018 11:27:36am
    Author:  Ken

  ==============================================================================
*/

#include "FileListBoxModel.h"
FileListBoxModel::FileListBoxModel()
{
	data.clear();
}
void FileListBoxModel::setListener(ListBoxModelListener* l, String t)
{
	listener = l;
	tag = t;
}
void FileListBoxModel::addFile(std::shared_ptr<OfflineLoudnessScanDataPacket> scanData)
{
	data.add(scanData);
}
std::shared_ptr<OfflineLoudnessScanDataPacket> FileListBoxModel::getFile(int idx)
{
	return data[idx];
}
void FileListBoxModel::clearFiles()
{
	data.clear();
}
void FileListBoxModel::resetFiles(ListBox& listBox)
{
	for (int i = 0; i < data.size(); i++)
	{
		std::shared_ptr<OfflineLoudnessScanDataPacket> dataPacket = data[i];
		dataPacket->reset();
		listBox.repaintRow(i);
	}
}

File FileListBoxModel::getParentDirectory()
{
	return data[0]->file.getParentDirectory();
}
int FileListBoxModel::getNumRows() 
{
	return data.size();
}
void FileListBoxModel::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) 
{
	g.fillAll(Colours::lightgrey);
}
void FileListBoxModel::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) 
{
	Rectangle<int> r(width, height);
	std::shared_ptr<OfflineLoudnessScanDataPacket> row = data[rowNumber];
	switch (columnId)
	{
	case 1:
		g.drawText(row->file.getFileName(), r, Justification::centredLeft);
		break;

	case 2:
		g.drawText(String(row->preIntegratedLufs, numDecimalPoints), r, Justification::centredRight);
		break;
	case 3:
		g.drawText(String(row->postIntegratedLufs, numDecimalPoints), r, Justification::centredRight);
		break;
	case 4:
		g.drawText(String(row->diffLufs, numDecimalPoints), r, Justification::centredRight);
		break;
	case 5:
		g.drawText(String(row->postLoudnessRange, numDecimalPoints), r, Justification::centredRight);
		break;
	case 6:
		g.drawText(String(row->postMaximumShortTermLoudness, numDecimalPoints), r, Justification::centredRight);
		break;
	case 7:
		g.drawText(String(row->gain, numDecimalPoints), r, Justification::centredRight);
		break;
	case 8:
		g.drawText(String(row->prePeakDbfs, numDecimalPoints), r, Justification::centredRight);
		break;
	case 9:
		g.drawText(String(row->postPeakDbfs, numDecimalPoints), r, Justification::centredRight);
		break;
	default:
		break;
	}
}
void FileListBoxModel::cellDoubleClicked(int rowNumber, int columnId, const MouseEvent &) 
{
	if (columnId == 1)
	{
		data.remove(rowNumber);
		listener->refreshFileTableModel(tag);
	}
}
