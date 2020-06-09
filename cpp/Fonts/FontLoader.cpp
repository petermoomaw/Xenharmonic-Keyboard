#include "pch.h"
#include "FontLoader.h"
#include "..//Enharmonic//Common.h"

IDWriteFontCollectionLoader* MFFontCollectionLoader::instance_(
  new(std::nothrow) MFFontCollectionLoader()
);

HRESULT STDMETHODCALLTYPE MFFontCollectionLoader::QueryInterface(REFIID iid, void** ppvObject)
{
  if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontCollectionLoader))
  {
    *ppvObject = this;
    AddRef();
    return S_OK;
  }
  else
  {
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }
}

ULONG STDMETHODCALLTYPE MFFontCollectionLoader::AddRef()
{
  return InterlockedIncrement(&refCount_);
}

ULONG STDMETHODCALLTYPE MFFontCollectionLoader::Release()
{
  ULONG newCount = InterlockedDecrement(&refCount_);
  if (newCount == 0)
    delete this;

  return newCount;
}

HRESULT STDMETHODCALLTYPE MFFontCollectionLoader::CreateEnumeratorFromKey(
  IDWriteFactory* factory,
  void const* collectionKey,                      // [collectionKeySize] in bytes
  UINT32 collectionKeySize,
  OUT IDWriteFontFileEnumerator** fontFileEnumerator
)
{
  *fontFileEnumerator = NULL;

  HRESULT hr = S_OK;

  if (collectionKeySize % sizeof(UINT) != 0)
    return E_INVALIDARG;

  MFFontFileEnumerator* enumerator = new(std::nothrow) MFFontFileEnumerator(
    factory
  );
  if (enumerator == NULL)
    return E_OUTOFMEMORY;

  UINT const* mfCollectionKey = static_cast<UINT const*>(collectionKey);
  UINT32 const mfKeySize = collectionKeySize;

  hr = enumerator->Initialize(
    mfCollectionKey,
    mfKeySize
  );

  if (FAILED(hr))
  {
    delete enumerator;
    return hr;
  }

  *fontFileEnumerator = SafeAcquire(enumerator);

  return hr;
}

// ------------------------------ MFFontFileEnumerator ----------------------------------------------------------

MFFontFileEnumerator::MFFontFileEnumerator(
  IDWriteFactory* factory
) :
  refCount_(0),
  factory_(SafeAcquire(factory)),
  currentFile_(),
  nextIndex_(0)
{
}

HRESULT MFFontFileEnumerator::Initialize(
  UINT const* collectionKey,    // [resourceCount]
  UINT32 keySize
)
{
  try
  {
    // dereference collectionKey in order to get index of collection in MFFontGlobals::fontCollections vector
    UINT cPos = *collectionKey;
    for (MFCollection::iterator it = MFFontGlobals::collections()[cPos].begin(); it != MFFontGlobals::collections()[cPos].end(); ++it)
    {
      filePaths_.push_back(it->c_str());
    }
  }
  catch (...)
  {
    return ExceptionToHResult();
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFFontFileEnumerator::QueryInterface(REFIID iid, void** ppvObject)
{
  if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileEnumerator))
  {
    *ppvObject = this;
    AddRef();
    return S_OK;
  }
  else
  {
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }
}

ULONG STDMETHODCALLTYPE MFFontFileEnumerator::AddRef()
{
  return InterlockedIncrement(&refCount_);
}

ULONG STDMETHODCALLTYPE MFFontFileEnumerator::Release()
{
  ULONG newCount = InterlockedDecrement(&refCount_);
  if (newCount == 0)
    delete this;

  return newCount;
}

HRESULT STDMETHODCALLTYPE MFFontFileEnumerator::MoveNext(OUT BOOL* hasCurrentFile)
{
  HRESULT hr = S_OK;

  *hasCurrentFile = FALSE;
  SafeRelease(&currentFile_);

  if (nextIndex_ < filePaths_.size())
  {
    hr = factory_->CreateFontFileReference(
      filePaths_[nextIndex_].c_str(),
      NULL,
      &currentFile_
    );

    if (SUCCEEDED(hr))
    {
      *hasCurrentFile = TRUE;

      ++nextIndex_;
    }
  }

  return hr;
}

HRESULT STDMETHODCALLTYPE MFFontFileEnumerator::GetCurrentFontFile(OUT IDWriteFontFile** fontFile)
{
  *fontFile = SafeAcquire(currentFile_);

  return (currentFile_ != NULL) ? S_OK : E_FAIL;
}

// ---------------------------------------- MFFontContext ---------------------------------------------------------

MFFontContext::MFFontContext(IDWriteFactory *pFactory) : hr_(S_FALSE), g_dwriteFactory(pFactory)
{
}

MFFontContext::~MFFontContext()
{
  g_dwriteFactory->UnregisterFontCollectionLoader(MFFontCollectionLoader::GetLoader());
}

HRESULT MFFontContext::Initialize()
{
  if (hr_ == S_FALSE)
  {
    hr_ = InitializeInternal();
  }
  return hr_;
}

HRESULT MFFontContext::InitializeInternal()
{
  HRESULT hr = S_OK;

  if (!MFFontCollectionLoader::IsLoaderInitialized())
  {
    return E_FAIL;
  }

  // Register our custom loader with the factory object.
  hr = g_dwriteFactory->RegisterFontCollectionLoader(MFFontCollectionLoader::GetLoader());

  return hr;
}

HRESULT MFFontContext::CreateFontCollection(
  MFCollection &newCollection,
  OUT IDWriteFontCollection** result
)
{
  *result = NULL;

  HRESULT hr = S_OK;

  // save new collection in MFFontGlobals::fontCollections vector
  UINT collectionKey = MFFontGlobals::push(newCollection);
  cKeys.push_back(collectionKey);
  const void *fontCollectionKey = &cKeys.back();
  UINT32 keySize = sizeof(collectionKey);

  hr = Initialize();
  if (FAILED(hr))
    return hr;

  hr = g_dwriteFactory->CreateCustomFontCollection(
    MFFontCollectionLoader::GetLoader(),
    fontCollectionKey,
    keySize,
    result
  );

  return hr;
}

std::vector<unsigned int> MFFontContext::cKeys = std::vector<unsigned int>(0);

// ----------------------------------- MFFontGlobals ---------------------------------------------------------

std::vector<MFCollection> MFFontGlobals::fontCollections = std::vector<MFCollection>(0);