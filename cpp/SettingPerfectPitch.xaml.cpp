//
// SettingPerfectPitch.xaml.cpp
// Implementation of the SettingPerfectPitch class
//

#include "pch.h"
#include "SettingPerfectPitch.xaml.h"
#include "EditCombo.xaml.h"
#include "Enharmonic/Common.h"
#include <ppltasks.h>   // For create_task
#include <random>

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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

SettingPerfectPitch::SettingPerfectPitch()
{
	InitializeComponent();

 // initialized = true;

}

void SettingPerfectPitch::PanelHidden()
{
}

void SDKTemplate::SettingPerfectPitch::instrumentChecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	StackPanel^ stack = (StackPanel ^)((CheckBox^)sender)->Parent;
	LatticeView^ latticeView = getLatticeView(stack);

	unsigned int index = 0;
	stack->Children->IndexOf((UIElement^)sender, &index);
	latticeView->instruments.push_back(index);
}

void SDKTemplate::SettingPerfectPitch::instrumentUnchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	StackPanel^ stack = (StackPanel ^)((CheckBox^)sender)->Parent;
	LatticeView^ latticeView = getLatticeView(stack);

	unsigned int index = 0;
	stack->Children->IndexOf((UIElement^)sender, &index);

	for (int i = latticeView->instruments.size() - 1; i >= 0; i--)
	{
		if (latticeView->instruments[i] == index)
			latticeView->instruments.erase(latticeView->instruments.begin() + i);
	}
}

void SettingPerfectPitch::syncComponents()
{
  populateFileCombo().then([this]() {this->syncCompsNoFileCombo();});
}

void SettingPerfectPitch::syncCompsNoFileCombo()
{
  initialized = false;

  LatticeView^ latticeView = getLatticeView(this);
  if(stack->Children->Size == 0)
  { 
    for (int i = 0; i < latticeView->instrument.programs.size(); i++)
    {
      CheckBox^ checkBox = ref new CheckBox();
      checkBox->Content = ref new String(latticeView->instrument.programs[i].c_str());
      checkBox->Checked += ref new RoutedEventHandler(&instrumentChecked);
      checkBox->Unchecked += ref new RoutedEventHandler(&instrumentUnchecked);
      stack->Children->Append(checkBox);
    }
  }

  sequenceLength->SelectedIndex = latticeView->sequenceLength;
  octavesBelow->SelectedIndex = latticeView->octavesBelow;
  octavesAbove->SelectedIndex = latticeView->octavesAbove;
  duration->SelectedIndex = latticeView->duration / 100 - 1;

  vector<int> inst = latticeView->instruments;
  for (unsigned int i = 0; i < stack->Children->Size; i++)
  {
    ((CheckBox^)stack->Children->GetAt(i))->IsChecked = false;
  }
 
  latticeView->instruments.clear();
  for(unsigned int i = 0; i < inst.size(); i++)
  {
    if(inst[i] < stack->Children->Size)
      ((CheckBox^)stack->Children->GetAt(inst[i]))->IsChecked = true;
  }

  initialized = true;
}


void SDKTemplate::SettingPerfectPitch::sequenceLength_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  latticeView->sequenceLength = sequenceLength->SelectedIndex;
}


void SDKTemplate::SettingPerfectPitch::octavesBelow_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  latticeView->octavesBelow = octavesBelow->SelectedIndex;
}


void SDKTemplate::SettingPerfectPitch::octavesAbove_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  latticeView->octavesAbove = octavesAbove->SelectedIndex;
}


void SDKTemplate::SettingPerfectPitch::playSequence_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  latticeView->playRandomNotes();
}


void SDKTemplate::SettingPerfectPitch::duration_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->duration = 100*(duration->SelectedIndex + 1);
}












