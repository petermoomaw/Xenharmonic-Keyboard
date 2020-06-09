#pragma once
#include <string>
#include "..//Enharmonic//Common.h"



#ifndef WINVER                  // Minimum platform is Windows 7
#define WINVER 0x0601
#endif

#ifndef _WIN32_WINNT            // Minimum platform is Windows 7
#define _WIN32_WINNT 0x0601
#endif

#ifndef _WIN32_WINDOWS          // Minimum platform is Windows 7
#define _WIN32_WINDOWS 0x0601
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifndef UNICODE
#define UNICODE
#endif

// Windows header files
#include <windows.h>
#include <dwrite.h>
#include <d2d1.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <memory>
#include <vector>

// Ignore unreferenced parameters, since they are very common
// when implementing callbacks.
#pragma warning(disable : 4100)


////////////////////////////////////////
// COM inheritance helpers.

// Releases a COM object and nullifies pointer.
template <typename InterfaceType>
inline void SafeRelease(InterfaceType** currentObject)
{
  if (*currentObject != NULL)
  {
    (*currentObject)->Release();
    *currentObject = NULL;
  }
}


// Acquires an additional reference, if non-null.
template <typename InterfaceType>
inline InterfaceType* SafeAcquire(InterfaceType* newObject)
{
  if (newObject != NULL)
    newObject->AddRef();

  return newObject;
}


// Sets a new COM object, releasing the old one.
template <typename InterfaceType>
inline void SafeSet(InterfaceType** currentObject, InterfaceType* newObject)
{
  SafeAcquire(newObject);
  SafeRelease(&currentObject);
  currentObject = newObject;
}


// Maps exceptions to equivalent HRESULTs,
inline HRESULT ExceptionToHResult() throw()
{
  try
  {
    throw;  // Rethrow previous exception.
  }
  catch (std::bad_alloc&)
  {
    return E_OUTOFMEMORY;
  }
  catch (...)
  {
    return E_FAIL;
  }
}

typedef std::vector<std::wstring> MFCollection;

class MFFontCollectionLoader : public IDWriteFontCollectionLoader
{
public:
  MFFontCollectionLoader() : refCount_(0)
  {
  }

  // IUnknown methods
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
  virtual ULONG STDMETHODCALLTYPE AddRef();
  virtual ULONG STDMETHODCALLTYPE Release();

  // IDWriteFontCollectionLoader methods
  virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
    IDWriteFactory* factory,
    void const* collectionKey,                      // [collectionKeySize] in bytes
    UINT32 collectionKeySize,
    OUT IDWriteFontFileEnumerator** fontFileEnumerator
  );

  // Gets the singleton loader instance.
  static IDWriteFontCollectionLoader* GetLoader()
  {
    return instance_;
  }

  static bool IsLoaderInitialized()
  {
    return instance_ != NULL;
  }

private:
  ULONG refCount_;

  static IDWriteFontCollectionLoader* instance_;
};

class MFFontFileEnumerator : public IDWriteFontFileEnumerator
{
public:
  MFFontFileEnumerator(
    IDWriteFactory* factory
  );

  HRESULT Initialize(
    UINT const* collectionKey,    // [resourceCount]
    UINT32 keySize
  );

  ~MFFontFileEnumerator()
  {
    SafeRelease(&currentFile_);
    SafeRelease(&factory_);
  }

  // IUnknown methods
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
  virtual ULONG STDMETHODCALLTYPE AddRef();
  virtual ULONG STDMETHODCALLTYPE Release();

  // IDWriteFontFileEnumerator methods
  virtual HRESULT STDMETHODCALLTYPE MoveNext(OUT BOOL* hasCurrentFile);
  virtual HRESULT STDMETHODCALLTYPE GetCurrentFontFile(OUT IDWriteFontFile** fontFile);

private:
  ULONG refCount_;

  IDWriteFactory* factory_;
  IDWriteFontFile* currentFile_;
  std::vector<std::wstring> filePaths_;
  size_t nextIndex_;
};

class MFFontContext
{
public:
  MFFontContext(IDWriteFactory *pFactory);
  ~MFFontContext();

  HRESULT Initialize();

  HRESULT CreateFontCollection(
    MFCollection &newCollection,
    OUT IDWriteFontCollection** result
  );

private:
  // Not copyable or assignable.
  MFFontContext(MFFontContext const&);
  void operator=(MFFontContext const&);

  HRESULT InitializeInternal();
  IDWriteFactory *g_dwriteFactory;
  static std::vector<unsigned int> cKeys;

  // Error code from Initialize().
  HRESULT hr_;
};

class MFFontGlobals
{
public:
  MFFontGlobals() {}
  static unsigned int push(MFCollection &addCollection)
  {
    unsigned int ret = fontCollections.size();
    fontCollections.push_back(addCollection);
    return ret;
  }
  static std::vector<MFCollection>& collections()
  {
    return fontCollections;
  }
private:
  static std::vector<MFCollection> fontCollections;
};