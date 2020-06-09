#pragma once

#include "SettingsTemperament.g.h"
#include "SettingsTab.h"

namespace SDKTemplate
{
	public ref class SettingsTemperament sealed : public SettingsTab
	{
	public:
		SettingsTemperament();
		void syncComponents() override;
    void PanelHidden() override;
		void syncCompsNoFileCombo();
 
	//internal:
		void updateTemperamentRatios();

	private:
    bool initialized = false;
    task<void> populateFileCombo();
    task<void> populateFontCombo();
		void temperamentSelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

		static void periodCentsChanged(Object^ sender);
		static void generatorCentsChanged(Object^ sender);
		static void anchorFreqChanged(Object^ sender);

		//These are just to consume events to eliminate some wierdness.
		void ManipulateMe_ManipulationStarting(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationStartingRoutedEventArgs^ e);
		void ManipulateMe_ManipulationStarted(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationStartedRoutedEventArgs^ e);
		void ManipulateMe_ManipulationInertiaStarting(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationInertiaStartingRoutedEventArgs^ e);
		void ManipulateMe_ManipulationDelta(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationDeltaRoutedEventArgs^ e);
		void ManipulateMe_ManipulationCompleted(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationCompletedRoutedEventArgs^ e);
		void addRatioBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void deleteRatioBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void browse_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void justIntonation_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void anchorNote_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void notenames_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);

    void accidentals_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
    void tuningNote_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
    void rootNote_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);

	void Reset_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	void Delete_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	void Save_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	void SaveAs_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	void saveAsDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args);
	void saveAsDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
  };
};
