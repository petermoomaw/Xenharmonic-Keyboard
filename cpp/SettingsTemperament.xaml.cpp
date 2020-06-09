#include "pch.h"
#include "SettingsTemperament.xaml.h"
#include "TemperamentDataItem.xaml.h"
#include "TemperamentEditRatio.xaml.h"
#include "TemperamentEditRatioList.xaml.h"
#include "EditFloat.xaml.h"
#include "EditCombo.xaml.h"

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

SettingsTemperament::SettingsTemperament()
{
	InitializeComponent();

	periodCents->ValueChanged = SettingsTemperament::periodCentsChanged;
	generatorCents->ValueChanged = SettingsTemperament::generatorCentsChanged;
  anchorFreq->ValueChanged = SettingsTemperament::anchorFreqChanged;

	// These are just to consume events to elimiate wierdness.
	scrollView->ManipulationStarting += ref new ManipulationStartingEventHandler(this, &SettingsTemperament::ManipulateMe_ManipulationStarting);
	scrollView->ManipulationStarted += ref new ManipulationStartedEventHandler(this, &SettingsTemperament::ManipulateMe_ManipulationStarted);
	scrollView->ManipulationDelta += ref new ManipulationDeltaEventHandler(this, &SettingsTemperament::ManipulateMe_ManipulationDelta);
	scrollView->ManipulationCompleted += ref new ManipulationCompletedEventHandler(this, &SettingsTemperament::ManipulateMe_ManipulationCompleted);
	scrollView->ManipulationInertiaStarting += ref new ManipulationInertiaStartingEventHandler(this, &SettingsTemperament::ManipulateMe_ManipulationInertiaStarting);
}


void SettingsTemperament::PanelHidden()
{
}

void SettingsTemperament::periodCentsChanged(Object^ sender)
{
	EditFloat^ editor = (EditFloat^)sender;
	LatticeView^ latticeView = getLatticeView(editor);
  critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->temperament.periodCents = editor->Value;
  latticeView->temperament.clearNoteNames();
	latticeView->invalidateBitmap();
}

void SettingsTemperament::generatorCentsChanged(Object^ sender)
{
	EditFloat^ editor = (EditFloat^)sender;
	LatticeView^ latticeView = getLatticeView(editor);
  critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->temperament.genCents = editor->Value;
  latticeView->temperament.clearNoteNames();
	latticeView->invalidateBitmap();
}

void SettingsTemperament::anchorFreqChanged(Object^ sender)
{
	EditFloat^ editor = (EditFloat^)sender;
	LatticeView^ latticeView = getLatticeView(editor);
  critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->temperament.baseFreq = editor->Value;
  latticeView->temperament.clearNoteNames();
	latticeView->invalidateBitmap();
}


