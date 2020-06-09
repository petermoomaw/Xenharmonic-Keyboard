#pragma once

#include "SettingsTouchScreen.g.h"
#include "SettingsTab.h"

namespace SDKTemplate
{
	public ref class SettingsTouchScreen sealed : public SettingsTab
	{
	public:
		SettingsTouchScreen();
		void syncComponents() override;
    void PanelHidden() override;
	private:
		void speed_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void linger_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void diameter_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
	};
};
