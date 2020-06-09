#include "pch.h"
#include "TemperamentDataItem.xaml.h"
#include "TemperamentEditInteger.xaml.h"
#include "TemperamentEditRatio.xaml.h"
#include "Enharmonic\LatticeView.h"
#include "SettingsPopup.xaml.h"
#include "SettingsTemperament.xaml.h"

#include <string>

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


TemperamentDataItem::TemperamentDataItem()
{
	InitializeComponent();

	//enabled
	period->ValueChanged = somethingChanged;
	generator->ValueChanged = somethingChanged;
	ratio->ValueChanged = somethingChanged;

	forceManipulationsToEnd = false;

	DragTab->ManipulationStarting += ref new ManipulationStartingEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationStarting);
	DragTab->ManipulationStarted += ref new ManipulationStartedEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationStarted);
	DragTab->ManipulationDelta += ref new ManipulationDeltaEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationDelta);
	DragTab->ManipulationCompleted += ref new ManipulationCompletedEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationCompleted);
	DragTab->ManipulationInertiaStarting += ref new ManipulationInertiaStartingEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationInertiaStarting);

	InitManipulationTransforms();
}

TemperamentDataItem::TemperamentDataItem(bool enabled, Ratio& ratio, int period, int generator)
{
	InitializeComponent();

	this->enabled->IsChecked = enabled;
	this->ratio->Value = ratio;
	this->period->Value = period;
	this->generator->Value = generator;

	//enabled
	this->period->ValueChanged = somethingChanged;
	this->generator->ValueChanged = somethingChanged;
	this->ratio->ValueChanged = somethingChanged;

	forceManipulationsToEnd = false;

	DragTab->ManipulationStarting += ref new ManipulationStartingEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationStarting);
	DragTab->ManipulationStarted += ref new ManipulationStartedEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationStarted);
	DragTab->ManipulationDelta += ref new ManipulationDeltaEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationDelta);
	DragTab->ManipulationCompleted += ref new ManipulationCompletedEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationCompleted);
	DragTab->ManipulationInertiaStarting += ref new ManipulationInertiaStartingEventHandler(this, &TemperamentDataItem::ManipulateMe_ManipulationInertiaStarting);

	InitManipulationTransforms();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Drag Stuff


void TemperamentDataItem::InitManipulationTransforms()
{
	_transformGroup = ref new TransformGroup();
	_compositeTransform = ref new CompositeTransform();
	_previousTransform = ref new MatrixTransform();
	_previousTransform->Matrix = Windows::UI::Xaml::Media::Matrix::Identity;

	_transformGroup->Children->Append(_previousTransform);
	_transformGroup->Children->Append(_compositeTransform);

	DragLayer->RenderTransform = _transformGroup;
}

void TemperamentDataItem::ManipulateMe_ManipulationStarting(Object^ sender, ManipulationStartingRoutedEventArgs^ e)
{
	forceManipulationsToEnd = false;
	e->Handled = true;
}

void TemperamentDataItem::ManipulateMe_ManipulationStarted(Object^ sender, ManipulationStartedRoutedEventArgs^ e)
{
	e->Handled = true;
	Canvas::SetZIndex(this, Canvas::GetZIndex(this) + 400);
}

void TemperamentDataItem::ManipulateMe_ManipulationInertiaStarting(Object^ sender, ManipulationInertiaStartingRoutedEventArgs^ e)
{
	e->Handled = true;
}

void TemperamentDataItem::ManipulateMe_ManipulationDelta(Object^ sender, ManipulationDeltaRoutedEventArgs^ e)
{
	if (forceManipulationsToEnd)
	{
		e->Complete();
		return;
	}

	_previousTransform->Matrix = _transformGroup->Value;

	Point center = _previousTransform->TransformPoint(Point(e->Position.X, e->Position.Y));
	_compositeTransform->CenterX = center.X;
	_compositeTransform->CenterY = center.Y;

	//_compositeTransform->Rotation = e->Delta.Rotation;
	//_compositeTransform->ScaleX = e->Delta.Scale;;
	//_compositeTransform->ScaleY = e->Delta.Scale;
	_compositeTransform->TranslateX = e->Delta.Translation.X;
	_compositeTransform->TranslateY = e->Delta.Translation.Y;

	double deltaX = center.X + e->Delta.Translation.X;
	int width = this->Width;
	if (deltaX > width/2 || deltaX < -width/2)
	{
		StackPanel^ parent = (StackPanel^) this->Parent;

		unsigned int index;
		parent->Children->IndexOf(this, &index);
		
		if (deltaX < -width/2)
		{
			if (index > 0)
			{
				parent->Children->Move(index, index - 1);
				_compositeTransform->TranslateX += width;
			}
		}
		else if (deltaX >width/2)
		{
			if (index < parent->Children->Size-2)
			{
				parent->Children->Move(index, index + 1);
				_compositeTransform->TranslateX -= width;
			}
		}
	}

	e->Handled = true;
}

void TemperamentDataItem::ManipulateMe_ManipulationCompleted(Object^ sender, ManipulationCompletedRoutedEventArgs^ e)
{
	e->Handled = true;
	InitManipulationTransforms();
	Canvas::SetZIndex(this, Canvas::GetZIndex(this) - 400);
	somethingChanged(sender);
}

void TemperamentDataItem::somethingChanged(Object^ sender)
{
	Windows::UI::Xaml::FrameworkElement^ comp = (Windows::UI::Xaml::FrameworkElement^) sender;
	while (comp->GetType()->FullName != "SDKTemplate.SettingsTemperament")
	{
		comp = (Windows::UI::Xaml::FrameworkElement^)comp->Parent;
	}

	((SDKTemplate::SettingsTemperament^)comp)->updateTemperamentRatios();
}

void TemperamentDataItem::updateRatio(Temperament& temp)
{
	temp.enabledIn.push_back(enabled->IsChecked->Value);
	temp.persIn.push_back(period->Value);
	temp.gensIn.push_back(generator->Value);
	temp.ratioGeneratorsIn.push_back(ratio->Value);
}

void SDKTemplate::TemperamentDataItem::enabled_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	somethingChanged(sender);
}