void SettingsTemperament::temperamentSelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
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
	latticeView->temperament.temperamentFolder = item->folder;
	fileName += L".tpmt";

  if (latticeView->temperament.temperamentFolder->Path == latticeView->temperament.preloadedTemperamentFolder->Path)
  {
    name->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightBlue);
  }
  else
  {
    name->Foreground = ref new SolidColorBrush(Windows::UI::Colors::White);
  }

	create_task(item->folder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([this, item, latticeView](StorageFile^ file)
	{
		{
      latticeView->temperament.name = item->fileName.c_str();
		  latticeView->temperament.loadTemperamentAsync(item->folder).then([this, latticeView](bool)
      {
        syncCompsNoFileCombo();
        critical_section::scoped_lock lock(latticeView->criticalSection);
        latticeView->temperament.clearNoteNames();
        latticeView->invalidateBitmap();
      });
		}
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


/*task<void> SettingsTemperament::populateFontCombo()
{
  new InstalledFontCollection();

  string[] fonts = Microsoft::Graphics::Canvas::Text::CanvasTextFormat::GetSystemFontFamilies();

  List<string> fonts = new List<string>();

  foreach(FontFamily font in System::Drawing::FontFamily::Families)
  {
    comboBox->Items->Append(item);
  }
}
*/

task<void> SettingsTemperament::populateFileCombo()
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
	item->Content = ref new String(L"Prepackaged Temperaments");
	item->FontWeight = Windows::UI::Text::FontWeights::Bold;
	item->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightBlue);
	item->IsEnabled = false;
	comboBox->Items->Append(item);

  return create_task(latticeView->temperament.preloadedTemperamentFolder->GetItemsAsync()).then([this, comboBox, latticeView](IVectorView<IStorageItem^>^ items)
  {
    for (unsigned int i = 0; i < items->Size; i++)
		{
			StorageFile^ file = (StorageFile^)items->GetAt(i);
			wstring name = file->Name->Data();
			if (ends_with(name, L".tpmt"))
			{
				name = convertFileNameToString(name.c_str());
				trimFileExtension(name);
				FileComboBoxItem^ item = ref new FileComboBoxItem();
				item->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightBlue);
				item->Content = ref new String((L" ●  " + name).c_str());
				item->fileName = name;
				item->folder = latticeView->temperament.preloadedTemperamentFolder;
				comboBox->Items->Append(item);

				if (latticeView->temperament.name == name && latticeView->temperament.temperamentFolder->Path == item->folder->Path)
        {
          initialized = false;
          comboBox->SelectedItem = item;
          initialized = true;
        }
			}
		}
    return latticeView->temperament.localTemperamentFolder->GetItemsAsync();
	}).then([this, comboBox, latticeView](IVectorView<IStorageItem^>^ items)
	{
    if(items->Size > 0)   
    { 
		  FileComboBoxItem^ item = ref new FileComboBoxItem();
		  item->Content = ref new String(L"User Created Temperaments");
		  item->FontWeight = Windows::UI::Text::FontWeights::Bold;
		  item->IsEnabled = false;
		  comboBox->Items->Append(item);
		  for (unsigned int i = 0; i < items->Size; i++)
		  {
			  StorageFile^ file = (StorageFile^)items->GetAt(i);
			  wstring name = file->Name->Data();
			  if (ends_with(name, L".tpmt"))
			  {
				  name = convertFileNameToString(name.c_str());
				  trimFileExtension(name);
				  FileComboBoxItem^ item = ref new FileComboBoxItem();
				  item->Content = ref new String((L" ◆  " + name).c_str());
				  item->fileName = name;
				  item->folder = latticeView->temperament.localTemperamentFolder;
				  comboBox->Items->Append(item);

				  if (latticeView->temperament.name == name && latticeView->temperament.temperamentFolder->Path == item->folder->Path)
          {
            initialized = false;
            comboBox->SelectedItem = item;
            initialized = true;
          }
			  }
		  }
    }

    if (latticeView->temperament.temperamentFolder->Path == latticeView->temperament.preloadedTemperamentFolder->Path)
    {
      name->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightBlue);
    }
    else
    {
      name->Foreground = ref new SolidColorBrush(Windows::UI::Colors::White);
    }
	});
}

void SettingsTemperament::syncComponents()
{
  populateFileCombo().then([this]() {this->syncCompsNoFileCombo();});
}

void SettingsTemperament::syncCompsNoFileCombo()
{
  initialized = false;

	LatticeView^ latticeView = getLatticeView(this);
	Temperament& temp = latticeView->temperament;

	unsigned int size = temp.persIn.size();

	if (size != temp.persIn.size() || size != temp.gensIn.size() || size != temp.ratioGeneratorsIn.size() || size != temp.ratioGeneratorsIn.size() || size != temp.enabledIn.size())
		Throw(L"Periods and genorators don't contain the same number of entries");

	for (int i = ratioGrid->Children->Size - 2; i >= 0; i--)
		ratioGrid->Children->RemoveAt(i);

	for (int i = 0;  i < temp.persIn.size(); i++)
	{
		TemperamentDataItem^ col = ref new TemperamentDataItem(temp.enabledIn[i], temp.ratioGeneratorsIn[i], temp.persIn[i], temp.gensIn[i]);
		ratioGrid->Children->InsertAt(ratioGrid->Children->Size-1, col);
	}

	//no need to wory about anchorNote combo, it will be populated when notenames text changed via an event.

  wstring names;
  for (unsigned int i = 0; i < latticeView->temperament.inNamingRatios.size(); i++)
  {
    if (latticeView->temperament.inNamingRatios[i].noteNames.size() > 0)
    {
      for (unsigned int j = 0; j < latticeView->temperament.inNamingRatios[i].noteNames.size(); j++)
      {  
        if(j > 0)
          names += L" ";

        names += latticeView->temperament.inNamingRatios[i].noteNames[j];
      }
    }
  }

  noteNames->Text = ref new String(names.c_str());

  wstring accs;
  for (unsigned int i = 0; i < latticeView->temperament.inNamingRatios.size(); i++)
  {
    if (i > 0)
      accs += L" ";

    accs += latticeView->temperament.inNamingRatios[i].asciiName;
  }

  accidentals->Text = ref new String(accs.c_str());

	periodCents->Value = temp.periodCents;
	generatorCents->Value = temp.genCents;

  rootNote->Text = ref new String(latticeView->temperament.rootNote.c_str());
  tuningNote->Text = ref new String(latticeView->temperament.tuningNote.c_str());
	anchorFreq->Value = temp.baseFreq;

	wstringstream content;
	for (unsigned int i = 0; i < temp.namingRatios.size(); i++)
	{
		content<< temp.namingRatios[i].num << ":" << temp.namingRatios[i].denom;
		if (i < temp.namingRatios.size() - 1)
			content << " ";
	}
//	accidentals->Value = ref new Platform::String(content.str().c_str());
//  accidentals->ValueChanged = SettingsTemperament::nameRatioListChanged;
  
  initialized = true;
}

