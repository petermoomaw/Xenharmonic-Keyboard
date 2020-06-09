#pragma once

#include "TemperamentEditRatioList.g.h"
#include "Enharmonic/Temperament.h"

namespace SDKTemplate
{
	public ref class TemperamentEditRatioList sealed
	{
	public:
		TemperamentEditRatioList();

	internal:
		property Platform::String^  Value
		{
			Platform::String^ get()
			{
				return value;
			}

			void set(Platform::String^ val)
			{
				value = val;
				textBox->Text = value;
			}
		}


	internal:
		void(*ValueChanged)(Object^ sender) = 0;

		

	private:
		void textBox_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args);
		Platform::String^ value;
	};
};

