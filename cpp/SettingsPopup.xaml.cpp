#include "pch.h"
#include "SettingsPopup.xaml.h"

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

SettingsPopup::SettingsPopup()
{
	InitializeComponent();

	forceManipulationsToEnd = false;

	DragLayer->ManipulationStarting += ref new ManipulationStartingEventHandler(this, &SettingsPopup::ManipulateMe_ManipulationStarting);
	DragLayer->ManipulationStarted += ref new ManipulationStartedEventHandler(this, &SettingsPopup::ManipulateMe_ManipulationStarted);
	DragLayer->ManipulationDelta += ref new ManipulationDeltaEventHandler(this, &SettingsPopup::ManipulateMe_ManipulationDelta);
	DragLayer->ManipulationCompleted += ref new ManipulationCompletedEventHandler(this, &SettingsPopup::ManipulateMe_ManipulationCompleted);
	DragLayer->ManipulationInertiaStarting += ref new ManipulationInertiaStartingEventHandler(this, &SettingsPopup::ManipulateMe_ManipulationInertiaStarting);
	
	InitManipulationTransforms();

	{
		IIterator<UIElement^>^ iter = TabPanels->Children->First();
		UIElement^ child = (UIElement^)iter->Current;
		child->Visibility = Windows::UI::Xaml::Visibility::Visible;
		iter->MoveNext();

		unsigned int tabIndex = 0;
		while (iter->HasCurrent)
		{
			child = (UIElement^)iter->Current;
			child->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			iter->MoveNext();
		}
	}
}

void SettingsPopup::TabClicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ToggleButton^ btn = (ToggleButton^)sender;
	StackPanel^ panel = (StackPanel^)btn->Parent;

	LatticeView^ latticeView = getLatticeView(this);
	
  {
    critical_section::scoped_lock lock(latticeView->criticalSection);
    latticeView->playMusic = true;
    latticeView->clearTouchData();
    latticeView->invalidateBitmap();
  }

	btn->IsChecked = true;  //deal with clicking a tab which is already selected

	IIterator<UIElement^>^ iter = panel->Children->First();

	while (iter->HasCurrent)
	{
		ToggleButton^ child = (ToggleButton^)iter->Current;
		if (child != btn)
			child->IsChecked = false;

		iter->MoveNext();
	}

	unsigned int index;
	panel->Children->IndexOf(btn, &index);

	//Canvas^ canvas = (Canvas^)panel->Parent;

	{
		IIterator<UIElement^>^ iter = TabPanels->Children->First();


		unsigned int tabIndex = 0;
		while (iter->HasCurrent)
		{
			UIElement^ child = (UIElement^)iter->Current;

			if (tabIndex == index)
			{
				child->Visibility = Windows::UI::Xaml::Visibility::Visible;
				((SettingsTab^)child)->syncComponents();
			}
			else
			{
        child->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        ((SettingsTab^)child)->PanelHidden();
			}
			tabIndex++;

			iter->MoveNext();
		}
	}
}

void SettingsPopup::syncActiveTab()
{
	IIterator<UIElement^>^ iter = TabPanels->Children->First();

	LatticeView^ latticeView = getLatticeView(this);
	latticeView->playMusic = true;

	while (iter->HasCurrent)
	{
		UIElement^ child = (UIElement^)iter->Current;

		if (child->Visibility == Windows::UI::Xaml::Visibility::Visible)
		{
			((SettingsTab^)child)->syncComponents();
			return;
		}

		iter->MoveNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Popup Stuff


void SettingsPopup::InitManipulationTransforms()
{
	_transformGroup = ref new TransformGroup();
	_compositeTransform = ref new CompositeTransform();
	_previousTransform = ref new MatrixTransform();
	_previousTransform->Matrix = Windows::UI::Xaml::Media::Matrix::Identity;
	
	TransformCollection^ children = ref new TransformCollection();

	_transformGroup->Children->Append(_previousTransform);
	_transformGroup->Children->Append(_compositeTransform);

	DragLayer->RenderTransform = _transformGroup;
}

void SettingsPopup::ManipulateMe_ManipulationStarting(Object^ sender, ManipulationStartingRoutedEventArgs^ e)
{
	forceManipulationsToEnd = false;
	e->Handled = true;
}

void SettingsPopup::ManipulateMe_ManipulationStarted(Object^ sender, ManipulationStartedRoutedEventArgs^ e)
{
	e->Handled = true;
}

void SettingsPopup::ManipulateMe_ManipulationInertiaStarting(Object^ sender, ManipulationInertiaStartingRoutedEventArgs^ e)
{
	e->Handled = true;
}

void SettingsPopup::ManipulateMe_ManipulationDelta(Object^ sender, ManipulationDeltaRoutedEventArgs^ e)
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

//	_compositeTransform->Rotation = e->Delta.Rotation;
	_compositeTransform->ScaleX = e->Delta.Scale;;
	_compositeTransform->ScaleY = e->Delta.Scale;
	_compositeTransform->TranslateX = e->Delta.Translation.X;
	_compositeTransform->TranslateY = e->Delta.Translation.Y;

	e->Handled = true;
}

void SettingsPopup::ManipulateMe_ManipulationCompleted(Object^ sender, ManipulationCompletedRoutedEventArgs^ e)
{
	e->Handled = true;
}



void SDKTemplate::SettingsPopup::Close_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FrameworkElement^ comp = (FrameworkElement^)sender;

	LatticeView^ latticeView = getLatticeView(this);
	latticeView->playMusic = true;

	while (comp->GetType()->FullName != "Windows.UI.Xaml.Controls.Primitives.Popup")
	{
		comp = (Windows::UI::Xaml::FrameworkElement^)comp->Parent;
	}

	((Popup^)comp)->IsOpen = false;
}