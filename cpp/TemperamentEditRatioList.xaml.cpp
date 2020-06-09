#include "pch.h"
#include "TemperamentEditRatioList.xaml.h"

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

TemperamentEditRatioList::TemperamentEditRatioList()
{
	InitializeComponent();
}

void SDKTemplate::TemperamentEditRatioList::textBox_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args)
{
	TextBox^ tb = safe_cast<TextBox^>(sender);
	Platform::String^ s = tb->Text;

	if (s->Length() == 0)
	{
		value = tb->Text;
		return;
	}


	int st = tb->SelectionStart;

	bool valid = true;
	unsigned int i = 0;
	bool foundSemi = false;
	bool zeroAllowed = false;
	for (;i < s->Length(); i++)
	{
		wchar_t c = *(s->Data() + i);

        if (c == L' ')
		{
			foundSemi = false;
		}
        else if (c == L':')
		{
			if (foundSemi || i == 0)
				valid = false;
			else
				foundSemi = true;

			zeroAllowed = false;
		}
		else if (!isdigit(c))
		{
			valid = false;
		}
		else if(!zeroAllowed && c == L'0')
		{
			valid = false;
		}
		else
		{
			zeroAllowed = true;
		}
	}

	if (! valid)
	{
		tb->Text = value;
		tb->SelectionStart = st > 0 ? st - 1 : 0;
		return;
	}

	value = tb->Text;

	if (ValueChanged)
	{
		ValueChanged(this);
	}
}