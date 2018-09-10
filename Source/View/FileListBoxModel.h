/*
  ==============================================================================

    InputFileListBoxModel.h
    Created: 2 Sep 2018 4:02:34pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
class FileLoudnessDetails
{
	public:
		FileLoudnessDetails() {}
		FileLoudnessDetails(int rNo, File f) : rowNo(rNo), file(f) {}
		~FileLoudnessDetails() {}

		int rowNo = 0;
		File file;
		float preIntegratedLufs = 0;
		float prePeakDbfs = 0;
		float diffLufs = 0;
		float gain = 0;
		float postIntegratedLufs = 0;
		float postPeakDbfs = 0;
};

class ListBoxModelListener
{
	public:
		virtual void refreshFileTableModel(String tag) {};
};

class FileListBoxModel : public TableListBoxModel
{
	public:
		FileListBoxModel()
		{
			data.clear();
		}
		void setListener(ListBoxModelListener* l, String t)
		{
			listener = l; 
			tag = t;
		}
		int getNumRows() override
		{
			return data.size();
		}
		void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
		{
			g.fillAll(Colours::lightgrey);
		}
		void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
		{
			Rectangle<int> r(width, height);
			FileLoudnessDetails* row = data[rowNumber];
			switch (columnId)
			{
			case 1:
				g.drawText(row->file.getFileName(), r, Justification::centredLeft);
				break;
			case 2:
				g.drawText(std::to_string(row->preIntegratedLufs), r, Justification::centredRight);
				break;
			case 3:
				g.drawText(std::to_string(row->prePeakDbfs), r, Justification::centredRight);
				break;
			case 4:
				g.drawText(std::to_string(row->diffLufs), r, Justification::centredRight);
				break;
			case 5:
				g.drawText(std::to_string(row->gain), r, Justification::centredRight);
				break;
			case 6:
				g.drawText(std::to_string(row->postIntegratedLufs), r, Justification::centredRight);
				break;
			case 7:
				g.drawText(std::to_string(row->postPeakDbfs), r, Justification::centredRight);
				break;
			default:
				break;
			}
		}
		void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent &) override
		{
			if (columnId == 1)
			{
				data.remove(rowNumber);
				listener->refreshFileTableModel(tag);
			}
		}

		
		OwnedArray<FileLoudnessDetails> data;
		ListBoxModelListener* listener;
		String tag;
};