#include "pch.h"
#include "TemperamentEditInteger.xaml.h"
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

TemperamentEditInteger::TemperamentEditInteger()
{
	InitializeComponent();
}


void SDKTemplate::TemperamentEditInteger::textBox_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args)
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
	if (negativeAllowed)
	{
		wchar_t c = *(s->Data() + i);
		if (c != L'-' && !isdigit(c))
			valid = false;

		i++;
	}

	for (;i < s->Length(); i++)
	{
		wchar_t c = *(s->Data() + i);
		if (!isdigit(c))
			valid = false;
	}

	if (!valid)
	{
		tb->Text = value;
		tb->SelectionStart = st > 0 ? st - 1:0;
		return;
	}

	value = tb->Text;

	if (ValueChanged)
	{
		ValueChanged(this);
	}
}