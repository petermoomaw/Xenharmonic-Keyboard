#pragma once
#include "Enharmonic\LatticeView.h"
#include "MainPage.xaml.h"


LatticeView^ getLatticeView(Windows::UI::Xaml::FrameworkElement^ comp);

namespace SDKTemplate
{
	public interface class SettingsTab 
	{
	public:
		void syncComponents();
    void PanelHidden();
	};
}

