/*
  ==============================================================================

    InputFileListBoxModel.h
    Created: 2 Sep 2018 4:02:34pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
class ListBoxModelListener
{
public:
	virtual void ModelRefresh(String tag) {};
};

class FileListBoxModel : public ListBoxModel
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
		void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override
		{
			Rectangle<int> r(width, height);
			File f = data[rowNumber];
			g.drawText(f.getFileName(), r, Justification::centredLeft);
		}

		void listBoxItemClicked(int row, const MouseEvent &) override
		{
		}

		void listBoxItemDoubleClicked(int row, const MouseEvent &) override
		{
			data.remove(row);
			listener->ModelRefresh(tag);
		}
		
		Array<File> data;
		ListBoxModelListener* listener;
		String tag;
};