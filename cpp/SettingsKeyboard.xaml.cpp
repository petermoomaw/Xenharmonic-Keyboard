#include "pch.h"
#include "EditCombo.xaml.h"
#include "SettingsKeyboard.xaml.h"
#include "SettingsExpression.xaml.h"
#include "Enharmonic/Common.h"

#include <ppltasks.h>   // For create_task

using namespace SDKTemplate;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Popups;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace concurrency;

SettingsKeyboard::SettingsKeyboard()
{
	InitializeComponent();
	initialized = true;
  keyboardEditPopup->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void SettingsKeyboard::PanelHidden()
{
//  keyboardEditPopup->Visibility = Windows::UI::Xaml::Visibility::Visible;
}

task<void> SettingsKeyboard::populateFileCombo()
{
  LatticeView^ latticeView = getLatticeView(this);

	Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
	ComboBox^ comboBox = name;

  {
    initialized = false;
    comboBox->Items->Clear();
    initialized = true;
  }

  FileComboBoxItem^ item = ref new FileComboBoxItem();
  item->Content = ref new String(L"Prepackaged Keyboards");
  item->FontWeight = Windows::UI::Text::FontWeights::Bold;
  item->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightBlue);
	item->IsEnabled = false;
  comboBox->Items->Append(item);

  return create_task(latticeView->preloadedKeyboardFolder->GetItemsAsync()).then([this, comboBox, latticeView](IVectorView<IStorageItem^>^ items)
  {
		for (unsigned int i = 0; i < items->Size; i++)
		{
      StorageFile^ file = (StorageFile^)items->GetAt(i);
      wstring name = file->Name->Data();
      if (ends_with(name, L".kb"))
      {
        name = convertFileNameToString(name.c_str());
        trimFileExtension(name);
        FileComboBoxItem^ item = ref new FileComboBoxItem();
        item->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightBlue);
        item->Content = ref new String((L" ●  " + name).c_str());
        item->fileName = name;
        item->folder = latticeView->preloadedKeyboardFolder;
        comboBox->Items->Append(item);   
              
        if(latticeView->keyboardName == name && latticeView->keyboardFolder->Path == item->folder->Path)
        {
          initialized = false;
          comboBox->SelectedItem = item;
          initialized = true;
        }
      }
		}
    return latticeView->localKeyboardFolder->GetItemsAsync();
  }).then([this, comboBox, latticeView](IVectorView<IStorageItem^>^ items)
  {
    if(items->Size > 0)
    {
      FileComboBoxItem^ item = ref new FileComboBoxItem();
      item->Content = ref new String(L"User Created Keyboards");
      item->FontWeight = Windows::UI::Text::FontWeights::Bold;
		  item->IsEnabled = false;
      comboBox->Items->Append(item);
		  for (unsigned int i = 0; i < items->Size; i++)
		  {
			  StorageFile^ file = (StorageFile^) items->GetAt(i);
			  wstring name = file->Name->Data();
			  if (ends_with(name, L".kb"))
			  {
          name = convertFileNameToString(name.c_str());
          trimFileExtension(name);
          FileComboBoxItem^ item = ref new FileComboBoxItem();
          item->Content = ref new String((L" ◆  " + name).c_str());
          item->fileName = name;
          item->folder = latticeView->localKeyboardFolder;
          comboBox->Items->Append(item);

          if (latticeView->keyboardName == name && latticeView->keyboardFolder->Path == item->folder->Path)
          {
            initialized = false;
            comboBox->SelectedItem = item;
            initialized = true;
          }
			  }
		  }
    }

    if (latticeView->keyboardFolder->Path == latticeView->preloadedKeyboardFolder->Path)
    {
      name->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightBlue);
    }
    else
    {
      name->Foreground = ref new SolidColorBrush(Windows::UI::Colors::White);
    }
  });


}

void SettingsKeyboard::syncComponents()
{
  populateFileCombo().then([this](){this->syncCompsNoFileCombo();});
}

