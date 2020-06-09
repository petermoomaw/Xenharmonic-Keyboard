#pragma once

#include "EditCombo.g.h"
#include "Enharmonic\LatticeView.h"

namespace SDKTemplate
{
  public ref class FileComboBoxItem sealed : public Windows::UI::Xaml::Controls::ComboBoxItem
  {
  internal:
    wstring fileName;
    StorageFolder^ folder;
  };

	public ref class EditCombo sealed
	{
	public:
		EditCombo();
		Windows::UI::Xaml::Controls::ComboBox^ getComboBox();
		void setText(Platform::String^ text);
		Platform::String^ getText();
		

	internal:
		StorageFolder^ folder;
		void (*SelectionChanged)(int newIndex, void* data) = 0;
		void (*TextChanged)(Object^ sender) = 0;
		void* data = 0;
	private:
		void comboBox_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		int oldSelectedIndex = -1;
		void comboBox_DropDownOpened(Platform::Object^ sender, Platform::Object^ e);
		void comboBox_DropDownClosed(Platform::Object^ sender, Platform::Object^ e);
		void textBox_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args);
		void textBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
	};
};

