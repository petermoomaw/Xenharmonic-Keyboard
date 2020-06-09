#pragma once

#include "TemperamentEditRatio.g.h"
#include "Enharmonic/Temperament.h"

namespace SDKTemplate
{
	public ref class TemperamentEditRatio sealed
	{
	public:
		TemperamentEditRatio();

	internal:
		property Ratio Value
		{
			Ratio get()
			{
				Ratio ratio;
				LPTSTR buf = (LPTSTR)textBox->Text->Data();
				LPTSTR start = buf;
				LPTSTR end = buf;

				int num = wcstol(start, &end, 10);
				if (*end == _T('\0'))
				{
					ratio.num = num;
				}
				else if (start != end && *end == _T(':'))
				{
					end++;
					start = end;
					int denom = wcstol(start, &end, 10);

					if (start == end)
					{
						ratio.num = num;
					}
					else if (*end == _T('\0'))
					{
						ratio.num = num;
						ratio.denom = denom;
					}
				}

				return ratio;
			}

			void set(Ratio val)
			{
				value = ref new Platform::String((std::to_wstring(val.num) + L":"+ std::to_wstring(val.denom)).c_str());
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