void SettingsKeyboard::syncCompsNoFileCombo()
{
  initialized = false;
	LatticeView^ latticeView = getLatticeView(this);
  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->clearTouchData();
  }

  if (latticeView->dragKeyboard)
  {
    drag->IsChecked = true;
  }
  else
  {
    drag->IsChecked = false;
  }

  if(Edit->IsChecked->Value)
  { 
    keyboardEditPopup->Visibility = Windows::UI::Xaml::Visibility::Visible;
    latticeView->playMusic = false;
  }
  else
  {
    keyboardEditPopup->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    latticeView->playMusic = true;
  }

  FileComboBoxItem^ keyboard = (FileComboBoxItem^)name->SelectedItem;

  if(keyboard != nullptr && keyboard->folder != nullptr && keyboard->fileName.size() > 0)
  {
    wstring fileName = convertStringToFileName(keyboard->fileName.c_str());
    fileName += L".kb";
    create_task(keyboard->folder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
    {
      Reset->IsEnabled = true;
      Delete->IsEnabled = keyboard->folder->Path != latticeView->preloadedKeyboardFolder->Path;
    }).then([=](task<void> t)
    {
      try
      {
        t.get();
      }
      catch (Platform::Exception^ e)  // file dose not exist
      {
        Reset->IsEnabled = false;
        Delete->IsEnabled = false;
      }
    });
  }
  else
  {
    Delete->IsEnabled = false;
    Reset->IsEnabled = false;
  }

	switch (latticeView->mode)
	{
	case EDIT_SCALE:
		twoTouchMode->SelectedIndex = 0;
		break;

	case EDIT_ROTATE:
		twoTouchMode->SelectedIndex = 1;
		break;

	case EDIT_ROTATE_SCALE:
		twoTouchMode->SelectedIndex = 2;
		break;

	case EDIT_SQUEEZE:
		twoTouchMode->SelectedIndex = 3;
		break;

	case EDIT_SHEAR:
		twoTouchMode->SelectedIndex = 4;
		break;
	}

	switch (latticeView->lattice.keyMode)
	{
	case VORONOI:
		keyShape->SelectedIndex = 0;
		resetShape->IsEnabled = false;
		break;

	case RECTANGLE:
		keyShape->SelectedIndex = 1;
		resetShape->IsEnabled = false;
		break;

	case CUSTOM:
		keyShape->SelectedIndex = 2;
		resetShape->IsEnabled = true;
		break;
	}

	scale->Value = latticeView->scale;
  oneTouchEdit->IsOn = latticeView->oneTouchEdit;
	twoTouchToggle->IsOn = latticeView->twoTouchEdit;
	threeTouchToggle->IsOn = latticeView->threeTouchEdit;
  playUnnamedNotes->IsOn = latticeView->playUnamedNotes;
  duplicateRatios->IsChecked = latticeView->duplicateRatios;
  duplicateKeys->IsChecked = latticeView->dupKeys;
  showName->IsChecked = latticeView->showName;
  showCents->IsChecked = latticeView->showCents;
  showRatio->IsChecked = latticeView->showRatio;
  showDelta->IsChecked = latticeView->showDelta;

  scientificPitchNotation->IsChecked = latticeView->scientificPitchNotation;
  use53TETNotation->IsChecked = latticeView->use53TETNotation;

  textSize->Value = latticeView->textSize;
  dynamicTemperament->IsChecked = latticeView->dynamicTemperament;
  initialized = true;
};



void SDKTemplate::SettingsKeyboard::browse_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileOpenPicker^ picker = ref new FileOpenPicker();
	picker->ViewMode = PickerViewMode::List;
	picker->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;
	picker->FileTypeFilter->Append(".kb");

	create_task(picker->PickSingleFileAsync()).then([this](StorageFile^ file)
	{
		MessageDialog^ dialog = ref new MessageDialog("File Selected");
		dialog->ShowAsync();


		if (file != nullptr)
		{
			// Application now has read/write access to the picked file
		//	this.textBlock.Text = "Picked photo: " + file.Name;
		}
	});
}


