/*
  ==============================================================================

    InputFileListBoxModel.h
    Created: 2 Sep 2018 4:02:34pm
    Author:  Ken

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "../OfflineLoudnessProcessor/OfflineLoudnessScanDataPacket.h"


class ListBoxModelListener
{
	public:
		virtual void refreshFileTableModel(String tag) {};
};

class FileListBoxModel : public TableListBoxModel
{
	public:
		FileListBoxModel();
		void setListener(ListBoxModelListener* l, String t);

		void addFile(std::shared_ptr<OfflineLoudnessScanDataPacket> scanData);
		std::shared_ptr<OfflineLoudnessScanDataPacket> getFile(int idx);
		void clearFiles();
		void resetFiles(ListBox& listBox);
		File getParentDirectory();

		int getNumRows() override;
		void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
		void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
		void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent &) override;
	
		enum ColumnID
		{
			File = 1,
			InLufs = 2,
			OutLufs = 3,
			Range = 4,
			MaxShortTerm = 5,
			Diff = 6,
			Gain = 7,
			InDbfs = 8,
			OutDbfs = 9,
		};
	private:		
		const int numDecimalPoints = 1;
		Array<std::shared_ptr<OfflineLoudnessScanDataPacket>> data;
		ListBoxModelListener* listener;
		String tag;
};