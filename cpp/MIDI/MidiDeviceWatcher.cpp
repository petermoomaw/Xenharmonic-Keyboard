#include "pch.h"
#include "MidiDeviceWatcher.h"

#include <ppltasks.h>   // For create_task

using namespace Concurrency;
using namespace MIDI;
using namespace Windows::Foundation;


MidiDeviceWatcher::MidiDeviceWatcher(String^ midiSelectorString, CoreDispatcher^ dispatcher, ComboBox^ portListBox)
{
	this->deviceWatcher = DeviceInformation::CreateWatcher(midiSelectorString);
	this->portList = portListBox;
	this->midiSelector = midiSelectorString;
	this->coreDispatcher = dispatcher;
	
	this->addedToken = this->deviceWatcher->Added += ref new TypedEventHandler<DeviceWatcher^, DeviceInformation^>(this, &MidiDeviceWatcher::DeviceWatcher_Added);
	this->removedToken = this->deviceWatcher->Removed += ref new TypedEventHandler<DeviceWatcher^, DeviceInformationUpdate^>(this, &MidiDeviceWatcher::DeviceWatcher_Removed);
	this->updatedToken = this->deviceWatcher->Updated += ref new TypedEventHandler<DeviceWatcher^, DeviceInformationUpdate^>(this, &MidiDeviceWatcher::DeviceWatcher_Updated);
	this->enumerationToken = this->deviceWatcher->EnumerationCompleted += ref new TypedEventHandler<DeviceWatcher^, Object^>(this, &MidiDeviceWatcher::DeviceWatcher_EnumerationCompleted);
}

MidiDeviceWatcher::~MidiDeviceWatcher()
{
	this->deviceWatcher->Added -= this->addedToken;
	this->deviceWatcher->Removed -= this->removedToken;
	this->deviceWatcher->Updated -= this->updatedToken;
	this->deviceWatcher->EnumerationCompleted -= this->enumerationToken;
}

void MidiDeviceWatcher::Start()
{
	if (this->deviceWatcher->Status != DeviceWatcherStatus::Started)
	{
		this->deviceWatcher->Start();
	}
}


void MidiDeviceWatcher::Stop()
{
	if (this->deviceWatcher->Status != DeviceWatcherStatus::Stopped)
	{
		this->deviceWatcher->Stop();
	}
}

DeviceInformationCollection^ MidiDeviceWatcher::GetDeviceInformationCollection()
{
	return this->deviceInformationCollection;
}

void MidiDeviceWatcher::UpdateDevices()
{
	// Get a list of all MIDI devices
	create_task(DeviceInformation::FindAllAsync(this->midiSelector)).then([this](DeviceInformationCollection^ devices)
	{
		this->deviceInformationCollection = devices;

		// If no devices are found, update the ListBox
		if ((this->deviceInformationCollection == nullptr) || (this->deviceInformationCollection->Size == 0))
		{
			// Start with a clean list
			this->portList->Items->Clear();

			this->portList->Items->Append("No MIDI ports found");
			this->portList->IsEnabled = false;
		}
		// If devices are found, enumerate them and add them to the list
		else
		{
			// Start with a clean list
			this->portList->Items->Clear();

			for (int i = 0; i < devices->Size; i++)
			{
				DeviceInformation^ di = devices->GetAt(i);
				this->portList->Items->Append(di->Name);
			}

			this->portList->IsEnabled = true;
		}
	}); 
}

void MidiDeviceWatcher::DeviceWatcher_Added(DeviceWatcher^ sender, DeviceInformation^ args)
{
	// If all devices have been enumerated
	if (this->enumerationCompleted)
	{
		coreDispatcher->RunAsync(CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=]()
		{
			// Update the device list
			UpdateDevices();
		}));
	}
}


void MidiDeviceWatcher::DeviceWatcher_Removed(DeviceWatcher^ sender, DeviceInformationUpdate^ args)
{
	// If all devices have been enumerated
	if (this->enumerationCompleted)
	{
		coreDispatcher->RunAsync(CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=]()
		{
			// Update the device list
			UpdateDevices();
		}));
	}
}


void MidiDeviceWatcher::DeviceWatcher_Updated(DeviceWatcher^ sender, DeviceInformationUpdate^ args)
{
	// If all devices have been enumerated
	if (this->enumerationCompleted)
	{
		coreDispatcher->RunAsync(CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=]()
		{
			// Update the device list
			UpdateDevices();
		}));
	}
}


void MidiDeviceWatcher::DeviceWatcher_EnumerationCompleted(DeviceWatcher^ sender, Object^ args)
{
	this->enumerationCompleted = true;
	coreDispatcher->RunAsync(CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=]()
	{
		// Update the device list
		UpdateDevices();
	}));
}