void SDKTemplate::SettingsKeyboard::twoTouchMode_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
    if (!initialized)
		return;

	LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();

	switch (twoTouchMode->SelectedIndex)
	{
	case 0:
		latticeView->mode = EDIT_SCALE;
		break;

	case 1:
		latticeView->mode = EDIT_ROTATE;
		break;

	case 2:
		latticeView->mode = EDIT_ROTATE_SCALE;
		break;

	case 3:
		latticeView->mode = EDIT_SQUEEZE;
		break;

	case 4:
		latticeView->mode = EDIT_SHEAR;
		break;
	}
}


void SDKTemplate::SettingsKeyboard::KeyShape_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (!initialized)
		return;

	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();

	switch (keyShape->SelectedIndex)
	{
	case 0:
		latticeView->lattice.keyMode = VORONOI;
		resetShape->IsEnabled = false;
		latticeView->invalidateCellPath();
		break;

	case 1:
		latticeView->lattice.keyMode = RECTANGLE;
		resetShape->IsEnabled = false;
		latticeView->invalidateCellPath();
		break;

	case 2:
		latticeView->lattice.keyMode = CUSTOM;
		resetShape->IsEnabled = true;
		latticeView->invalidateCellPath();
		break;
	}
}

void SDKTemplate::SettingsKeyboard::oneTouchEdit_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->oneTouchEdit = oneTouchEdit->IsOn;
  latticeView->clearTouchData();

  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::threeTouchToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!initialized)
		return;

	LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
	latticeView->threeTouchEdit = threeTouchToggle->IsOn;
}


void SDKTemplate::SettingsKeyboard::twoTouchToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!initialized)
		return;

	LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
	latticeView->twoTouchEdit = twoTouchToggle->IsOn;
}


void SDKTemplate::SettingsKeyboard::rotate90_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->clearTouchData();

	double largeGen1X = latticeView->largeGen1(0);
	double largeGen2X = latticeView->largeGen2(0);

	latticeView->largeGen1(0) = latticeView->largeGen1(1);
	latticeView->largeGen1(1) = -largeGen1X;

	latticeView->largeGen2(0) = latticeView->largeGen2(1);
	latticeView->largeGen2(1)= -largeGen2X;

	double temp = latticeView->originX;
	latticeView->originX = latticeView->originY*(latticeView->m_d3dRenderTargetSize.Height / latticeView->m_d3dRenderTargetSize.Width);
	latticeView->originY = -temp*(latticeView->m_d3dRenderTargetSize.Width / latticeView->m_d3dRenderTargetSize.Height);

	latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::flipVertical_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
	latticeView->largeGen1(1) *= -1;
	latticeView->largeGen2(1) *= -1;
	latticeView->originY *= -1;

	latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::flipHorizontal_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
	latticeView->largeGen1(0) *= -1;
	latticeView->largeGen2(0) *= -1;
	latticeView->originX *= -1;

	latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::fileMenu_Opening(Platform::Object^ sender, Platform::Object^ e)
{
/*	 StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
	 create_task(localFolder->GetFolderAsync("Keyboards")).then([=](StorageFolder^ folder)
	 {
		 wstring fileName = convertStringToFileName(name->getText()->Data());
		 fileName += L".kb";
		 create_task(folder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
		 {
			 Delete->IsEnabled = true;
			 Reset->IsEnabled = true;
		 }).then([=](task<void> t)
		 {
			 try
			 {
				 t.get();
			 }
			 catch (Platform::Exception^ e)  // file dose not exist
			 {
				 Delete->IsEnabled = false;
				 Reset->IsEnabled = false;
			 }
		 });
	 });
	 */
}


void SDKTemplate::SettingsKeyboard::Reset_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
  if(item == nullptr)
    return;

	wstring fileName = convertStringToFileName(item->fileName.c_str());
	fileName += L".kb";
	LatticeView^ latticeView = getLatticeView(this);
	create_task(latticeView->keyboardFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
	{
		latticeView->loadKeyboardAsync(file).then([this, latticeView]()
    {
      syncCompsNoFileCombo();
		  latticeView->invalidateCellPath();
      if(latticeView->dynamicTemperament)
      {
        latticeView->mainPage->initializeKeyboardControls();
        latticeView->mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Visible);
      }
      else
      {
        latticeView->mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Collapsed);
      }
    });
	}).then([=](task<void> t)
	{
		try
		{
			t.get();
		}
		catch (Platform::Exception^ e)  // file dose not exist
		{
		}
	});
}


