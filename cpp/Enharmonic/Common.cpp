#include "pch.h"
#include "Common.h"

using namespace Windows::UI::Core;
using namespace Windows::Foundation::Collections;
using namespace Microsoft::WRL;
using namespace Microsoft::Services::Store::Engagement;

void Print(LPCTSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);
  int nBuf;
  TCHAR szBuffer[512 * 4]; // get rid of this hard-coded buffer
  nBuf = _vsntprintf_s(szBuffer, 512 * 4 - 1, lpszFormat, args);
  ::OutputDebugString(szBuffer);
  va_end(args);
}

void Throw(LPCTSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);
  int nBuf;
  TCHAR szBuffer[512 * 4]; // get rid of this hard-coded buffer
  nBuf = _vsntprintf_s(szBuffer, 512 * 4 - 1, lpszFormat, args);
  ::OutputDebugString(szBuffer);
  va_end(args);

  StoreServicesCustomEventLogger^ logger = StoreServicesCustomEventLogger::GetDefault();
  logger->Log(ref new Platform::String(szBuffer));

  throw Platform::Exception::CreateException(S_FALSE, ref new Platform::String(szBuffer));
}

bool ends_with(wstring const & value, wstring const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

wstring trim(const wstring &s)
{
	auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) {return isspace(c);});
	auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {return isspace(c);}).base();
	return (wsback <= wsfront ? wstring() : wstring(wsfront, wsback));
}

void trimFileExtension(wstring& name)
{
	size_t dotIndex = name.find_last_of(L".");
	if (dotIndex != string::npos)
		name.erase(name.find_last_of(L"."), wstring::npos);
}

wstring convertStringToFileName(const wchar_t* strIn)
{
	wstring fileName;
	wstring wstr = trim(strIn);
	const wchar_t* str = wstr.c_str();
	wchar_t* ch = (wchar_t*)str;
	while (*ch != 0)
	{
		if (isspace(*ch))
		{
			fileName.push_back(L'_');
		}
	    else if (*ch >= 0x41 && *ch <= 0x5A)
		{
			fileName.push_back(L'!');
			fileName.push_back(*ch+32);
		}
		else if (*ch < ' ' || *ch >= 0x7F
			|| *ch =='<' //(less than)
			|| *ch =='>' //(greater than)
			|| *ch ==':' //(colon)
			|| *ch =='\"' //(double quote)
			|| *ch =='/' //(forward slash)
			|| *ch =='\\' //(backslash)
			|| *ch =='|' //(vertical bar or pipe)
			|| *ch =='?' //(question mark)
			|| *ch =='*' //(asterisk)
			|| (*ch == '.' && ch == str) // we don't want to collide with "." or ".."!
			|| *ch == '%'
			|| *ch == '$'
			|| *ch == '_'
			|| *ch == '!')
		{
			if (*ch <= 255)
			{
				fileName.push_back(L'%');
				wchar_t buff[3];
				buff[2] = 0;
				swprintf_s(buff, L"%02X", (int)*ch);
				fileName += buff;
			}
			else
			{
				fileName.push_back(L'$');
				wchar_t buff[5];
				buff[2] = 0;
				swprintf_s(buff, L"%04X", (int)*ch);
				fileName += buff;
			}
		}
		else 
		{
		  fileName.push_back(*ch);
		}

		ch++;
	}

	return fileName;
}

wstring convertFileNameToString(const wchar_t* fileName)
{
	wstring str;

	wchar_t* ch = (wchar_t*)fileName;
	while (*ch != 0)
	{
		if (*ch == L'_')
		{
			str.push_back(L' ');
		}
		else if (*ch == L'!')
		{
			if (*(ch + 1) >= 0x61 && *(ch + 1) <= 0x7A)
			{
				ch++;
				if (*ch == 0)
					break;

				str.push_back(*ch - 32);
			}
			else
				str.push_back(L'!');
		}
		else if (*ch == L'%')
		{
			ch++;
			if (*ch == 0 || *(ch + 1) == 0)
				break;

			wstring num;
			num.push_back(*ch++);
			num.push_back(*ch);

			long int val = wcstol(num.c_str(), NULL, 16);
			str.push_back((wchar_t)val);
		}
		else if (*ch == L'$')
		{
			ch++;
			if (*ch == 0 || *(ch+1) == 0 || *(ch + 2) == 0 || *(ch + 3) == 0)
				break;

			wstring num;
			num.push_back(*ch++);
			num.push_back(*ch++);
			num.push_back(*ch++);
			num.push_back(*ch);

     		long int val = wcstol(num.c_str(), NULL, 16);
			str.push_back((wchar_t)val);
		}
		else
		{
			str.push_back(*ch);
		}

		ch++;
	}

	return str;
}

wstring getName(StorageFile^ file)
{
	return convertFileNameToString(file->Name->Data());
}
