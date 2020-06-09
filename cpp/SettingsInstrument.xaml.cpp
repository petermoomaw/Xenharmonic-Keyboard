#include "pch.h"
#include "SettingsInstrument.xaml.h"


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
using namespace MIDI;

SettingsInstrument::SettingsInstrument()
{
	InitializeComponent();

	for (unsigned int i = 0; i < 128; i++)
	{
		msb->Items->Append(ref new String(std::to_wstring(i).c_str()));
		lsb->Items->Append(ref new String(std::to_wstring(i).c_str()));
	}

	setProg->IsOn = true;


	// Set up the MIDI output device watcher
	this->midiOutDeviceWatcher = ref new MidiDeviceWatcher(MidiOutPort::GetDeviceSelector(), Dispatcher, devices);

	// Start watching for devices
	this->midiOutDeviceWatcher->Start();
}

void SettingsInstrument::PanelHidden()
{
}

void SettingsInstrument::syncComponents()
{
	LatticeView^ latticeView = getLatticeView(this);
	
  if(program->Items->Size == 0)
  {
    for (int i = 0; i < latticeView->instrument.programs.size(); i++)
    {
      program->Items->Append(ref new String(latticeView->instrument.programs[i].c_str()));
    }
  }

	program->SelectedIndex = latticeView->instrument.midi_program;
	msb->SelectedIndex = latticeView->instrument.midi_bank_MSB;
	lsb->SelectedIndex = latticeView->instrument.midi_bank_LSB;
	velocity->Value = latticeView->instrument.velocity;

	if (latticeView->instrument.device != nullptr)
	{
		DeviceInformationCollection^ devInfoCollection = this->midiOutDeviceWatcher->GetDeviceInformationCollection();
		if (devInfoCollection != nullptr)
		{
			for (unsigned int i = 0; i < devInfoCollection->Size; i++)
			{
				if (devInfoCollection->GetAt(i)->Id == latticeView->instrument.device->DeviceId)
				{
					bool ison = setProg->IsOn;
					setProg->IsOn = false;
					devices->SelectedIndex = i;
					setProg->IsOn = ison;
					break;
				}
			}
			return;
		}
	}
	else
	{
		devices->SelectedIndex = -1;
	}
};

void SDKTemplate::SettingsInstrument::devices_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->instrument.closeMidiPort();

	// Get the selected output MIDI device
	int selectedOutputDeviceIndex = this->devices->SelectedIndex;
	if (selectedOutputDeviceIndex == -1)
	{
		latticeView->instrument.device = nullptr;
		return;
	}


	DeviceInformationCollection^ devInfoCollection = this->midiOutDeviceWatcher->GetDeviceInformationCollection();
	
	if (devInfoCollection == nullptr)
	{
		latticeView->instrument.device = nullptr;
		return;
	}

	if (selectedOutputDeviceIndex >= devInfoCollection->Size)
		return;

	DeviceInformation^ devInfo = devInfoCollection->GetAt(selectedOutputDeviceIndex);
	if (devInfo == nullptr)
	{
		latticeView->instrument.device = nullptr;
		return;
	}

	create_task(MidiOutPort::FromIdAsync(devInfo->Id)).then([this](IMidiOutPort^ port)
	{
		LatticeView^ latticeView = getLatticeView(this);
		latticeView->instrument.device = port;
		if (setProg->IsOn)
		{
			latticeView->instrument.updateMidiProgram();
		}
		else
		{
			this->program->SelectedIndex = -1;
			this->lsb->SelectedIndex = -1;
			this->msb->SelectedIndex = -1;
		}
	});
}


void SDKTemplate::SettingsInstrument::velocity_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->instrument.velocity = e->NewValue;
}


void SDKTemplate::SettingsInstrument::program_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
//	if (this->program->SelectedIndex < 0 || this->program->SelectedIndex > 127)
//		return;

	LatticeView^ latticeView = getLatticeView(this);
	latticeView->instrument.midi_program = this->program->SelectedIndex;
	latticeView->instrument.updateMidiProgram();
}


void SDKTemplate::SettingsInstrument::msb_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
//	if (this->msb->SelectedIndex < 0 || this->program->SelectedIndex > 127)
//		return;

	LatticeView^ latticeView = getLatticeView(this);
	latticeView->instrument.midi_bank_MSB = this->msb->SelectedIndex;
	latticeView->instrument.updateMidiProgram();
}


void SDKTemplate::SettingsInstrument::lsb_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
//	if (this->lsb->SelectedIndex < 0 || this->program->SelectedIndex > 127)
//		return;

	LatticeView^ latticeView = getLatticeView(this);
	latticeView->instrument.midi_bank_LSB= this->lsb->SelectedIndex;
	latticeView->instrument.updateMidiProgram();
}
