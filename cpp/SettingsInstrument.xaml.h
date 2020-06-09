#pragma once

#include "SettingsInstrument.g.h"
#include "SettingsTab.h"
#include "MIDI\\MidiDeviceWatcher.h"

namespace SDKTemplate
{
	public ref class SettingsInstrument sealed : public SettingsTab 
	{
	public:
		SettingsInstrument();
		void syncComponents() override;
    void PanelHidden() override;
	private:

		MIDI::MidiDeviceWatcher^ midiOutDeviceWatcher;
		void devices_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void velocity_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void program_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void msb_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void lsb_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
	};
};