void SDKTemplate::SettingsKeyboard::Delete_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	if (latticeView->keyboardFolder->Path == latticeView->preloadedKeyboardFolder->Path)
	{
		MessageDialog^ dialog = ref new MessageDialog(ref new String(L"Can only delete user created keyboards."));
		dialog->Commands->Append(ref new UICommand("Ok"));

		create_task(dialog->ShowAsync());
		return;
	}

  FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
  if (item == nullptr)
    return;

	wstring fileName = convertStringToFileName(item->fileName.c_str());
	fileName += L".kb";

	MessageDialog^ dialog = ref new MessageDialog(ref new String((wstring(L"Are you sure you want to delete the keyboard \"") + item->fileName + L"\"?").c_str()));
	dialog->Commands->Append(ref new UICommand("No"));
	dialog->Commands->Append(ref new UICommand("Yes"));


	create_task(dialog->ShowAsync()).then([=](IUICommand^ command)
	{
		if (command->Label == "Yes")
		{
			StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
			create_task(localFolder->GetFolderAsync("Keyboards")).then([=](StorageFolder^ folder)
			{
				create_task(folder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
				{
					create_task(file->DeleteAsync()).then([=](task<void> t)
					{
						try
						{
							t.get();
							this->syncComponents();
						}
						catch (Platform::Exception^ e)  // file dose not exist
						{
						}
					});
				}).then([=](task<void> t)
				{
					try
					{
						t.get();
					}
					catch (Platform::Exception^ e)  // file dose not exist
					{
					}
				});
			});
		}
	});
}



void SDKTemplate::SettingsKeyboard::scale_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (!initialized)
		return;

	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
	latticeView->scale = scale->Value;
	latticeView->invalidateCellPath();
}



void SDKTemplate::SettingsKeyboard::resetShape_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!initialized)
		return;

	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
	latticeView->lattice.cellVerticies1.clear();
	latticeView->lattice.cellVerticies2.clear();
	latticeView->lattice.cellVerticies3.clear();

	latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::duplicateKeys_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!initialized)
		return;

	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
	latticeView->dupKeys = duplicateKeys->IsChecked->Value;

	if (latticeView->dupKeys)
	{
		if (latticeView->keyDuplicateVec == Vector2d::Zero())
		{
			latticeView->keyDuplicateVec = latticeView->periodVec - latticeView->generatorVec;
		}
	}
	else
	{
		if (latticeView->keyDuplicateVec == latticeView->periodVec - latticeView->generatorVec)
		{
			latticeView->keyDuplicateVec = Vector2d::Zero();
		}
	}

	latticeView->invalidateCellPath();
}

void SDKTemplate::SettingsKeyboard::duplicateRatios_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->duplicateRatios = duplicateRatios->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}

void SDKTemplate::SettingsKeyboard::showName_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->showName = showName->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::showRatio_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->showRatio = showRatio->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::showDelta_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->showDelta = showDelta->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::showCents_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->showCents = showCents->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::showRatio_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->showRatio = showRatio->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::showDelta_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->showDelta = showDelta->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::showCents_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->showCents = showCents->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::showName_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->showName = showName->IsChecked->Value;
  latticeView->temperament.clearNoteNames();
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::playUnnamedNotes_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();

  latticeView->playUnamedNotes = playUnnamedNotes->IsOn;
  latticeView->invalidateCellPath();
}

void SDKTemplate::SettingsKeyboard::scientificPitchNotation_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
//  if (!initialized)
//    return;

  LatticeView^ latticeView = getLatticeView(this);

  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->scientificPitchNotation = scientificPitchNotation->IsChecked->Value;
    latticeView->temperament.clearNoteNames();
    latticeView->invalidateBitmap();
  }
}


