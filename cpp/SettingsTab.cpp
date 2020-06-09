#include "pch.h"
#include "SettingsTab.h"

LatticeView^ getLatticeView(Windows::UI::Xaml::FrameworkElement^ comp)
{
	while (comp->GetType()->FullName != "SDKTemplate.MainPage")
	{
		comp = (Windows::UI::Xaml::FrameworkElement^)comp->Parent;

    if (comp == nullptr)
    {
      return nullptr;
    }
	}

	return ((SDKTemplate::MainPage^)comp)->latticeView;
}