void SettingsTemperament::ManipulateMe_ManipulationStarting(Object^ sender, ManipulationStartingRoutedEventArgs^ e)
{
	e->Handled = true;
}

void SettingsTemperament::ManipulateMe_ManipulationStarted(Object^ sender, ManipulationStartedRoutedEventArgs^ e)
{
	e->Handled = true;
}

void SettingsTemperament::ManipulateMe_ManipulationInertiaStarting(Object^ sender, ManipulationInertiaStartingRoutedEventArgs^ e)
{
	e->Handled = true;
}

void SettingsTemperament::ManipulateMe_ManipulationDelta(Object^ sender, ManipulationDeltaRoutedEventArgs^ e)
{
	e->Handled = true;
}

void SettingsTemperament::ManipulateMe_ManipulationCompleted(Object^ sender, ManipulationCompletedRoutedEventArgs^ e)
{
	e->Handled = true;
}



void SDKTemplate::SettingsTemperament::addRatioBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ratioGrid->Children->InsertAt(ratioGrid->Children->Size - 1, ref new TemperamentDataItem());
	updateTemperamentRatios();
}


void SDKTemplate::SettingsTemperament::deleteRatioBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (ratioGrid->Children->Size > 1)
	{
		ratioGrid->Children->RemoveAt(ratioGrid->Children->Size - 2);
		updateTemperamentRatios();
	}
}


void SDKTemplate::SettingsTemperament::browse_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileOpenPicker^ picker = ref new FileOpenPicker();
	picker->ViewMode = PickerViewMode::List;
	picker->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;
	picker->FileTypeFilter->Append(".tpmt");

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


void SDKTemplate::SettingsTemperament::justIntonation_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if(!initialized)
    return;

	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->temperament.justIntonation = justIntonation->IsOn;
  latticeView->temperament.clearNoteNames();
	latticeView->invalidateBitmap();
}


void SDKTemplate::SettingsTemperament::anchorNote_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
  if (!initialized)
    return;

	ComboBox^ cb= (ComboBox^)sender;
	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->temperament.clearNoteNames();
	latticeView->invalidateBitmap();
}



void SDKTemplate::SettingsTemperament::updateTemperamentRatios()
{
	LatticeView^ latticeView = getLatticeView(this);
	critical_section::scoped_lock lock(latticeView->criticalSection);

	Temperament& temp = latticeView->temperament;

	temp.persIn.clear();
	temp.gensIn.clear();
	temp.ratioGeneratorsIn.clear();
	temp.enabledIn.clear();
  temp.clearNoteNames();

	for (unsigned int i = 0; i < ratioGrid->Children->Size-1; i++)
	{
		TemperamentDataItem^ child = (TemperamentDataItem^)ratioGrid->Children->GetAt(i);
		child->updateRatio(temp);
	}

	temp.calcMatricies();
  latticeView->temperament.clearNoteNames();
	latticeView->invalidateBitmap();
}