void SettingPerfectPitch::exerciseFileSelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);

  ComboBox^ comboBox = name;

  FileComboBoxItem^ item = (FileComboBoxItem^)comboBox->SelectedItem;
  if (item == nullptr || item->fileName.size() == 0 || item->folder == nullptr)
  {
    return;
  }

  wstring fileName = convertStringToFileName(item->fileName.c_str());
  fileName += L".etr";
  create_task(latticeView->localEarTrainingFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
  {
    latticeView->loadEarTrainingExerciseAsync(file).then([this]()
    {
      syncCompsNoFileCombo();
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

task<void> SettingPerfectPitch::populateFileCombo()
{
  Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
  StorageFolder^ installedLocation = package->InstalledLocation;
  StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
  ComboBox^ comboBox = name;

  {
    initialized = false;
    comboBox->Items->Clear();
    initialized = true;
  }

  LatticeView^ latticeView = getLatticeView(this);

  return create_task(latticeView->localEarTrainingFolder->GetItemsAsync()).then([=](IVectorView<IStorageItem^>^ items)
  {
    for (unsigned int i = 0; i < items->Size; i++)
    {
      StorageFile^ file = (StorageFile^)items->GetAt(i);
      wstring name = file->Name->Data();
      if (name.size() > 4 && ends_with(name, L".etr"))
      {
        name = convertFileNameToString(name.c_str());
        trimFileExtension(name);
        FileComboBoxItem^ item = ref new FileComboBoxItem();
        item->Content = ref new String(name.c_str());
        item->fileName = name;
        item->folder = latticeView->localEarTrainingFolder;
        comboBox->Items->Append(item);

        LatticeView^ latticeView = getLatticeView(this);

        if (latticeView->exerciseName == name)
        {
          initialized = false;
          comboBox->SelectedItem = item;
          initialized = true;
        }
      }
    }
  });
}

void SDKTemplate::SettingPerfectPitch::changeInstrument_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView((Button^)sender);

  random_device rd;   // non-deterministic generator  
  mt19937 gen(rd());  // to seed mersenne twister

  if (latticeView->instruments.size())
  {
    std::uniform_int_distribution<int> randInst(0, latticeView->instruments.size() - 1);
    latticeView->instrument.midi_program = latticeView->instruments[randInst(gen)];
    latticeView->instrument.updateMidiProgram();
  }

  latticeView->playRandomNotes();
}


void SDKTemplate::SettingPerfectPitch::Restart_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView((Button^)sender);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->restartTrainingExercise();  
}


void SDKTemplate::SettingPerfectPitch::Clear_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView((Button^)sender);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearEarTrainingExercise();
  syncComponents();
}



void SDKTemplate::SettingPerfectPitch::Reset_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
  if (item == nullptr)
  {
    Clear_Click(sender, e);
    return;
  }

  wstring fileName = convertStringToFileName(item->fileName.c_str());
  fileName += L".etr";

  LatticeView^ latticeView = getLatticeView(this);
  create_task(latticeView->localEarTrainingFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->loadEarTrainingExerciseAsync(file).then([this]()
    {
      syncCompsNoFileCombo();
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


void SDKTemplate::SettingPerfectPitch::Delete_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
  if (item == nullptr)
    return;

  wstring fileName = convertStringToFileName(item->fileName.c_str());
  fileName += L".etr";

  MessageDialog^ dialog = ref new MessageDialog(ref new String((wstring(L"Are you sure you want to delete the ear training exercise \"") + item->fileName + L"\"?").c_str()));
  dialog->Commands->Append(ref new UICommand("No"));
  dialog->Commands->Append(ref new UICommand("Yes"));


  create_task(dialog->ShowAsync()).then([=](IUICommand^ command)
  {
    if (command->Label == "Yes")
    {
      StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
      create_task(localFolder->GetFolderAsync("EarTraining")).then([=](StorageFolder^ folder)
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

void SDKTemplate::SettingPerfectPitch::SaveAs_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  saveAsDialog->ShowAsync();
}

void SDKTemplate::SettingPerfectPitch::saveAsDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args)
{
  FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
  if (item == nullptr)
    newName->Text = "";
  else
    newName->Text = ref new String(item->fileName.c_str());
}

void SDKTemplate::SettingPerfectPitch::saveAsDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
  wstring fileName = convertStringToFileName(newName->Text->Data());
  fileName += L".etr";
  LatticeView^ latticeView = getLatticeView(this);
  create_task(latticeView->localEarTrainingFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
  {
    MessageDialog^ dialog = ref new MessageDialog(ref new String((wstring(L"A user created temperament with the name \"") + newName->Text->Data() + L"\" already exists. Do you want to replace it?").c_str()));
    dialog->Commands->Append(ref new UICommand("No"));
    dialog->Commands->Append(ref new UICommand("Yes"));

    create_task(dialog->ShowAsync()).then([=](IUICommand^ command)
    {
      if (command->Label == "Yes")
      {
        latticeView->exerciseName = newName->Text->Data();
        latticeView->saveEarTrainingExercise();
        this->syncComponents();
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
      latticeView->exerciseName = newName->Text->Data();
      latticeView->saveEarTrainingExercise();
      this->syncComponents();
    }
  });
}



void SDKTemplate::SettingPerfectPitch::Save_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
  if (item == nullptr)
  {
    SaveAs_Click(sender, e);
    return;
  }

  wstring fileName = convertStringToFileName(item->fileName.c_str());
  fileName += L".etr";
  LatticeView^ latticeView = getLatticeView(this);
  latticeView->exerciseName = item->fileName;
  latticeView->saveEarTrainingExercise();
  this->syncComponents();
}