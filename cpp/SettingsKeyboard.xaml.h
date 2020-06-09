#pragma once

#include "SettingsKeyboard.g.h"
#include "SettingsTab.h"


namespace SDKTemplate
{
	public ref class SettingsKeyboard sealed : public SettingsTab
	{
	public:
		SettingsKeyboard();
		void syncComponents() override;
    void syncCompsNoFileCombo();
    void PanelHidden() override;
	private:
		bool initialized = false;
		void browse_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		task<void> populateFileCombo();

		void twoTouchMode_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void KeyShape_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void threeTouchToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void twoTouchToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void rotate90_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void flipVertical_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void flipHorizontal_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void fileMenu_Opening(Platform::Object^ sender, Platform::Object^ e);
		void Reset_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Delete_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Save_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void SaveAs_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void scale_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void resetShape_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void duplicateKeys_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void showName_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void showRatio_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void showDelta_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void showCents_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void showRatio_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void showDelta_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void showCents_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void showName_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void scientificPitchNotation_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void scientificPitchNotation_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void use53TETNotation_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void use53TETNotation_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void duplicateRatios_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void playUnnamedNotes_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void textSize_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void oneTouchEdit_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void recalibrateScale_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Edit_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Edit_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void saveAsDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args);
    void saveAsDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);

    void name_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
    void drag_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void drag_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void afterTouch_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
    void Options_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Options_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void dynamicTemperament_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void dynamicTemperament_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  };
};
