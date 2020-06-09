#pragma once

#include <string>
#include <vector>
#include <fstream>

//#include <Shobjidl.h> // for file dialog
//#include <Shellapi.h>
//#include "atlbase.h"
#include <algorithm>
#include <math.h>   

using namespace std;
using namespace Windows::Storage;
using namespace concurrency;

#define C_PAGES 3 

class Enharmonic;

const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 20;
const char* const DELIMITER = " ";

#define BUFSIZE 4096

void Print(LPCTSTR lpszFormat, ...);
void Throw(LPCTSTR lpszFormat, ...);

#ifdef _DEBUG
#define DebugThrow Throw
#else
#define DebugThrow
#endif

#ifdef _DEBUG
#define PRINT Print
#else
#define PRINT
#endif

struct FileInfo
{
	wstring name;
	wstring file;
};

bool ends_with(std::wstring const & value, std::wstring const & ending);
void trimFileExtension(wstring& name);
wstring trim(const wstring &s);
wstring convertStringToFileName(const wchar_t* str);
wstring convertFileNameToString(const wchar_t* fileName);
wstring getName(StorageFile^ file);

//wstring makeFileName(const wstring& in);
//int fileExists(const wstring& path);
//int getFileIndex(wstring file, vector<FileInfo>& fileInfos);

//template <typename TResult>
//TResult PerformSynchronously(Windows::Foundation::IAsyncOperation<TResult>^ asyncOp)
//{
//	Concurrency::event synchronizer;
//	//Concurrency::task<TResult>
//	create_task(asyncOp).then([&](TResult taskResult) {
//		synchronizer.set();
//	}, Concurrency::task_continuation_context::use_arbitrary());
//	synchronizer.wait();
//	return asyncOp->GetResults();
//}