void SDKTemplate::SettingsKeyboard::scientificPitchNotation_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
//  if (!initialized)
//    return;


  LatticeView^ latticeView = getLatticeView(this);

  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->scientificPitchNotation = scientificPitchNotation->IsChecked->Value;
    latticeView->temperament.clearNoteNames();
    latticeView->invalidateBitmap();
  }
}


void SDKTemplate::SettingsKeyboard::use53TETNotation_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
//  if (!initialized)
//    return;


  LatticeView^ latticeView = getLatticeView(this);

  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->use53TETNotation = use53TETNotation->IsChecked->Value;
    latticeView->temperament.clearNoteNames();
    latticeView->invalidateBitmap();
  }
}


void SDKTemplate::SettingsKeyboard::use53TETNotation_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
//  if (!initialized)
//    return;

  LatticeView^ latticeView = getLatticeView(this);

  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->use53TETNotation = use53TETNotation->IsChecked->Value;
    latticeView->temperament.clearNoteNames();
    latticeView->invalidateBitmap();
  }
}


void SDKTemplate::SettingsKeyboard::textSize_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->textSize = textSize->Value;
  latticeView->temperament.clearNoteText();
  latticeView->invalidateCellPath();
}



void SDKTemplate::SettingsKeyboard::recalibrateScale_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();

  latticeView->largeGen1 *= latticeView->scale;
  latticeView->largeGen2 *= latticeView->scale;
  latticeView->textSize *= latticeView->scale;
  latticeView->originX *= latticeView->scale;
  latticeView->originY *= latticeView->scale;
  latticeView->scale = 1;

  initialized = false;
  scale->Value = latticeView->scale;
  textSize->Value = latticeView->textSize;
  initialized = true;
  latticeView->invalidateCellPath();
}


void SDKTemplate::SettingsKeyboard::Edit_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  Options->IsChecked = false;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  keyboardEditPopup->Visibility = Windows::UI::Xaml::Visibility::Visible;
  latticeView->playMusic = false;
}


void SDKTemplate::SettingsKeyboard::Edit_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  keyboardEditPopup->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
  latticeView->playMusic = true;
}


void SDKTemplate::SettingsKeyboard::SaveAs_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  saveAsDialog->ShowAsync();
}

void SDKTemplate::SettingsKeyboard::saveAsDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args)
{
  FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
  if (item == nullptr)
    newName->Text = "";
  else
    newName->Text = ref new String(item->fileName.c_str());
}

void SDKTemplate::SettingsKeyboard::saveAsDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
  wstring fileName = convertStringToFileName(newName->Text->Data());
  fileName += L".kb";
  LatticeView^ latticeView = getLatticeView(this);
  create_task(latticeView->localKeyboardFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
  {
    MessageDialog^ dialog = ref new MessageDialog(ref new String((wstring(L"A user created keyboard with the name \"") + newName->Text->Data() + L"\" already exists. Do you want to replace it?").c_str()));
    dialog->Commands->Append(ref new UICommand("No"));
    dialog->Commands->Append(ref new UICommand("Yes"));

    create_task(dialog->ShowAsync()).then([=](IUICommand^ command)
    {
      if (command->Label == "Yes")
      {
        latticeView->keyboardName = newName->Text->Data();
        latticeView->keyboardFolder = latticeView->localKeyboardFolder;
        latticeView->saveKeyboard().then([this]() {this->syncComponents();});
      }
    });

  }).then([=](task<void> t)
  {
    try
    {
      t.get();
    }
    catch (Platform::Exception^ e)  // file dose not exist
    {
      latticeView->keyboardName = newName->Text->Data();
      latticeView->keyboardFolder = latticeView->localKeyboardFolder;
      latticeView->saveKeyboard().then([this]() {this->syncComponents();});
    }
  });
}

