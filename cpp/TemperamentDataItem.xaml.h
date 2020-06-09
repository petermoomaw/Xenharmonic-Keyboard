#pragma once

#include "TemperamentDataItem.g.h"
#include "Enharmonic/Temperament.h"


namespace SDKTemplate
{
	public ref class TemperamentDataItem sealed
	{
	public:
		TemperamentDataItem();
	internal:
		TemperamentDataItem(bool enabled, Ratio& ratio, int period, int generator);
		void updateRatio(Temperament& temp);

	public:
/////////////////////////////////////////////

		void InitManipulationTransforms();
		void ManipulateMe_ManipulationStarting(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationStartingRoutedEventArgs^ e);
		void ManipulateMe_ManipulationStarted(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationStartedRoutedEventArgs^ e);
		void ManipulateMe_ManipulationInertiaStarting(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationInertiaStartingRoutedEventArgs^ e);
		void ManipulateMe_ManipulationDelta(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationDeltaRoutedEventArgs^ e);
		void ManipulateMe_ManipulationCompleted(Platform::Object^ sender, Windows::UI::Xaml::Input::ManipulationCompletedRoutedEventArgs^ e);

	private:
		static void somethingChanged(Object^ sender);

		bool forceManipulationsToEnd;

		///////////////////////////////////////////////////
		Windows::UI::Xaml::Media::TransformGroup^ _transformGroup;
		Windows::UI::Xaml::Media::MatrixTransform^ _previousTransform;
		Windows::UI::Xaml::Media::CompositeTransform^ _compositeTransform;
		void enabled_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
};

