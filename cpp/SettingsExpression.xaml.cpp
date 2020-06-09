#include "pch.h"
#include "SettingsExpression.xaml.h"

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

SettingsExpression::SettingsExpression()
{
	InitializeComponent();

  for (int i = 1; i <= 127; i++)
  {
    wstringstream content;
    content << L"±" << i*100 << L"¢";
    pitchBendRange->Items->Append(ref new String(content.str().c_str()));
  }

	initialized = true;
}

void SettingsExpression::PanelHidden()
{
}

void SettingsExpression::syncComponents()
{
	LatticeView^ latticeView = getLatticeView(this);

	/*
	double vibratoCuttoffFreq;   // cutoff frequance in herts
	double vibratoAmplitude;      // in cents;
	double triggerFreq;
	double triggerAmplitude;
	DragMode dragMode;
	*/

  switch (latticeView->pitchAxis)
  {
  case BOTH:
    pitchBendAxis->SelectedIndex = 0;
    break;

  case HORIZONTAL:
    pitchBendAxis->SelectedIndex = 1;
    break;

  case VERTICAL:
    pitchBendAxis->SelectedIndex = 2;
    break;
  }


	round->IsOn = latticeView->roundFirstNote;
	trigger->IsOn = latticeView->vibratoTrigger;

	vibratoCuttoff->Value = latticeView->vibratoCuttoff;
	vibradoAnplitude->Value = latticeView->vibratoAmplitude;
	triggerResetRate->Value = latticeView->triggerResetRate;
	triggerThreshold->Value = latticeView->triggerThreshold;
};


void SDKTemplate::SettingsExpression::round_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->roundFirstNote = round->IsOn;
}


void SDKTemplate::SettingsExpression::trigger_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->vibratoTrigger = trigger->IsOn;
}


void SDKTemplate::SettingsExpression::vibratoCuttoff_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->vibratoCuttoff = vibratoCuttoff->Value;
}


void SDKTemplate::SettingsExpression::vibradoAnplitude_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->vibratoAmplitude = vibradoAnplitude->Value;
}


void SDKTemplate::SettingsExpression::triggerResetRate_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->triggerResetRate = triggerResetRate->Value;
}


void SDKTemplate::SettingsExpression::triggerThreshold_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	LatticeView^ latticeView = getLatticeView(this);
	latticeView->triggerThreshold = triggerThreshold->Value;
}



void SDKTemplate::SettingsExpression::pitchBendAxis_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
  LatticeView^ latticeView = getLatticeView(this);
  critical_section::scoped_lock lock(latticeView->criticalSection);
  switch(pitchBendAxis->SelectedIndex)
  {  
     case 0:
       latticeView->pitchAxis = BOTH;
       break;

     case 1:
       latticeView->pitchAxis = HORIZONTAL;
       break;

     case 2:
       latticeView->pitchAxis = VERTICAL;
       break;
  }
}
