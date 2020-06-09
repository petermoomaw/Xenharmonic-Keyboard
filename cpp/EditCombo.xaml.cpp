#include "pch.h"
#include "EditCombo.xaml.h"
#include "Enharmonic/Common.h"

using namespace SDKTemplate;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;
using namespace Windows::UI::Popups;

EditCombo::EditCombo()
{
	InitializeComponent();
}



void SDKTemplate::EditCombo::comboBox_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	//ComboBox^ cb= (ComboBox^)sender;
	//textBox->Text = ((String^)cb->Items->GetAt(cb->SelectedIndex));

//	if (cb->SelectedIndex != -1 && cb->SelectedIndex != this->selectedIndex)
//	{
////		textBox->Text = ((ComboBoxItem^)cb->Items->GetAt(cb->SelectedIndex))->Content->ToString();
//		textBox->Text = ((String^)cb->Items->GetAt(cb->SelectedIndex));
//		this->selectedIndex = cb->SelectedIndex;
//		cb->SelectedIndex = -1;
//		if (this->selectedIndex >= 0 && this->SelectionChanged != 0)
//			SelectionChanged(this->selectedIndex, data);
//	}
}


void SDKTemplate::EditCombo::comboBox_DropDownOpened(Platform::Object^ sender, Platform::Object^ e)
{
	ComboBox^ cb = (ComboBox^)sender;

	String^ text = textBox->Text;

	//std::wstring str = text->Data();
	//str += L"\n";
	//wstring fileName= convertStringToFileName(text->Data());
	//str += fileName;
	//str += L"\n";
	//str += convertFileNameToString(fileName.c_str());
	//MessageDialog^ dialog = ref new MessageDialog(ref new String(str.c_str()), "");
	//create_task(dialog->ShowAsync());

	if (cb->SelectedIndex != -1)
		return;

	cb->SelectedIndex = -1;
	for (int i = 0; i < this->comboBox->Items->Size; i++)
	{
		FileComboBoxItem^ item = (FileComboBoxItem^) this->comboBox->Items->GetAt(i);

		if (item->fileName == text->Data() && item->folder != nullptr && this->folder != nullptr && item->folder->Path ==  this->folder->Path)
		{
			cb->SelectedIndex = i;
			break;
		}
	}

	oldSelectedIndex = cb->SelectedIndex;
}


void SDKTemplate::EditCombo::comboBox_DropDownClosed(Platform::Object^ sender, Platform::Object^ e)
{
	ComboBox^ cb = (ComboBox^)sender;
	
	if (cb->SelectedIndex != this->oldSelectedIndex)
	{
		SelectionChanged(cb->SelectedIndex, data);
		FileComboBoxItem^ selectedItem = ((FileComboBoxItem^)cb->Items->GetAt(cb->SelectedIndex));
		textBox->Text = ref new String(selectedItem->fileName.c_str());
		this->folder = selectedItem->folder;
	}

	cb->SelectedIndex = -1;
	this->oldSelectedIndex = -1;
}


void SDKTemplate::EditCombo::textBox_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args)
{
//	this->selectedIndex = -1;
}


ComboBox^ SDKTemplate::EditCombo::getComboBox()
{
	return this->comboBox;
}

void SDKTemplate::EditCombo::setText(Platform::String^ text)
{
	this->textBox->Text = text;
}

Platform::String^ SDKTemplate::EditCombo::getText()
{
	return this->textBox->Text;
}

void SDKTemplate::EditCombo::textBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
	if (TextChanged)
		TextChanged(sender);
}
