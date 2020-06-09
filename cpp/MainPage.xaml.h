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

#include "MainPage.g.h"


#include "Main.h"
ref class LatticeView;


namespace SDKTemplate
{
	// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>

	public ref class MainPage sealed
	{
	public:
		MainPage();
		virtual ~MainPage();
	//	LatticeView^ getLatticeView();

	private:
    bool initialized = false;

		// Window event handlers.
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnSizeChanged(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::WindowSizeChangedEventArgs ^args);

		// DisplayInformation event handlers.
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		// Other event handlers.
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ args);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);

		// Track our independent input on a background worker thread.
		Windows::Foundation::IAsyncAction^ inputLoopWorker;
internal:
		Windows::UI::Core::CoreIndependentInputSource^ coreInput;
private:
		// Independent input handling functions.
		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerEntered(Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerExited(Object^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnInputEnabled(Object^ sender, Windows::UI::Core::InputEnabledEventArgs^ args);


		// Resources used to render the DirectX content in the XAML page background.
//		std::shared_ptr<DX::DeviceResources> deviceResources;

	internal:	
		LatticeView^ latticeView;
    void setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility visibility);
    void SDKTemplate::MainPage::initializeKeyboardControls();
    int getRibbonHeight();
    void setColor(Windows::UI::Color c);
    Windows::ApplicationModel::Core::CoreApplicationView^ mainView;
	private:
		std::unique_ptr<SDKTemplate::Main> main;
		bool windowVisible;

		void Settings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void SDKTemplate::MainPage::layoutTicks();
    void ticks_LayoutUpdated(Platform::Object^ sender, Platform::Object^ e);
    void lockKeyPattern_Changed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void lockYcents_Changed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void sliderEDO_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void ColsPlus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void ColsMinus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    void interpolateEDOs_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void dragBtn_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void minEdoPlus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void minEdoMinus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void maxEdoMinus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void maxEdoPluss_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void sliderShift_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void sliderZoom_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void aSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void cSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void sSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
    void colorPicker_ColorChanged(Windows::UI::Xaml::Controls::ColorPicker^ sender, Windows::UI::Xaml::Controls::ColorChangedEventArgs^ args);
  };

	//ref class SettingsPopup : Windows::UI::Xaml::Controls::UserControl 
	//{

	//};
}

