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

#include "pch.h"
#include "MainPage.xaml.h"
#include "SettingsPopup.xaml.h"
#include "Enharmonic\LatticeView.h"

using namespace SDKTemplate;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;

MainPage::MainPage()
{
  initialized = false;
	InitializeComponent();


  mainView = Windows::ApplicationModel::Core::CoreApplication::GetCurrentView();

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &MainPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged +=
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &MainPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &MainPage::OnSwapChainPanelSizeChanged);


//	ScaleTransform^ trans = ref new ScaleTransform();
//	trans->ScaleX = .5;
//	swapChainPanel->RenderTransform = trans;


	Application::Current->Suspending +=
		ref new SuspendingEventHandler(this, &MainPage::OnSuspending);

	// Disable all pointer visual feedback for better performance when touching.
	auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
	pointerVisualizationSettings->IsContactFeedbackEnabled = false;
	pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;

	// At this point we have access to the device and can create the device-dependent resources
	// and set the swap chain panel to the XAML SwapChainPanel element
	latticeView = ref new LatticeView();
	latticeView->SetSwapChainPanel(swapChainPanel);
  latticeView->mainPage = this;

	window->SizeChanged +=
		ref new TypedEventHandler<CoreWindow ^, WindowSizeChangedEventArgs ^>(this, &MainPage::OnSizeChanged);

	//Check initial width of app window, and set the rect to the approriate size
	main = std::unique_ptr<Main>(new Main(latticeView));

	//	main->StartRenderLoop();

	// Register our SwapChainPanel to get input pointer events on a dedicated input thread and off the UI thread
	auto inputWorkItemHandler = ref new WorkItemHandler([this](IAsyncAction ^)
	{
		// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
		coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			CoreInputDeviceTypes::Mouse |
			CoreInputDeviceTypes::Touch |
			CoreInputDeviceTypes::Pen
			);

		// Register for pointer events, which will be raised on the dedicated input thread.
		coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerPressed);
		coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerMoved);
		coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerReleased);
		coreInput->PointerEntered += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerEntered);
		coreInput->PointerExited += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerExited);
		coreInput->PointerCaptureLost += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerReleased);
		coreInput->PointerWheelChanged += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerReleased);
		coreInput->InputEnabled += ref new TypedEventHandler<Object^, InputEnabledEventArgs^>(this, &MainPage::OnInputEnabled);
		
		// event loop
		while (true)
		{
			coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
      latticeView->updateTouchTimes();  
		}
	});

	// Run input task on a dedicated high priority background thread.
	inputLoopWorker = ThreadPool::RunAsync(inputWorkItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
	//inputLoopWorker = ThreadPool::RunAsync(inputWorkItemHandler, WorkItemPriority::Low, WorkItemOptions::None);

  initialized = true;
}

MainPage::~MainPage()
{
	// Stop rendering and processing events on destruction.
	main->StopRenderLoop();
	coreInput->Dispatcher->StopProcessEvents();
}

//LatticeView^ MainPage::getLatticeView()
//{
//	return latticeView;
//}

void MainPage::OnSizeChanged(CoreWindow ^sender, WindowSizeChangedEventArgs ^args)
{
//	critical_section::scoped_lock lock(main->GetCriticalSection());
//	latticeView->refresh();
}

////////////////////////////////////////////////////////////////////////////////////

void MainPage::OnPointerPressed(Object^ sender, PointerEventArgs^ args)
{
	latticeView->touchDown(args);
}

void MainPage::OnPointerMoved(Object^ sender, PointerEventArgs^ args)
{
	latticeView->touchMoved(args);
}

void MainPage::OnPointerReleased(Object^ sender, PointerEventArgs^ args)
{
	latticeView->touchUp(args);
}

void MainPage::OnPointerEntered(Object^ sender, PointerEventArgs^ args)
{
//	latticeView->touchDown(args);
}


void MainPage::OnPointerExited(Object^ sender, PointerEventArgs^ args)
{
	latticeView->touchUp(args);
}

void MainPage::OnInputEnabled(Object^ sender, InputEnabledEventArgs^ args)
{
	latticeView->stopPlayingAllNotes();
}

////////////////////////////////////////////////////////////////////////////////////


void MainPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
}

void MainPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ args)
{
	critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->SetLogicalSize(args->NewSize);
}

void MainPage::OnSuspending(Object^ /* sender */, Windows::ApplicationModel::SuspendingEventArgs^ /* e */)
{
	critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->Trim();
}

// Window event handlers.
void MainPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	windowVisible = args->Visible;
	if (windowVisible)
	{
		main->StartRenderLoop();
	}
	else
	{
		main->StopRenderLoop();
	}
}

// DisplayInformation event handlers.
void MainPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->SetDpi(sender->LogicalDpi);
}

void MainPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
//	critical_section::scoped_lock lock(main->GetCriticalSection());
//	latticeView->SetCurrentOrientation(sender->CurrentOrientation);
}

void MainPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(latticeView->criticalSection);
	latticeView->ValidateDevice();
}



void SDKTemplate::MainPage::Settings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	settingsPopup->IsOpen = true;
	((SettingsPopup^)settingsPopup->Child)->InitManipulationTransforms();
	((SettingsPopup^)settingsPopup->Child)->syncActiveTab();
}


void SDKTemplate::MainPage::ticks_LayoutUpdated(Platform::Object^ sender, Platform::Object^ e)
{
  layoutTicks();
}

void SDKTemplate::MainPage::layoutTicks()
{
  if(!initialized || keyboardControls->Visibility == Windows::UI::Xaml::Visibility::Collapsed || ticks->Children->Size == 0)
    return;
    
  double widthPerEdo = (edoView->ActualWidth*(ticks->Children->Size+1))/ ticks->Children->Size - 10;
//  if(widthPerEdo < 25)
    widthPerEdo = 25;
//  ticks->Width = width; 
//  double widthPerEdo = 30;
  double width = ticks->Children->Size * widthPerEdo;
  ticks->Width = width;
  sliderEDO->Width = width - (widthPerEdo-8);
  sliderEDO->Margin = Thickness((widthPerEdo - 8)/2,-5,0,0);
  double remainingWidth = width;
  for (int i = 0; i < ticks->Children->Size; i++)
  {
    FrameworkElement^ label = (FrameworkElement^) ticks->Children->GetAt(i);
    label->Width = remainingWidth / (ticks->Children->Size-i);
    label->SetValue(Canvas::LeftProperty, width- remainingWidth);
    remainingWidth -= label->Width;
  }

  if (latticeView->EDOs)
    sliderEDO->StepFrequency = 1;
  else
    sliderEDO->StepFrequency = 0.01;
}

void SDKTemplate::MainPage::setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility visibility)
{
  keyboardControls->Visibility = visibility;
}

void SDKTemplate::MainPage::lockKeyPattern_Changed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->lockKeyPattern = lockKeyPattern->IsChecked->Value;
  latticeView->generateKeyboard(latticeView->EDO);
}

void SDKTemplate::MainPage::lockYcents_Changed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->fixedYCentsScale = lockYcents->IsChecked->Value;
  latticeView->generateKeyboard(latticeView->EDO);
}



void SDKTemplate::MainPage::interpolateEDOs_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->EDOs = interpolateEDOs->IsChecked->Value;
  latticeView->generateKeyboard(latticeView->EDO);
  if(latticeView->EDOs)
    sliderEDO->StepFrequency = 1;
  else
    sliderEDO->StepFrequency = 0.01;
}


void SDKTemplate::MainPage::sliderEDO_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->generateKeyboard(sliderEDO->Value, true);
}



void SDKTemplate::MainPage::ColsPlus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->columns++;
  latticeView->generateKeyboard(latticeView->EDO);
  ColsText->Text = ref new String(to_wstring(latticeView->columns).c_str());
}


void SDKTemplate::MainPage::ColsMinus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->columns--;
  if(latticeView->columns < 2)
    latticeView->columns = 2;

  latticeView->generateKeyboard(latticeView->EDO);
  ColsText->Text = ref new String(to_wstring(latticeView->columns).c_str());
}