void SDKTemplate::SettingsKeyboard::Save_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
  if (item == nullptr)
  {
    SaveAs_Click(sender, e);
    return;
  }

  wstring fileName = convertStringToFileName(item->fileName.c_str());
  fileName += L".kb";
  LatticeView^ latticeView = getLatticeView(this);
  if (latticeView->keyboardFolder->Path == latticeView->localKeyboardFolder->Path)
  {
    latticeView->saveKeyboard().then([this]() {this->syncComponents();});
  }
  else
  {
    create_task(latticeView->localKeyboardFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
    {
      MessageDialog^ dialog = ref new MessageDialog(ref new String((wstring(L"A user created keyboard with the name \"") + item->fileName + L"\" already exists. Do you want to replace it?").c_str()));
      dialog->Commands->Append(ref new UICommand("No"));
      dialog->Commands->Append(ref new UICommand("Yes"));

      create_task(dialog->ShowAsync()).then([=](IUICommand^ command)
      {
        if (command->Label == "Yes")
        {
          latticeView->keyboardFolder = latticeView->localKeyboardFolder;
          latticeView->saveKeyboard().then([this]() {this->syncComponents();});
        }
      });

    }).then([=](task<void> t)
    {
      try
      {
        t.get();
      }
      catch (Platform::Exception^ e)  // file dose not exist
      {
        latticeView->keyboardFolder = latticeView->localKeyboardFolder;
        latticeView->saveKeyboard().then([this]() {this->syncComponents();});
      }
    });
  }
}

void SDKTemplate::SettingsKeyboard::name_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
  if(!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);

  ComboBox^ comboBox = name;
  FileComboBoxItem^ item = (FileComboBoxItem^)comboBox->SelectedItem;
  if (item == nullptr || item->fileName.size() == 0 || item->folder == nullptr)
  {
    return;
  }

  wstring fileName = convertStringToFileName(item->fileName.c_str());
  latticeView->keyboardFolder = item->folder;
  fileName += L".kb";

  if (latticeView->keyboardFolder->Path == latticeView->preloadedKeyboardFolder->Path)
  {
    name->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightBlue);
  }
  else
  {
    name->Foreground = ref new SolidColorBrush(Windows::UI::Colors::White);
  }

  create_task(item->folder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
  {
    latticeView->loadKeyboardAsync(file).then([this, latticeView]()
    {
      syncCompsNoFileCombo();
      if (latticeView->dynamicTemperament)
      {
        latticeView->mainPage->initializeKeyboardControls();
        latticeView->mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Visible);
      }
      else
      {
        latticeView->mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Collapsed);
      }
    });
  }).then([=](task<void> t)
  {
    try
    {
      t.get();
    }
    catch (Platform::Exception^ e)  // file dose not exist
    {
    }
  });
}


void SDKTemplate::SettingsKeyboard::drag_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->dragKeyboard = true;
}


void SDKTemplate::SettingsKeyboard::drag_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->dragKeyboard = false;
}


void SDKTemplate::SettingsKeyboard::afterTouch_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  switch (afterTouch->SelectedIndex)
  {
  case 0:
    latticeView->afterTouchMode = AfterTouchMode::SUSTAIN;
    break;

  case 1:
    latticeView->afterTouchMode = AfterTouchMode::VIBRATO;
    break;

  case 2:
    latticeView->afterTouchMode = AfterTouchMode::BEND;
    break;

  case 3:
    latticeView->afterTouchMode = AfterTouchMode::GLISSANDO;
    break;
  }
}


void SDKTemplate::SettingsKeyboard::Options_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  Edit->IsChecked = false;
  ((UIElement^)expressionAdvanced)->Visibility = Windows::UI::Xaml::Visibility::Visible;
  ((SettingsExpression^)expressionAdvanced)->syncComponents();
}


void SDKTemplate::SettingsKeyboard::Options_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  ((UIElement^)expressionAdvanced)->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
  ((SettingsExpression^)expressionAdvanced)->PanelHidden();
}


void SDKTemplate::SettingsKeyboard::dynamicTemperament_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->dynamicTemperament = dynamicTemperament->IsChecked->Value;
  latticeView->mainPage->initializeKeyboardControls();
  latticeView->mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Visible);
  latticeView->generateKeyboard(latticeView->EDO);
}


void SDKTemplate::SettingsKeyboard::dynamicTemperament_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->dynamicTemperament = dynamicTemperament->IsChecked->Value;
  latticeView->mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Collapsed);
}

