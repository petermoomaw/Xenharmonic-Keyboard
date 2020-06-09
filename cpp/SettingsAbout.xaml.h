#pragma once

#include "SettingsAbout.g.h"
#include "SettingsTab.h"

namespace SDKTemplate
{
	public ref class SettingsAbout sealed : public SettingsTab
	{
	public:
		SettingsAbout();
		void syncComponents() override;
    void PanelHidden() override;
	};
};
