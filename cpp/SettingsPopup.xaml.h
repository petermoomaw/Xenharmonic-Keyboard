#pragma once

#include "SettingsPopup.g.h"
#include "SettingsTab.h"

namespace SDKTemplate
{
	public ref class SettingsPopup sealed 
	{
	public:
		SettingsPopup();
		void TabClicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void syncActiveTab();
		void InitManipulationTransforms();
		void ManipulateMe_ManipulationStarting(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationStartingRoutedEventArgs^ e);
		void ManipulateMe_ManipulationStarted(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationStartedRoutedEventArgs^ e);
		void ManipulateMe_ManipulationInertiaStarting(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationInertiaStartingRoutedEventArgs^ e);
		void ManipulateMe_ManipulationDelta(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationDeltaRoutedEventArgs^ e);
		void ManipulateMe_ManipulationCompleted(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationCompletedRoutedEventArgs^ e);

	private:
		bool forceManipulationsToEnd;

		///////////////////////////////////////////////////
		Windows::UI::Xaml::Media::TransformGroup^ _transformGroup;
		Windows::UI::Xaml::Media::MatrixTransform^ _previousTransform;
		Windows::UI::Xaml::Media::CompositeTransform^ _compositeTransform;
		void ToggleButton_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Close_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
};