void SDKTemplate::MainPage::initializeKeyboardControls()
{
  if (!initialized)
    return;

  initialized = false;
  LatticeView^ latticeView = this->latticeView;
  ColsText->Text = ref new String(to_wstring(latticeView->columns).c_str());
  minEdoText->Text = ref new String(to_wstring(latticeView->minEDO).c_str());
  maxEdoText->Text = ref new String(to_wstring(latticeView->maxEDO).c_str());
  lockKeyPattern->IsChecked = latticeView->lockKeyPattern;
  interpolateEDOs->IsChecked = latticeView->EDOs;

  if (latticeView->dragKeyboard)
  {
    dragBtn->IsChecked = true;
  }
  else
  {
    dragBtn->IsChecked = false;
  }

  sliderZoom->Value = latticeView->scale;
  sliderShift->Value = latticeView->shiftPercent*100;

  sliderEDO->Value = (double)latticeView->EDO;
  sliderEDO->Minimum = (double) latticeView->minEDO;
  sliderEDO->Maximum= (double) latticeView->maxEDO;


  ticks->Children->Clear();
  for (int i = latticeView->minEDO; i <= latticeView->maxEDO; i++)
  {
    TextBlock^ label = ref new TextBlock();
    label->Text = ref new String(to_wstring(i).c_str());
    label->TextAlignment = TextAlignment::Center;
    ticks->Children->Append(label);
  }

  layoutTicks();
  initialized = true;
}



void SDKTemplate::MainPage::dragBtn_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->dragKeyboard = dragBtn->IsChecked->Value;
}


void SDKTemplate::MainPage::minEdoPlus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->minEDO++;

  if (latticeView->minEDO > latticeView->maxEDO-1)
    latticeView->minEDO = latticeView->maxEDO - 1;

  if (latticeView->EDO < latticeView->minEDO)
    latticeView->generateKeyboard(latticeView->minEDO + 1);
  else
    latticeView->generateKeyboard(latticeView->EDO);

  initializeKeyboardControls();
}


void SDKTemplate::MainPage::minEdoMinus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->minEDO--;
  if (latticeView->minEDO < 2)
    latticeView->minEDO = 2;

  latticeView->generateKeyboard(latticeView->EDO);
  initializeKeyboardControls();
}


void SDKTemplate::MainPage::maxEdoMinus_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->maxEDO--;
  if (latticeView->maxEDO < latticeView->minEDO + 1)
    latticeView->maxEDO = latticeView->minEDO + 1;

  if (latticeView->EDO < latticeView->minEDO)
    latticeView->generateKeyboard(latticeView->maxEDO);
  else
    latticeView->generateKeyboard(latticeView->EDO);

  initializeKeyboardControls();
}


void SDKTemplate::MainPage::maxEdoPluss_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->maxEDO++;

  latticeView->generateKeyboard(latticeView->EDO);
  initializeKeyboardControls();
}

int SDKTemplate::MainPage::getRibbonHeight()
{
  if(latticeView->dynamicTemperament)
    return 90;
  else
    return 0;

  //return keyboardControls->Height;
}

void SDKTemplate::MainPage::sliderShift_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->shiftPercent = .01 *sliderShift->Value;
  latticeView->generateKeyboard(sliderEDO->Value, true);
}


void SDKTemplate::MainPage::sliderZoom_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->clearTouchData();
  latticeView->scale = sliderZoom->Value;
  latticeView->invalidateCellPath();
}


void SDKTemplate::MainPage::aSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->a = aSlider->Value;
  latticeView->invalidateCellPath();
}


void SDKTemplate::MainPage::cSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->c = cSlider->Value;
  latticeView->invalidateCellPath();
}


void SDKTemplate::MainPage::sSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  critical_section::scoped_lock lock(latticeView->criticalSection);
  latticeView->shift = sSlider->Value;
  latticeView->invalidateCellPath();
}

void SDKTemplate::MainPage::setColor(Windows::UI::Color c)
{
  colorPicker->Color = c;
}

void SDKTemplate::MainPage::colorPicker_ColorChanged(Windows::UI::Xaml::Controls::ColorPicker^ sender, Windows::UI::Xaml::Controls::ColorChangedEventArgs^ args)
{
  if (!initialized)
    return;

  LatticeView^ latticeView = this->latticeView;
  if (latticeView->updateColor)
  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
//    latticeView->UpdateColor(colorPicker->Color);
    latticeView->invalidateCellPath();
  }
}