void SDKTemplate::SettingsTemperament::notenames_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);

  {
    critical_section::scoped_lock lock(latticeView->criticalSection);

    Platform::String^ str = noteNames->Text;
    wchar_t buf[MAX_CHARS_PER_LINE] = { 0 };
    wcscpy_s(buf, str->Data());

    vector<wstring> names;
    LPTSTR next_token = NULL;
    LPTSTR token = wcstok_s(buf, _T(" "), &next_token); // first token
    while(token)
    {
      names.push_back(token);
      token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
    }

    latticeView->temperament.refreshNames(names);
    latticeView->temperament.clearNoteNames();
    latticeView->invalidateBitmap();
  }
}


void SDKTemplate::SettingsTemperament::accidentals_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);

  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->temperament.inNamingRatios;
    latticeView->temperament.inNamingRatios.clear();

    {
      Platform::String^ str = accidentals->Text;
      wchar_t buf[MAX_CHARS_PER_LINE] = { 0 };
      wcscpy_s(buf, str->Data());

      vector<wstring> names;
      LPTSTR next_token = NULL;
      LPTSTR token = wcstok_s(buf, _T(" "), &next_token); // first token
  
      while (token)
      {
        if (latticeView->temperament.namingRatioMap.count(token))
        {
          latticeView->temperament.inNamingRatios.push_back(latticeView->temperament.namingRatioMap[token]);
        }

        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
      }
    }

    latticeView->temperament.namingRatios.clear();
    latticeView->temperament.noteNames.clear();
    latticeView->temperament.sharps.clear();
    latticeView->temperament.flats.clear();

    if (latticeView->temperament.inNamingRatios.size() == 0)
    {
      latticeView->temperament.inNamingRatios.push_back(latticeView->temperament.namingRatioMap[L"#"]);
      latticeView->temperament.inNamingRatios.push_back(latticeView->temperament.namingRatioMap[L"2"]);
    }

    {
      Platform::String^ str = noteNames->Text;
      wchar_t buf[MAX_CHARS_PER_LINE] = { 0 };
      wcscpy_s(buf, str->Data());

      vector<wstring> names;
      LPTSTR next_token = NULL;
      LPTSTR token = wcstok_s(buf, _T(" "), &next_token); // first token
      while (token)
      {
        names.push_back(token);
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
      }

      latticeView->temperament.refreshNames(names);
    }

    latticeView->temperament.clearNoteNames();
    latticeView->invalidateBitmap();
  }
}


void SDKTemplate::SettingsTemperament::tuningNote_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);

  {
    critical_section::scoped_lock lock(latticeView->criticalSection);

    latticeView->temperament.tuningNote = tuningNote->Text->Data();

    {
      Platform::String^ str = noteNames->Text;
      wchar_t buf[MAX_CHARS_PER_LINE] = { 0 };
      wcscpy_s(buf, str->Data());

      vector<wstring> names;
      LPTSTR next_token = NULL;
      LPTSTR token = wcstok_s(buf, _T(" "), &next_token); // first token
      while (token)
      {
        names.push_back(token);
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
      }

      latticeView->temperament.refreshNames(names);
    }

    latticeView->temperament.clearNoteNames();
    latticeView->invalidateBitmap();
  }
}


void SDKTemplate::SettingsTemperament::rootNote_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);

  {
    critical_section::scoped_lock lock(latticeView->criticalSection);

    latticeView->temperament.rootNote = rootNote->Text->Data();

    {
      Platform::String^ str = noteNames->Text;
      wchar_t buf[MAX_CHARS_PER_LINE] = { 0 };
      wcscpy_s(buf, str->Data());

      vector<wstring> names;
      LPTSTR next_token = NULL;
      LPTSTR token = wcstok_s(buf, _T(" "), &next_token); // first token
      while (token)
      {
        names.push_back(token);
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
      }

      latticeView->temperament.refreshNames(names);
    }

    latticeView->temperament.clearNoteNames();
    latticeView->invalidateBitmap();
  }
}


