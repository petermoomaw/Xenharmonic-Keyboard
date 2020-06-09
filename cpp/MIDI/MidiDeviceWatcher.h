//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#pragma once

#include "string.h"


//using namespace SDKTemplate;
//using namespace System;
//using namespace System::Collections::Generic;
//using namespace System::hreading::Tasks;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Midi;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Platform;

namespace MIDI
{
    ref class MidiDeviceWatcher sealed
    {
        DeviceWatcher^ deviceWatcher = nullptr;
        DeviceInformationCollection^ deviceInformationCollection = nullptr;
        bool enumerationCompleted = false;
        ComboBox^ portList = nullptr;
        String^ midiSelector = nullptr;
        CoreDispatcher^ coreDispatcher = nullptr;

	public:
		MidiDeviceWatcher(String^ midiSelectorString, CoreDispatcher^ dispatcher, ComboBox^ portListBox);
		virtual ~MidiDeviceWatcher();


		void Start();
		void Stop();
		DeviceInformationCollection^ GetDeviceInformationCollection();

	private:
		void UpdateDevices();
		void DeviceWatcher_Added(DeviceWatcher^ sender, DeviceInformation^ args);
		void DeviceWatcher_Removed(DeviceWatcher^ sender, DeviceInformationUpdate^ args);
        void DeviceWatcher_Updated(DeviceWatcher^ sender, DeviceInformationUpdate^ args);
		void DeviceWatcher_EnumerationCompleted(DeviceWatcher^ sender, Object^ args);

		Windows::Foundation::EventRegistrationToken addedToken;
		Windows::Foundation::EventRegistrationToken removedToken;
		Windows::Foundation::EventRegistrationToken updatedToken;
		Windows::Foundation::EventRegistrationToken enumerationToken;
	};
};
