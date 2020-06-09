//
// SettingPerfectPitch.xaml.h
// Declaration of the SettingPerfectPitch class
//

#pragma once

#include "SettingPerfectPitch.g.h"
#include "SettingsTab.h"

namespace SDKTemplate
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SettingPerfectPitch sealed : public SettingsTab
	{
	public:
		SettingPerfectPitch();
    void syncComponents() override;
    void  syncCompsNoFileCombo();
    void PanelHidden() override;
  private:
    bool initialized = false;
    void sequenceLength_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
    void octavesBelow_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
    void octavesAbove_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
    void playSequence_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	void duration_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
	static void instrumentChecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	static void instrumentUnchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  void Reset_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  void Delete_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  void Save_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  task<void> populateFileCombo();

  void exerciseFileSelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
  void changeInstrument_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  void Restart_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  void Clear_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  void saveAsDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args);
  void saveAsDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
  void SaveAs_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

  };
}