void SDKTemplate::SettingsTemperament::Reset_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
	if (item == nullptr)
		return;

	wstring fileName = convertStringToFileName(item->fileName.c_str());
	fileName += L".tpmt";

	create_task(item->folder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
	{
		LatticeView^ latticeView = getLatticeView(this);
    latticeView->temperament.name = item->fileName.c_str();
		latticeView->temperament.loadTemperamentAsync(item->folder).then([this, latticeView](bool)
    {
      syncCompsNoFileCombo();
      critical_section::scoped_lock lock(latticeView->criticalSection);
      latticeView->temperament.clearNoteNames();
      latticeView->invalidateBitmap();
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



void SDKTemplate::SettingsTemperament::Delete_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	if (latticeView->temperament.temperamentFolder->Path == latticeView->temperament.preloadedTemperamentFolder->Path)
	{
		MessageDialog^ dialog = ref new MessageDialog(ref new String(L"Can only delete user created temperaments."));
		dialog->Commands->Append(ref new UICommand("Ok"));

		create_task(dialog->ShowAsync());
		return;
	}

	FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
	if (item == nullptr)
		return;

	wstring fileName = convertStringToFileName(item->fileName.c_str());
	fileName += L".tpmt";

	MessageDialog^ dialog = ref new MessageDialog(ref new String((wstring(L"Are you sure you want to delete the temperament \"") + item->fileName + L"\"?").c_str()));
	dialog->Commands->Append(ref new UICommand("No"));
	dialog->Commands->Append(ref new UICommand("Yes"));


	create_task(dialog->ShowAsync()).then([=](IUICommand^ command)
	{
		if (command->Label == "Yes")
		{
			create_task(latticeView->temperament.localTemperamentFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
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
		}
	});
}

void SDKTemplate::SettingsTemperament::SaveAs_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	saveAsDialog->ShowAsync();
}

void SDKTemplate::SettingsTemperament::saveAsDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args)
{
	FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
	if (item == nullptr)
		newName->Text = "";
	else
		newName->Text = ref new String(item->fileName.c_str());
}

void SDKTemplate::SettingsTemperament::saveAsDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
	wstring fileName = convertStringToFileName(newName->Text->Data());
	fileName += L".tmpt";
	LatticeView^ latticeView = getLatticeView(this);
	create_task(latticeView->temperament.localTemperamentFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
	{
		MessageDialog^ dialog = ref new MessageDialog(ref new String((wstring(L"A user created temperament with the name \"") + newName->Text->Data() + L"\" already exists. Do you want to replace it?").c_str()));
		dialog->Commands->Append(ref new UICommand("No"));
		dialog->Commands->Append(ref new UICommand("Yes"));

		create_task(dialog->ShowAsync()).then([this, latticeView](IUICommand^ command)
		{
			if (command->Label == "Yes")
			{
				latticeView->temperament.name = newName->Text->Data();
				latticeView->temperament.temperamentFolder = latticeView->temperament.localTemperamentFolder;
				latticeView->temperament.saveTemperament().then([this]() {this->syncComponents();});
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
			latticeView->temperament.name = newName->Text->Data();
			latticeView->temperament.temperamentFolder = latticeView->temperament.localTemperamentFolder;
			latticeView->temperament.saveTemperament().then([this]() {this->syncComponents();});
		}
	});
}

void SDKTemplate::SettingsTemperament::Save_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileComboBoxItem^ item = (FileComboBoxItem^)name->SelectedItem;
	if (item == nullptr)
	{
		SaveAs_Click(sender, e);
		return;
	}

	wstring fileName = convertStringToFileName(item->fileName.c_str());
	fileName += L".tpmt";
	LatticeView^ latticeView = getLatticeView(this);
	if (latticeView->temperament.temperamentFolder->Path == latticeView->temperament.localTemperamentFolder->Path)
	{
		latticeView->temperament.saveTemperament().then([this]() {this->syncComponents();});;
	}
	else
	{
		create_task(latticeView->temperament.localTemperamentFolder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([=](StorageFile^ file)
		{
			MessageDialog^ dialog = ref new MessageDialog(ref new String((wstring(L"A user created temperament with the name \"") + item->fileName + L"\" already exists. Do you want to replace it?").c_str()));
			dialog->Commands->Append(ref new UICommand("No"));
			dialog->Commands->Append(ref new UICommand("Yes"));

			create_task(dialog->ShowAsync()).then([=](IUICommand^ command)
			{
				if (command->Label == "Yes")
				{
					latticeView->temperament.temperamentFolder = latticeView->temperament.localTemperamentFolder;
					latticeView->temperament.saveTemperament().then([this]() {this->syncComponents();});;
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
				latticeView->temperament.temperamentFolder = latticeView->temperament.localTemperamentFolder;
				latticeView->temperament.saveTemperament().then([this]() {this->syncComponents();});
			}
		});
	}
}

