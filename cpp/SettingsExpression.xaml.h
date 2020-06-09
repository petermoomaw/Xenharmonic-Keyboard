#pragma once

#include "SettingsExpression.g.h"
#include "SettingsTab.h"

namespace SDKTemplate
{
	public ref class SettingsExpression sealed : public SettingsTab
	{
	public:
		SettingsExpression();
		void syncComponents() override;
    void PanelHidden() override;

	private:
		void round_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void trigger_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  	bool initialized = false;
		void vibratoCuttoff_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void vibradoAnplitude_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void triggerResetRate_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void triggerThreshold_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void pitchBendAxis_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
  };
};
