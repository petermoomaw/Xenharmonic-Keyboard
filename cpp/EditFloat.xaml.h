#pragma once

#include "EditFloat.g.h"


namespace SDKTemplate
{
	public ref class EditFloat sealed
	{
	public:
		EditFloat();


		property double Value
		{
			double get()
			{
				return wcstod(textBox->Text->Data(), NULL);
			}

			void set(double val)
			{
				wchar_t buffer[64];
				int cx = swprintf(buffer, 64, _T("%.3f"), val);

				value = ref new Platform::String(buffer);
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

