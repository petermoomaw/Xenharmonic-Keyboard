#pragma once

#include "TemperamentEditInteger.g.h"


namespace SDKTemplate
{
	public ref class TemperamentEditInteger sealed
	{
	public:
		TemperamentEditInteger();

		property int Value
		{
			int get()
			{
				return wcstol(textBox->Text->Data(), NULL, 10);
			}

			void set(int val)
			{
				value = ref new Platform::String(std::to_wstring(val).c_str());
				textBox->Text = value;
			}
		}

		property bool NegativeAllowed
		{ 
			bool get()
			{
				return negativeAllowed;
			}

			void set(bool val)
			{
				this->negativeAllowed = val;
			}
		}

	internal:
		void(*ValueChanged)(Object^ sender) = 0;

	private:
		bool negativeAllowed = true;
		void textBox_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args);
		Platform::String^ value;
	};
};

