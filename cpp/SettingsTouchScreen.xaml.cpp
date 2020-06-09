#include "pch.h"
#include "SettingsTouchScreen.xaml.h"

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

SettingsTouchScreen::SettingsTouchScreen()
{
	InitializeComponent();
}

void SettingsTouchScreen::PanelHidden()
{
}

void SettingsTouchScreen::syncComponents()
{
	LatticeView^ latticeView = getLatticeView(this);
	speed->Value = latticeView->touchDiameterEscapeTime;
	linger->Value = latticeView->touchLingerDuration;
	diameter->Value = latticeView->touchDiameter;
}

void SDKTemplate::SettingsTouchScreen::speed_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->touchDiameterEscapeTime = e->NewValue;
}


void SDKTemplate::SettingsTouchScreen::linger_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->touchLingerDuration = e->NewValue;
}


void SDKTemplate::SettingsTouchScreen::diameter_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->touchDiameter = e->NewValue;
	latticeView->cellPathValid = false;
}

