#include "pch.h"
#include "LatticeView.h"
#include "Common.h"
#include "..//Fonts//FontLoader.h"

#include <fstream>
#include <string>
#include <sstream>
#include <random>

//#define SHOWRASTERLINES

#ifdef SHOWRASTERLINES
#define MARGIN 200
#else
#define MARGIN 0
#endif

using namespace Windows::UI::Core;
using namespace Windows::Foundation::Collections;
using namespace SDKTemplate;
using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::UI::Popups;

#include <eigen/Eigen/Dense>
using  Eigen::Vector2d;
//using namespace Eigen;

#define interval1ToCartesian(x1,y1)  x1 = lattice.largeGen1(0) * generatorVec(0) +\
                                          lattice.largeGen2(0) * generatorVec(1);\
                                     y1 = lattice.largeGen1(1) * generatorVec(0) +\
                                          lattice.largeGen2(1) * generatorVec(1);
#define interval2ToCartesian(x1,y1)  x1 = lattice.largeGen1(0) * periodVec(0) +\
                                          lattice.largeGen2(0) * periodVec(1);\
                                     y1 = lattice.largeGen1(1) * periodVec(0) +\
                                          lattice.largeGen2(1) * periodVec(1);

#define keyDupToCartesian(x1,y1)     x1 = lattice.largeGen1(0) * keyDuplicateVec(0) +\
                                          lattice.largeGen2(0) * keyDuplicateVec(1);\
                                     y1 = lattice.largeGen1(1) * keyDuplicateVec(0) +\
                                          lattice.largeGen2(1) * keyDuplicateVec(1);

#define CELLBITMAPMARGIN 4
#define PATHBITMAPMARGIN 20



//#define originXToScreen -(originX + 0.5/scale)*m_d3dRenderTargetSize.Width / m_dpi * 96.0f
//#define originYToScreen -(originY + 0.5/scale)*m_d3dRenderTargetSize.Height / m_dpi * 96.0f
//
//#define screenToOriginX -originXScreen / (m_d3dRenderTargetSize.Width / m_dpi * 96.0f)-0.5/scale
//#define screenToOriginY -originYScreen / (m_d3dRenderTargetSize.Height / m_dpi * 96.0f)-0.5/scale


//#define originXToScreen -(originX + 0.5*m_d3dRenderTargetSize.Width/scale/ m_dpi * 96.0) 
//#define originYToScreen -(originY + 0.5*m_d3dRenderTargetSize.Height/scale/ m_dpi * 96.0) 
//
//#define screenToOriginX -(originXScreen + 0.5*m_d3dRenderTargetSize.Width / scale / m_dpi * 96.0 ) 
//#define screenToOriginY -(originYScreen + 0.5*m_d3dRenderTargetSize.Height / scale / m_dpi * 96.0 )


#define originXToScreen -(originX + shiftPercent*largeGen1(0) + 0.5*m_d3dRenderTargetSize.Width/scale/ m_dpi * 96.0) 
#define originYToScreen -(originY + shiftPercent*largeGen1(1) + 0.5*m_d3dRenderTargetSize.Height/scale/ m_dpi * 96.0) 

#define screenToOriginX -(originXScreen + shiftPercent*largeGen1(0) + 0.5*m_d3dRenderTargetSize.Width / scale / m_dpi * 96.0 ) 
#define screenToOriginY -(originYScreen  + shiftPercent*largeGen1(1) + 0.5*m_d3dRenderTargetSize.Height / scale / m_dpi * 96.0 )

// COnversion
// (originX' + 0.5 / scale)*m_d3dRenderTargetSize.Width =  (originX + 0.5*m_d3dRenderTargetSize.Width / scale)
//  originX'*m_d3dRenderTargetSize.Width  + 0.5 / scale*m_d3dRenderTargetSize.Width =  originX + 0.5*m_d3dRenderTargetSize.Width / scale
// originX'*m_d3dRenderTargetSize.Width =  originX

LatticeView::LatticeView() :
	m_pWhiteBrush(NULL),
	m_pBlackBrush(NULL),
	m_pBorderBrush(NULL),
	m_pCellPathGeometry(NULL),
	m_pArrowPathGeometry(NULL),
	m_pTextFormat(NULL),
	m_pGeneratorBrush(NULL),
	m_pGlowBrush(NULL),
	m_pVerticiesBrush_1(NULL),
	m_pVerticiesBrush_2(NULL),
	m_pVerticiesBrush_3(NULL),
	m_pCornerBrush(NULL),
	latticeBitMap(nullptr),
	latticeBitMapValid(false),
	instrument(),
  backgroundColor(0x79847d, 1.0f)
//backgroundColor(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f))
{
	CreateDeviceIndependentResources();
	CreateDeviceDependentResources();


	QueryPerformanceFrequency(&Freq);
	QueryPerformanceCounter(&EndingTime);


	this->temperament.criticalSection = &criticalSection;

	this->vibratoCuttoff = .5;
	//this->vibratoCuttoffFreq = 1000;
	this->vibratoAmplitude = 1;

	//this->vibrato = true;
//	this->roundFirstNote = false;
	this->roundFirstNote = true;
	this->afterTouchMode = SUSTAIN;
//	this->afterTouchMode = GLISSANDO;
//	this->afterTouchMode = BEND;


	this->vibratoTrigger = true;
	this->triggerResetRate = 2;
	this->triggerThreshold = 5;

  pitchAxis = BOTH;

	m_d3dRenderTargetSize.Width = 100;
	m_d3dRenderTargetSize.Height = 100;

	this->originX = .5;
	this->originY = .5;
	this->maxSpeed = 3;

	this->mode = TouchMode::EDIT_ROTATE_SCALE;

	this->twoTouchEdit = true;
	this->threeTouchEdit = true;

	intervalOrigin = Vector2d(0, 0);
	intervalOriginTouch = 0;

	Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
	StorageFolder^ installedLocation = package->InstalledLocation;
	StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;


  create_task(installedLocation->GetFolderAsync(L"Fonts")).then([this](StorageFolder^ folder) {
    return folder->GetFileAsync(L"Sagittal2.ttf");
  }).then([this, localFolder](StorageFile^ fontFile) {
    IDWriteFactory* pDWriteFactory = m_dwriteFactory.Get();
    MFFontContext fContext(pDWriteFactory);
    vector<std::wstring> filePaths; // vector containing ABSOLUTE file paths of the font files which are to be added to the collection
    filePaths.push_back(fontFile->Path->Data());
    DX::ThrowIfFailed(fContext.CreateFontCollection(filePaths, &customFonts), L"Error: fContext.CreateFontCollection(filePaths, &customFonts)"); // create custom font collection
    return localFolder->CreateFolderAsync("OldPreloaded", Windows::Storage::CreationCollisionOption::OpenIfExists);
  }).then([this, localFolder](StorageFolder^ folder) {
    oldPreloadedFolder = folder;
    return localFolder->CreateFolderAsync("Keyboards", Windows::Storage::CreationCollisionOption::OpenIfExists);
  }).then([this, installedLocation](StorageFolder^ folder) {
    localKeyboardFolder = folder;
    return installedLocation->GetFolderAsync(L"Keyboards");
  }).then([this, localFolder](StorageFolder^ folder) {
    preloadedKeyboardFolder = folder;
    keyboardFolder = folder;
    deleteDuplicateFiles(oldPreloadedFolder, localKeyboardFolder);
    return localFolder->CreateFolderAsync("Temperaments", Windows::Storage::CreationCollisionOption::OpenIfExists);
  }).then([this, installedLocation](StorageFolder^ folder) {
    temperament.localTemperamentFolder = folder;
    return installedLocation->GetFolderAsync(L"Temperaments");
  }).then([this, localFolder](StorageFolder^ folder) {
    temperament.preloadedTemperamentFolder = folder;
    temperament.temperamentFolder = folder;
    deleteDuplicateFiles(oldPreloadedFolder, temperament.localTemperamentFolder);
    return localFolder->CreateFolderAsync("EarTraining", Windows::Storage::CreationCollisionOption::OpenIfExists);
  }).then([this](StorageFolder^ folder) {
    localEarTrainingFolder = folder;
    return preloadedKeyboardFolder->GetFileAsync(L"!wicki-!hayden_!rotated.kb");
  }).then([this](StorageFile^ file) {
    return loadKeyboardAsync(file);
  }).then([this]()
  {
    if (dynamicTemperament)
    {
      mainPage->initializeKeyboardControls();
      mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Visible);
    }
    else
    {
      mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Collapsed);
    }
  });
}

LatticeView::~LatticeView()
{
	SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pWhiteBrush);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pBorderBrush);
	SafeRelease(&m_pCellPathGeometry);
	SafeRelease(&m_pArrowPathGeometry);
	SafeRelease(&m_pGeneratorBrush);
	SafeRelease(&m_pGlowBrush);
	SafeRelease(&m_pVerticiesBrush_1);
	SafeRelease(&m_pVerticiesBrush_2);
	SafeRelease(&m_pVerticiesBrush_3);
	SafeRelease(&m_pCornerBrush);
}


void LatticeView::CreateDeviceIndependentResources()
{
	DirectXBase::CreateDeviceIndependentResources();

//  static const WCHAR msc_fontName[] = L"poop";

  static const WCHAR msc_fontName[] = L"Verdana";
//  static const WCHAR msc_fontName[] = L"Script MT";

//  static const WCHAR msc_fontName[] = L"Sagittal";
//  static const WCHAR msc_accidentalFontName[] = L"Sagittal";
//	static const WCHAR msc_fontName[] = L"Verdana";
//  static const WCHAR msc_fontName[] = L"Arial Unicode MS";
//static const WCHAR msc_fontName[] = L"Arial";
	//static const WCHAR msc_fontName[] = L"Gothic";
	static const FLOAT msc_fontSize = 20;


	// Create a DirectWrite text format object.
    DX::ThrowIfFailed(m_dwriteFactory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"", //locale
			&m_pTextFormat
			), L"Error: m_dwriteFactory->CreateTextFormat(...)");


	// Center the text horizontally and vertically.
	DX::ThrowIfFailed(m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER),
  L"Error: m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER)");
	DX::ThrowIfFailed(m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER),
    L"m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)");
}


void LatticeView::invalidateCellPath()
{
	latticeBitMapValid = false;
	cellPathValid = false;
}

void LatticeView::invalidateBitmap()
{
  latticeBitMapValid = false;
}


void LatticeView::DrawLatticeBitmap()
{
		if (latticeBitMapValid )
    {
			return;
    }

    if (latticeBitMap == nullptr)
    {
      D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
        static_cast<UINT32>(m_d3dRenderTargetSize.Width),
        static_cast<UINT32>((m_d3dRenderTargetSize.Height))
      );

      // we don't want to keep drawing to bit maps when a resize is occuring.
      if (prevWidowSizeInPixels.width != bitmapSizeInPixels.width || prevWidowSizeInPixels.height != bitmapSizeInPixels.height)  
      {
        prevWidowSizeInPixels = bitmapSizeInPixels;
        return;
      }
      else
      {
        prevWidowSizeInPixels = bitmapSizeInPixels;
      }

      // Create the bitmap to which the effects will be applied.

      HRESULT hr = m_d2dContext->CreateBitmap(
        bitmapSizeInPixels,
        nullptr,
        0,
        D2D1::BitmapProperties1(
          D2D1_BITMAP_OPTIONS_TARGET,
          D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_PREMULTIPLIED
          ),
          m_dpi,
          m_dpi
        ),
        &latticeBitMap
      );

      if (!SUCCEEDED(hr))
      {
        latticeBitMap = nullptr;
        latticeBitMapValid = false;
        return;
      }

      PRINT(L"Created latticeBitMap\n");
    }

//		temperament.clearNoteNames();		

		DX::ThrowIfFailed(CreateCellPath(), L"Error: CreateCellPath()" );

		ComPtr<ID2D1Image> oldTarget;
		m_d2dContext->GetTarget(&oldTarget);

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);

		// Draw onto the input bitmap instead of the window's surface.
		m_d2dContext->SetTarget(latticeBitMap.Get());

		RECT bounds;
		bounds.left = static_cast<long>(MARGIN / m_dpi * 96.0f / scale);
		bounds.right = static_cast<long>((m_d3dRenderTargetSize.Width - MARGIN) / m_dpi * 96.0f / scale);
		bounds.top = static_cast<long>(MARGIN / m_dpi * 96.0f / scale);
		bounds.bottom = static_cast<long>((m_d3dRenderTargetSize.Height - MARGIN )/ m_dpi * 96.0f / scale);

		//bounds.left = MARGIN / m_dpi * 96.0f;
		//bounds.right = (m_d3dRenderTargetSize.Width - MARGIN) / m_dpi * 96.0f;
		//bounds.top = MARGIN / m_dpi * 96.0f;
		//bounds.bottom = (m_d3dRenderTargetSize.Height - MARGIN) / m_dpi * 96.0f;



		m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::SizeF(scale, scale), D2D1::Point2F(0, 0)));

		m_d2dContext->BeginDraw();
		m_d2dContext->Clear(backgroundColor);
//		try
//		{
			renderLattice(bounds);
		//}
		//catch (...)
		//{
		//	PRINT(L"crap");
		//}

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		HRESULT hr = m_d2dContext->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET)
		{
			DX::ThrowIfFailed(hr, L"Error: hr != D2DERR_RECREATE_TARGET");
		}

		m_d2dContext->SetTarget(oldTarget.Get());
		m_d2dContext->SetTransform(t);

    latticeBitMapValid = true;
}


HRESULT LatticeView::CreateCellPath()
{
	HRESULT hr = S_OK;
	if (cellPathValid)
		return hr;

  map<int, ID2D1SolidColorBrush*>::iterator iter = whiteBrushes.begin();
  for (map<int, ID2D1SolidColorBrush*>::iterator it = whiteBrushes.begin(); it != whiteBrushes.end(); ++it)
  {
    SafeRelease(&it->second);
  }

  for (map<int, ID2D1SolidColorBrush*>::iterator it = blackBrushes.begin(); it != blackBrushes.end(); ++it)
  {
    SafeRelease(&it->second);
  }

	if(m_pCellPathGeometry)
	  SafeRelease(&m_pCellPathGeometry);

	lattice.setBasis(largeGen1, largeGen2);
	lattice.calculateCellShape();

	Lattice::latticeToCartesian(&generatorVec(0), &generatorVec(1), &largeGen1, &largeGen2);
	Lattice::cartesianToLattice(&generatorVec(0), &generatorVec(1), &lattice.largeGen1, &lattice.largeGen2);
	Lattice::latticeToCartesian(&periodVec(0), &periodVec(1), &largeGen1, &largeGen2);
	Lattice::cartesianToLattice(&periodVec(0), &periodVec(1), &lattice.largeGen1, &lattice.largeGen2);

	generatorVec(0) = round(generatorVec(0));
	generatorVec(1) = round(generatorVec(1));
	periodVec(0) = round(periodVec(0));
	periodVec(1) = round(periodVec(1));
	largeGen1 = lattice.largeGen1;
	largeGen2 = lattice.largeGen2;

	hr = m_d2dFactory->CreatePathGeometry(&m_pCellPathGeometry);

	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink *pSink = NULL;
		hr = m_pCellPathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

			int numPoints = lattice.cellVerticiesDraw.size();
	//		D2D1_POINT_2F* points = new D2D1_POINT_2F[numPoints];
			pSink->BeginFigure(
				D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[0](0)), static_cast<float>(lattice.cellVerticiesDraw[0](1))),
				D2D1_FIGURE_BEGIN_FILLED
				);

			for (int i = 1; i < numPoints; i++)
			{
				pSink->AddLine(D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))));
			}

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		}

		hr = pSink->Close();
		SafeRelease(&pSink);
	}


	if (lattice.keyMode == CUSTOM)
	{
		int i = 0;
		hr = m_d2dFactory->CreatePathGeometry(&m_pCellVerticiesGeometry_1);

		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink = NULL;
			hr = m_pCellVerticiesGeometry_1->Open(&pSink);

			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				int numPoints = lattice.cellVerticiesDraw.size();
				//	D2D1_POINT_2F* points = new D2D1_POINT_2F[numPoints];
				pSink->BeginFigure(
					D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))),
					D2D1_FIGURE_BEGIN_FILLED
					);

				
				while (true)
				{
					i++;
					pSink->AddLine(D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))));

					if (lattice.cellBounderyPoints[i] == &lattice.cellVerticies1.back())
						break;
				}

				pSink->EndFigure(D2D1_FIGURE_END_OPEN);
			}

			hr = pSink->Close();
			SafeRelease(&pSink);
		}

		hr = m_d2dFactory->CreatePathGeometry(&m_pCellVerticiesGeometry_2);

		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink = NULL;
			hr = m_pCellVerticiesGeometry_2->Open(&pSink);

			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				int numPoints = lattice.cellVerticiesDraw.size();
				//	D2D1_POINT_2F* points = new D2D1_POINT_2F[numPoints];
				pSink->BeginFigure(
					D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))),
					D2D1_FIGURE_BEGIN_FILLED
					);


				while (true)
				{
					i++;
					pSink->AddLine(D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))));

					if (lattice.cellBounderyPoints[i] == &lattice.cellVerticies2.back())
						break;
				}

				pSink->EndFigure(D2D1_FIGURE_END_OPEN);
			}

			hr = pSink->Close();
			SafeRelease(&pSink);
		}

		hr = m_d2dFactory->CreatePathGeometry(&m_pCellVerticiesGeometry_3);

		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink = NULL;
			hr = m_pCellVerticiesGeometry_3->Open(&pSink);

			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				int numPoints = lattice.cellVerticiesDraw.size();
				//	D2D1_POINT_2F* points = new D2D1_POINT_2F[numPoints];
				pSink->BeginFigure(
					D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))),
					D2D1_FIGURE_BEGIN_FILLED
					);


				while (true)
				{
					i++;
					pSink->AddLine(D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))));

					if (lattice.cellBounderyPoints[i] == &lattice.cellVerticies1.back())
						break;
				}

				pSink->EndFigure(D2D1_FIGURE_END_OPEN);
			}

			hr = pSink->Close();
			SafeRelease(&pSink);
		}

		hr = m_d2dFactory->CreatePathGeometry(&m_pCellVerticiesGeometry_4);

		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink = NULL;
			hr = m_pCellVerticiesGeometry_4->Open(&pSink);

			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				int numPoints = lattice.cellVerticiesDraw.size();
				//	D2D1_POINT_2F* points = new D2D1_POINT_2F[numPoints];
				pSink->BeginFigure(
					D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))),
					D2D1_FIGURE_BEGIN_FILLED
					);


				while (true)
				{
					i++;
					pSink->AddLine(D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))));

					if (lattice.cellBounderyPoints[i] == &lattice.cellVerticies2.back())
						break;
				}

				pSink->EndFigure(D2D1_FIGURE_END_OPEN);
			}

			hr = pSink->Close();
			SafeRelease(&pSink);
		}

		hr = m_d2dFactory->CreatePathGeometry(&m_pCellVerticiesGeometry_5);

		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink = NULL;
			hr = m_pCellVerticiesGeometry_5->Open(&pSink);

			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				int numPoints = lattice.cellVerticiesDraw.size();
				//	D2D1_POINT_2F* points = new D2D1_POINT_2F[numPoints];
				pSink->BeginFigure(
					D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))),
					D2D1_FIGURE_BEGIN_FILLED
					);


				while (true)
				{
					i++;
					pSink->AddLine(D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))));

					if (lattice.cellBounderyPoints[i] == &lattice.cellVerticies1.back())
						break;
				}

				pSink->EndFigure(D2D1_FIGURE_END_OPEN);
			}

			hr = pSink->Close();
			SafeRelease(&pSink);
		}

		hr = m_d2dFactory->CreatePathGeometry(&m_pCellVerticiesGeometry_6);

		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink = NULL;
			hr = m_pCellVerticiesGeometry_6->Open(&pSink);

			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				int numPoints = lattice.cellVerticiesDraw.size();
				//	D2D1_POINT_2F* points = new D2D1_POINT_2F[numPoints];
				pSink->BeginFigure(
					D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))),
					D2D1_FIGURE_BEGIN_FILLED
					);


				while (true)
				{
					i++;
					if (i < lattice.cellVerticiesDraw.size())
					{
						pSink->AddLine(D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[i](0)), static_cast<float>(lattice.cellVerticiesDraw[i](1))));
					}
					else
					{
						pSink->AddLine(D2D1::Point2F(static_cast<float>(lattice.cellVerticiesDraw[0](0)), static_cast<float>(lattice.cellVerticiesDraw[0](1))));
						break;
					}
				}

				pSink->EndFigure(D2D1_FIGURE_END_OPEN);
			}

			hr = pSink->Close();
			SafeRelease(&pSink);
		}
	}



	ComPtr<ID2D1Image> oldTarget;
	m_d2dContext->GetTarget(&oldTarget);

	float cellBitMapWidth = lattice.cellWidth + 2 * CELLBITMAPMARGIN;
	float cellBitMapHeight = lattice.cellHeight + 2 * CELLBITMAPMARGIN;
	///////////////////////////////////////////////////////////////////////////////////////
	// create white cell bitmap
	//{
	//	// Convert from DIPs to pixels, since bitmaps are created in units of pixels.
	//	D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
	//		static_cast<UINT32>(cellBitMapWidth / 96.0f * m_dpi),
	//		static_cast<UINT32>(cellBitMapHeight / 96.0f * m_dpi)
	//		);
	//	// Create the bitmap to which the effects will be applied.
	//	DX::ThrowIfFailed(
	//		m_d2dContext->CreateBitmap(
	//			bitmapSizeInPixels,
	//			nullptr,
	//			0,
	//			D2D1::BitmapProperties1(
	//				D2D1_BITMAP_OPTIONS_TARGET,
	//				D2D1::PixelFormat(
	//					DXGI_FORMAT_B8G8R8A8_UNORM,
	//					D2D1_ALPHA_MODE_PREMULTIPLIED
	//					),
	//				m_dpi,
	//				m_dpi
	//				),
	//			&whiteCellBitMap
	//			)
	//		);

	//	// Draw onto the input bitmap instead of the window's surface.
	//	m_d2dContext->SetTarget(whiteCellBitMap.Get());

	//	m_d2dContext->BeginDraw();

	//	// Clear the bitmap with transparent white.
	//	m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));

	//	D2D1_MATRIX_3X2_F t;
	//	m_d2dContext->GetTransform(&t);
	//	D2D1_MATRIX_3X2_F t2 = t*D2D1::Matrix3x2F::Translation(-lattice.cellMinX + CELLBITMAPMARGIN, -lattice.cellMinY + CELLBITMAPMARGIN);
	//	m_d2dContext->SetTransform(t2);
	//	m_d2dContext->FillGeometry(m_pCellPathGeometry, m_pWhiteBrush);
	//	m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pBorderBrush, 2.5);
	//	m_d2dContext->SetTransform(t);

	//	// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	//	// is lost. It will be handled during the next call to Present.
	//	hr = m_d2dContext->EndDraw();
	//	if (hr != D2DERR_RECREATE_TARGET)
	//	{
	//		DX::ThrowIfFailed(hr);
	//	}
	//}
	///////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	//// create black cell bitmap
	//{
	//	// Convert from DIPs to pixels, since bitmaps are created in units of pixels.
	//	D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
	//		static_cast<UINT32>(cellBitMapWidth / 96.0f * m_dpi),
	//		static_cast<UINT32>(cellBitMapHeight / 96.0f * m_dpi)
	//		);

	//	// Create the bitmap to which the effects will be applied.
	//	DX::ThrowIfFailed(
	//		m_d2dContext->CreateBitmap(
	//			bitmapSizeInPixels,
	//			nullptr,
	//			0,
	//			D2D1::BitmapProperties1(
	//				D2D1_BITMAP_OPTIONS_TARGET,
	//				D2D1::PixelFormat(
	//					DXGI_FORMAT_B8G8R8A8_UNORM,
	//					D2D1_ALPHA_MODE_PREMULTIPLIED
	//					),
	//				m_dpi,
	//				m_dpi
	//				),
	//			&blackCellBitMap
	//			)
	//		);

	//	// Draw onto the input bitmap instead of the window's surface.
	//	m_d2dContext->SetTarget(blackCellBitMap.Get());

	//	m_d2dContext->BeginDraw();

	//	// Clear the bitmap with transparent white.
	//	m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));

	//	D2D1_MATRIX_3X2_F t;
	//	m_d2dContext->GetTransform(&t);
	//	D2D1_MATRIX_3X2_F t2 = t*D2D1::Matrix3x2F::Translation(-lattice.cellMinX + CELLBITMAPMARGIN, -lattice.cellMinY + CELLBITMAPMARGIN);
	//	m_d2dContext->SetTransform(t2);
	//	m_d2dContext->FillGeometry(m_pCellPathGeometry, m_pBlackBrush);
	//	m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pBorderBrush, 2.5);
	//	m_d2dContext->SetTransform(t);

	//	// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	//	// is lost. It will be handled during the next call to Present.
	//	hr = m_d2dContext->EndDraw();
	//	if (hr != D2DERR_RECREATE_TARGET)
	//	{
	//		DX::ThrowIfFailed(hr);
	//	}
	//}

	///////////////////////////////////////////////////////////////////////////////////////
	//// Create Glowing for touched cells
	{
		float pathBitMapWidth = lattice.cellWidth + PATHBITMAPMARGIN;
		float pathBitMapHeight = lattice.cellHeight + PATHBITMAPMARGIN;

		// Convert from DIPs to pixels, since bitmaps are created in units of pixels.
		D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
			static_cast<UINT32>(pathBitMapWidth / 96.0f * m_dpi),
			static_cast<UINT32>(pathBitMapHeight / 96.0f * m_dpi)
			);

		// Create the bitmap to which the effects will be applied.
		DX::ThrowIfFailed(
			m_d2dContext->CreateBitmap(
				bitmapSizeInPixels,
				nullptr,
				0,
				D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET,
					D2D1::PixelFormat(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						D2D1_ALPHA_MODE_PREMULTIPLIED
						),
					m_dpi,
					m_dpi
					),
				&pathBitMap
				),
        L"Error: m_d2dContext->CreateBitmap(pathBitMap)"
			);

		// Draw onto the input bitmap instead of the window's surface.
		m_d2dContext->SetTarget(pathBitMap.Get());

		m_d2dContext->BeginDraw();

		// Clear the bitmap with transparent white.
		m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);
		D2D1_MATRIX_3X2_F t2 = t*D2D1::Matrix3x2F::Translation(-lattice.cellMinX + PATHBITMAPMARGIN, -lattice.cellMinY + PATHBITMAPMARGIN);
		m_d2dContext->SetTransform(t2);
		m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pGlowBrush, 10);
		m_d2dContext->SetTransform(t);

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		hr = m_d2dContext->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET)
		{
			DX::ThrowIfFailed(hr, L"Error: m_d2dContext->EndDraw()");
		}
	}

	//Create Glowing effect for touch points
	{
		touchBitMapWidth = touchDiameter + 20;
		touchBitMapHeight = touchDiameter + 20;

		// Convert from DIPs to pixels, since bitmaps are created in units of pixels.
		D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
			static_cast<UINT32>(touchBitMapWidth / 96.0f * m_dpi),
			static_cast<UINT32>(touchBitMapHeight / 96.0f * m_dpi)
			);

		// Create the bitmap to which the effects will be applied.
		DX::ThrowIfFailed(
			m_d2dContext->CreateBitmap(
				bitmapSizeInPixels,
				nullptr,
				0,
				D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET,
					D2D1::PixelFormat(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						D2D1_ALPHA_MODE_PREMULTIPLIED
						),
					m_dpi,
					m_dpi
					),
				&touchBitMap
				), L"Error: m_d2dContext->CreateBitmap(touchBitMap)"
			);

		// Draw onto the input bitmap instead of the window's surface.
		m_d2dContext->SetTarget(touchBitMap.Get());

		m_d2dContext->BeginDraw();

		// Clear the bitmap with transparent white.
		m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));

		D2D1_ELLIPSE ellipse = D2D1::Ellipse(
			D2D1::Point2F(touchBitMapWidth / 2, touchBitMapHeight / 2),
			touchDiameter/2,
			touchDiameter/2
			);
		m_d2dContext->FillEllipse(&ellipse, m_pGlowBrush);

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		hr = m_d2dContext->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET)
		{
			DX::ThrowIfFailed(hr, L"Error: m_d2dContext->EndDraw() #2");
		}
	}

	m_d2dContext->SetTarget(oldTarget.Get());

	float blurAmount = 6.0f;
	DX::ThrowIfFailed(m_d2dContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pathBlurEffect),
   L"m_d2dContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pathBlurEffect)"	);
	DX::ThrowIfFailed(m_pathBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, blurAmount),
    L"m_pathBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, blurAmount)");
	m_pathBlurEffect->SetInput(0, pathBitMap.Get());

	DX::ThrowIfFailed(m_d2dContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_touchBlurEffect),
    L"m_d2dContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_touchBlurEffect)");
	DX::ThrowIfFailed(m_touchBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 10.0f),
    L"m_touchBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 10.0f)");
	m_touchBlurEffect->SetInput(0, touchBitMap.Get());
	
	cellPathValid = true;
	return hr;
}

#define ARROWSIZE 10

HRESULT LatticeView::CreateArrowHead()
{
	HRESULT hr = S_OK;
	if (m_pArrowPathGeometry)
		return hr;

	hr = m_d2dFactory->CreatePathGeometry(&m_pArrowPathGeometry);

	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink *pSink = NULL;
		hr = m_pArrowPathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

			double shiftY = -1.5;

			D2D1_POINT_2F* points = new D2D1_POINT_2F[3];
			pSink->BeginFigure(
				D2D1::Point2F(ARROWSIZE, shiftY*ARROWSIZE),
				D2D1_FIGURE_BEGIN_FILLED
				);

			pSink->AddBezier(
				D2D1::BezierSegment(
					D2D1::Point2F(ARROWSIZE*.5, shiftY*ARROWSIZE+ ARROWSIZE*.5),
					D2D1::Point2F(-ARROWSIZE*.5, shiftY*ARROWSIZE+ ARROWSIZE*.5),
					D2D1::Point2F(-ARROWSIZE, shiftY*ARROWSIZE)
					));

			pSink->AddLine(D2D1::Point2F(0, shiftY*ARROWSIZE + 2* ARROWSIZE));
			pSink->AddLine(D2D1::Point2F(ARROWSIZE, shiftY*ARROWSIZE));

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		}

		hr = pSink->Close();
		SafeRelease(&pSink);
	}
	return hr;
}


void LatticeView::CreateDeviceDependentResources()
{
	DirectXBase::CreateDeviceDependentResources();

	DX::ThrowIfFailed(CreateArrowHead(), L"Error: CreateArrowHead()");

	// Create some brushes.
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pWhiteBrush),
   L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pWhiteBrush)");
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),&m_pBlackBrush),
    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pBlackBrush)");
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(backgroundColor,	&m_pBorderBrush),
    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pBorderBrush)");
//	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0), &m_pBorderBrush));
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pGeneratorBrush),
    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pGeneratorBrush)");
//	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &m_pGlowBrush),
//    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pGlowBrush)");

  DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pGlowBrush),
    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pGlowBrush)");

	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_pVerticiesBrush_1),
    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pVerticiesBrush_1)");
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &m_pVerticiesBrush_2),
    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pVerticiesBrush_2)");
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pVerticiesBrush_3),
    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pVerticiesBrush_3)");
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Magenta), &m_pCornerBrush),
    L"Error: m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pCornerBrush)");
}

void LatticeView::ReleaseDeviceDependentResources()
{
	SafeRelease(&m_pWhiteBrush);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pBorderBrush);
}

void LatticeView::CreateWindowSizeDependentResources()
{
	DirectXBase::CreateWindowSizeDependentResources();
  latticeBitMap = nullptr;
  invalidateCellPath();
}

void LatticeView::clearTouchData()
{
  for(int i = 0; i < touchDataArray.size(); i++)
    touchDataArray[i].clear();
}

TouchData* LatticeView::newTouchData(unsigned int dwID)
{
	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* td = &touchDataArray[i];
		if (td->dwID == dwID)
			return td;
	}

	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* td = &touchDataArray[i];
		if (td->phase == NOTHING)
		{
			td->dwID = dwID;
			return td;
		}
	}

	touchDataArray.push_back(TouchData());
	touchDataArray[touchDataArray.size() - 1].dwID = dwID;
	return &touchDataArray[touchDataArray.size() - 1];
}

TouchData* LatticeView::getTouchData(unsigned int dwID)
{
	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* td = &touchDataArray[i];
		if (td->dwID == dwID)
			return td;
	}

	return 0;
}

void LatticeView::Render()
{
  if(!initialized)
    return;

	HRESULT hr = S_OK;

	if (playMusic && !dragKeyboard)
	{
		DrawLatticeBitmap();
	}

  if (latticeBitMapValid)
  {
    m_d2dContext->BeginDraw();
    m_d2dContext->DrawBitmap(latticeBitMap.Get());
  }
	else  // This previously causes a crash on secon keyboard reload for some reason. the issues seems is the (intersects.y1 != intersects.y2) in render rows. THis throws an eception on line 808. Seemes like this is fixed now.
	{
		DX::ThrowIfFailed(CreateCellPath(), L"Error: CreateCellPath() #2");

		RECT bounds;
		bounds.left = static_cast<long>(MARGIN / m_dpi * 96.0f / scale);
		bounds.right = static_cast<long>((m_d3dRenderTargetSize.Width - MARGIN) / m_dpi * 96.0f / scale);
		bounds.top = static_cast<long>(MARGIN / m_dpi * 96.0f / scale);
		bounds.bottom = static_cast<long>((m_d3dRenderTargetSize.Height - MARGIN) / m_dpi * 96.0f / scale);

		//bounds.left = MARGIN / m_dpi * 96.0f;
		//bounds.right = (m_d3dRenderTargetSize.Width - MARGIN) / m_dpi * 96.0f;
		//bounds.top = MARGIN / m_dpi * 96.0f;
		//bounds.bottom = (m_d3dRenderTargetSize.Height - MARGIN) / m_dpi * 96.0f;

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);
		m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::SizeF(scale, scale), D2D1::Point2F(0, 0)));

		m_d2dContext->BeginDraw();
		m_d2dContext->Clear(backgroundColor);

		hr = renderLattice(bounds);
		if (!SUCCEEDED(hr))
		{
			PRINT(L"SOMETHING WENT TERRIBLY WRONG!!!!!\n");
			m_d2dContext->EndDraw();
			return;
		}

		m_d2dContext->SetTransform(t);
	}
	

	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::SizeF(scale, scale), D2D1::Point2F(0, 0)));

	if (playMusic || oneTouchEdit)
	{
		for (int i = 0; i < touchDataArray.size(); i++)
		{
			TouchData* td = &touchDataArray[i];
			//if (td->noteID != -1)
			if ( !playMusic || (td->phase != NOTHING
        && (playUnamedNotes || td->name && td->name->name.size() > 0) 
        && (td->midiNote >= 0 && td->midiNote <= 127)))
			{

        double x = td->cellPos(0) + lattice.cellMinX - PATHBITMAPMARGIN;
        double y = td->cellPos(1) + lattice.cellMinY - PATHBITMAPMARGIN;

        x += td->deltaPitchPos(0);
        y += td->deltaPitchPos(1);

        if(playMusic)
					m_d2dContext->DrawImage(m_pathBlurEffect.Get(), D2D1::Point2F(x,y));
	 
   			m_d2dContext->DrawImage(m_touchBlurEffect.Get(), D2D1::Point2F(td->pos(0) - touchBitMapWidth / 2, td->pos(1) - touchBitMapHeight / 2));
			}
		}
	}
		
  if(!playMusic)
	{
		renderArrows();

		if (lattice.keyMode == CUSTOM)
		{
			double x = 0;
			double y = 0;
			lattice.latticeToCartesian(&x, &y);
			x -= originXDraw;
			y -= originYDraw;

      double cellX = temperament.rootGens;
      double cellY = temperament.rootPers;
      Vector2d genVec = generatorVec;
      Vector2d perVec = periodVec;
      Lattice::latticeToCartesian(genVec, &lattice.largeGen1, &lattice.largeGen2);
      Lattice::latticeToCartesian(perVec, &lattice.largeGen1, &lattice.largeGen2);
      Lattice::latticeToCartesian(&cellX, &cellY, &genVec, &perVec);

      x+= cellX;
      y+= cellY;
			D2D1_MATRIX_3X2_F t;
			m_d2dContext->GetTransform(&t);
			D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Translation(x, y)*t;
			m_d2dContext->SetTransform(t2);

			m_d2dContext->DrawImage(m_pathBlurEffect.Get(), D2D1::Point2F(lattice.cellMinX - PATHBITMAPMARGIN, lattice.cellMinY - PATHBITMAPMARGIN));

			m_d2dContext->DrawGeometry(m_pCellVerticiesGeometry_1, m_pVerticiesBrush_1, 3);
			m_d2dContext->DrawGeometry(m_pCellVerticiesGeometry_2, m_pVerticiesBrush_2, 3);
			m_d2dContext->DrawGeometry(m_pCellVerticiesGeometry_3, m_pVerticiesBrush_3, 3);
			m_d2dContext->DrawGeometry(m_pCellVerticiesGeometry_4, m_pVerticiesBrush_1, 3);
			m_d2dContext->DrawGeometry(m_pCellVerticiesGeometry_5, m_pVerticiesBrush_2, 3);
			m_d2dContext->DrawGeometry(m_pCellVerticiesGeometry_6, m_pVerticiesBrush_3, 3);
			m_d2dContext->SetTransform(t);


			ID2D1SolidColorBrush* brush = m_pVerticiesBrush_3;
			for (int i = 0; i < lattice.cellVerticies.size(); i++)
			{
				if (lattice.cellBounderyPoints[i] == &lattice.cellVerticies1.back() )
				{
					D2D1_ELLIPSE dot = D2D1::Ellipse(
						D2D1::Point2F(x + lattice.cellVerticies[i](0), y + lattice.cellVerticies[i](1)),
						6.f,
						6.f
						);
					m_d2dContext->DrawEllipse(dot, m_pCornerBrush, 3);

					if (brush == m_pVerticiesBrush_1)
						brush = m_pVerticiesBrush_2;
					else if(brush == m_pVerticiesBrush_2)
						brush = m_pVerticiesBrush_3;
					else
						brush = m_pVerticiesBrush_1;
				}
				else if (lattice.cellBounderyPoints[i] == &lattice.cellVerticies2.back())
				{
					D2D1_RECT_F rect = D2D1::RectF( x + lattice.cellVerticies[i](0) -6,
							                          y + lattice.cellVerticies[i](1) -6,
													x + lattice.cellVerticies[i](0) +6,
													y + lattice.cellVerticies[i](1) +6);
					m_d2dContext->DrawRectangle(rect, m_pCornerBrush, 3);

					if (brush == m_pVerticiesBrush_1)
						brush = m_pVerticiesBrush_2;
					else if (brush == m_pVerticiesBrush_2)
						brush = m_pVerticiesBrush_3;
					else
						brush = m_pVerticiesBrush_1;
				}
				else
				{
					D2D1_ELLIPSE dot = D2D1::Ellipse(
						D2D1::Point2F(x + lattice.cellVerticies[i](0), y + lattice.cellVerticies[i](1)),
						5.f,
						5.f
						);
					m_d2dContext->FillEllipse(dot, brush);
				}
			}

			for (int i = 0; i < lattice.textPos.size(); i++)
			{
				D2D1_ELLIPSE dot = D2D1::Ellipse(
					D2D1::Point2F(x + lattice.textPos[i](0)*lattice.largeGen2(0) + lattice.textPos[i](1)* lattice.largeGen1(0), y + lattice.textPos[i](0)*lattice.largeGen2(1) + lattice.textPos[i](1)* lattice.largeGen1(1)),
					5.f,
					5.f
					);
				m_d2dContext->FillEllipse(dot, m_pGlowBrush);
			}
		}
			
	
	}

  m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

  if(questions>0)
  { 
    wchar_t str[256];
    swprintf_s(str, 256, L"%d/%d\nmidi program = %s", correct, questions, instrument.programs[instrument.midi_program].c_str());

    IDWriteTextLayout* ptr;
    hr = m_dwriteFactory->CreateTextLayout(
      str,
      wcslen(str),
      //		name->name.c_str(),
      //		name->name.length(),

      m_pTextFormat,  // The text format to apply to the string (contains font information, etc).
                      //		lattice.cellWidth,         // The width of the layout box.
                      //		lattice.cellHeight,        // The height of the layout box.
      400,         // The width of the layout box.
      50,        // The height of the layout box.
      &ptr // The IDWriteTextLayout interface pointer.
    );

   

    m_d2dContext->DrawTextLayout(D2D1::Point2F(0, mainPage->getRibbonHeight()+ptr->GetMaxHeight() / 2), ptr, m_pVerticiesBrush_3);
  //  m_d2dContext->DrawTextLayout(D2D1::Point2F(0, ptr->GetMaxHeight() / 2), ptr, m_pVerticiesBrush_3);

    ptr->Release();
  }




  //D2D1_ELLIPSE dot = D2D1::Ellipse(
  //    D2D1::Point2F(originXToScreen, originYToScreen),
  //    5.f,
  //    5.f
  //  );
  //m_d2dContext->FillEllipse(dot, m_pGlowBrush);


	//Call the render target's EndDraw method.
	//The EndDraw method returns an HRESULT to indicate whether the drawing operations were successful.
	hr = m_d2dContext->EndDraw();


	// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	// is lost. It will be handled during the next call to Present.
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr, L"hr != D2DERR_RECREATE_TARGET");
		hr = S_OK;
	}
}

void LatticeView::OnResize(UINT width, UINT height)
{
	//if (m_d2dContext)
	//{
	//	// Note: This method can fail, but it's okay to ignore the
	//	// error here, because the error will be returned again
	//	// the next time EndDraw is called.
	//	m_d2dContext->Resize(D2D1::SizeU(width, height));
	//}

//  PRINT(L"m_d3dRenderTargetSize = (%f, %f)\n", m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height);
//  PRINT(L"new size = (%d, %df)\n\n", width, height);

	//UpdateForWindowSizeChange();
}


HRESULT LatticeView::renderLattice(const RECT& invalidRect)
{
	HRESULT hr = S_OK;

	originXDraw = originXToScreen;
	originYDraw = originYToScreen;


	RECT bounds;
	//bounds.left = static_cast<int>(invalidRect.left + originXDraw - lattice.cellWidth / 2);
	//bounds.right = static_cast<int>(invalidRect.right + originXDraw + lattice.cellWidth / 2) + 1;
	//bounds.top = static_cast<int>(invalidRect.top + originYDraw - lattice.cellHeight / 2);
	//bounds.bottom = static_cast<int>(invalidRect.bottom + originYDraw + lattice.cellHeight / 2) + 1;

	bounds.left = static_cast<int>(invalidRect.left + originXDraw - lattice.cellMaxX);
	bounds.right = static_cast<int>(invalidRect.right + originXDraw - lattice.cellMinX) + 1;
	bounds.top = static_cast<int>(invalidRect.top + originYDraw - lattice.cellMaxY);
	bounds.bottom = static_cast<int>(invalidRect.bottom + originYDraw - lattice.cellMinY) + 1;

	double x = (bounds.left + bounds.right) / 2;
	double y = (bounds.top + bounds.bottom) / 2;

	lattice.getCell(&x, &y);
	lattice.latticeToCartesian(&x, &y);

//	lattice.getClosestLatticePoint(&x, &y);
//	lattice.latticeToCartesian(&x, &y);

	double m = lattice.gen1(1) / lattice.gen1(0);

	if (m > 1 || m < -1)
		//if(m == INFINITY || m == -INFINITY)
	{
		Vector2d temp = lattice.gen2;
		lattice.gen2 = lattice.gen1;
		lattice.gen1 = temp;

		m = lattice.gen1(1) / lattice.gen1(0);
	}

	double b = y - m * x;
	double bInit = b;
	hr = renderRows(lattice.gen2(1) - m * lattice.gen2(0), bounds, m, b);

	if (!SUCCEEDED(hr))
	  return hr;

	b = bInit - lattice.gen2(1) + m * lattice.gen2(0);
	hr = renderRows(-lattice.gen2(1) + m * lattice.gen2(0), bounds, m, b);
	
	if (!SUCCEEDED(hr))
		return hr;

#ifdef SHOWRASTERLINES
	hr = renderRasterLines(lattice.gen2(1) - m * lattice.gen2(0), bounds, m, bInit);

	if (!SUCCEEDED(hr))
		return hr;

	b = bInit - lattice.gen2(1) + m * lattice.gen2(0);
	hr = renderRasterLines(-lattice.gen2(1) + m * lattice.gen2(0), bounds, m, b);

	D2D1_RECT_F boundsRect = D2D1::RectF(bounds.left - originXDraw, bounds.top - originYDraw, bounds.right- originXDraw, bounds.bottom - originYDraw);
	m_d2dContext->DrawRectangle(&boundsRect, m_pGeneratorBrush, 4);
	boundsRect = D2D1::RectF(invalidRect.left, invalidRect.top, invalidRect.right, invalidRect.bottom);
	m_d2dContext->DrawRectangle(&boundsRect, m_pGlowBrush, 3);

	renderSmallGens();
#endif
	return hr;
}

HRESULT LatticeView::renderRows(double bInc, const RECT& bounds, const double& m,  double& b)
{
	HRESULT hr = S_OK;
	Intersects intersects;
	this->calculateLineRectangleIntersection(bounds, intersects, m ,b);

	while (intersects.numberIntersections == 2)
	{
		lattice.getClosestLatticePoint(&intersects.x1, &intersects.y1);
		lattice.getClosestLatticePoint(&intersects.x2, &intersects.y2);

		// xIntersection and its freinds now contain the latice coordinates
		// of the cells where the raster line intersects the oversized view rectangle.

		if (intersects.y1 != intersects.y2)
		{
			DebugThrow(L"Invalide state in render Rows: yIntersect1 != yIntersect2 : yIntersect1 = %f, yIntersect2 = %f", intersects.y1, intersects.y2);
		}

		double yFirst, yLast;
		if (intersects.y1 <= intersects.y2)
		{
			yFirst = intersects.y1;
			yLast = intersects.y2;
		}
	    else
		{
			yFirst = intersects.y2;
			yLast = intersects.y1;
		}

		for (double yCell = yFirst; yCell <= yLast; yCell++)
		{
			int i = static_cast<int>(intersects.x1);
			int last = static_cast<int>(intersects.x2);

			if (last < i)
			{
				int temp = last;
				last = i;
				i = temp;
			}

			double x = i;
		    double y = yCell;
			lattice.latticeToCartesian(&x, &y);

			/////////////////////////////////////////
			// Trim off some cells if they don't need to be drawn.
			// With this optimization, I beleve that there are at most 4 cells
			// extra which would ever be drawn, unnesisaraly because they don't intersect the clip
			// bounds. These 4 cells are at the 4 corners.
			// It is proboby not worth trying to optomize these away as the calculations
			// would be more involved.
			double x2 = x + (last - i)*lattice.gen1(0);
			double y2 = y + (last - i)*lattice.gen1(1);
			if (x2 < bounds.left || x2 > bounds.right || y2 < bounds.top || y2 > bounds.bottom)
			{
				last--;
			}

			if (x < bounds.left || x > bounds.right || y < bounds.top || y > bounds.bottom)
			{
				i++;
				x += lattice.gen1(0);
				y += lattice.gen1(1);
			}
			/////////////////////////////////////////
			
			x -= originXDraw;
			y -= originYDraw;


			while (i <= last)
			{
				double cents = getCents(x + originXDraw, y + originYDraw);
				int midiNote = instrument.getMidiNoteFromFreq(temperament.frequencyFromCents(cents));    // MIDI note-on message: Key number (60 = middle C) 69 = A4
				if (midiNote >= 0 && midiNote <= 127)  //only draw playable cells
				{
					hr = drawCell(x, y);
					if (!SUCCEEDED(hr))
						return hr;
				}

				i++;
				x += lattice.gen1(0);
				y += lattice.gen1(1);
			}

	    }

		b += bInc; // this shouild not be nessisary
	    this->calculateLineRectangleIntersection(bounds, intersects, m, b);
	}

	return hr;
}

HRESULT LatticeView::renderRasterLines(double bInc, const RECT& bounds, const double& m, double& b)
{
	HRESULT hr = S_OK;
	Intersects intersects;
	this->calculateLineRectangleIntersection(bounds, intersects, m, b);

	while (intersects.numberIntersections == 2)
	{
		Intersects rasterLine = intersects; // for drawing raster lines

		lattice.getClosestLatticePoint(&intersects.x1, &intersects.y1);
		lattice.getClosestLatticePoint(&intersects.x2, &intersects.y2);

		// xIntersection and its freinds now contain the latice coordinates
		// of the cells where the raster line intersects the oversized view rectangle.

		if (intersects.y1 != intersects.y2)
		{
			DebugThrow(L"Invalide state in renderRasterLines : yIntersect1 != yIntersect2 : yIntersect1 = %f, yIntersect2 = %f", intersects.y1, intersects.y2);
		}

		double yFirst, yLast;
		if (intersects.y1 <= intersects.y2)
		{
			yFirst = intersects.y1;
			yLast = intersects.y2;
		}
		else
		{
			yFirst = intersects.y2;
			yLast = intersects.y1;
		}

		for (double yCell = yFirst; yCell <= yLast; yCell++)
		{
			int i = static_cast<int>(intersects.x1);
			int last = static_cast<int>(intersects.x2);

			if (last < i)
			{
				int temp = last;
				last = i;
				i = temp;
			}

			double x = i;
			double y = yCell;
			lattice.latticeToCartesian(&x, &y);

			/////////////////////////////////////////
			// Trim off some cells if they don't need to be drawn.
			// With this optimization, I beleve that there are at most 4 cells
			// extra which would ever be drawn, unnesisaraly because they don't intersect the clip
			// bounds. These 4 cells are at the 4 corners.
			// It is proboby not worth trying to optomize these away as the calculations
			// would be more involved.
			double x2 = x + (last - i)*lattice.gen1(0);
			double y2 = y + (last - i)*lattice.gen1(1);
			if (x2 < bounds.left || x2 > bounds.right || y2 < bounds.top || y2 > bounds.bottom)
			{
				last--;
			}

			if (x < bounds.left || x > bounds.right || y < bounds.top || y > bounds.bottom)
			{
				i++;
				x += lattice.gen1(0);
				y += lattice.gen1(1);
			}
			/////////////////////////////////////////
			x -= originXDraw;
			y -= originYDraw;

			while (i <= last)
			{
				m_d2dContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 7, 7), m_pGeneratorBrush);

				i++;
				x += lattice.gen1(0);
				y += lattice.gen1(1);
			}

		}

		if (intersects.y1 == intersects.y2)
			m_d2dContext->DrawLine(D2D1::Point2F(rasterLine.x1 - originXDraw, rasterLine.y1 - originYDraw), D2D1::Point2F(rasterLine.x2 - originXDraw, rasterLine.y2 - originYDraw), m_pGeneratorBrush, 4, NULL);
		else
			m_d2dContext->DrawLine(D2D1::Point2F(rasterLine.x1 - originXDraw, rasterLine.y1 - originYDraw), D2D1::Point2F(rasterLine.x2 - originXDraw, rasterLine.y2 - originYDraw), m_pGlowBrush, 4, NULL);

		b += bInc; // this shouild not be nessisary
		this->calculateLineRectangleIntersection(bounds, intersects, m, b);
	}

	return hr;
}



void LatticeView::calculateLineRectangleIntersection(const RECT& bounds, Intersects& intersects, const double& m, const double& b)
{
	double* x = &intersects.x1;
	double* y = &intersects.y1;
	int& numberIntersections = intersects.numberIntersections;

	numberIntersections = 0;

	*x = bounds.left;
	*y = m * *x + b;

	if (*y >= bounds.top && *y <= bounds.bottom)
	{
		x = &intersects.x2;
		y = &intersects.y2;
		numberIntersections++;
	}

	*x = bounds.right;
	*y = m * *x + b;

	if (*y >= bounds.top && *y <= bounds.bottom)
	{
		numberIntersections++;
		if (numberIntersections == 2)
		{
			return;
		}
		else
		{
			x = &intersects.x2;
			y = &intersects.y2;
		}
	}

	*y = bounds.top;
	*x = (*y - b) / m;

	// don't use >= and <= here becauce we don't want to get more than two points.
	if (*x > bounds.left && *x < bounds.right)
	{
		numberIntersections++;
		if (numberIntersections == 2)
		{
			return;
		}
		else
		{
			x = &intersects.x2;
			y = &intersects.y2;
		}
	}

	*y = bounds.bottom;
	*x = (*y - b) / m;

	// don't use >= and <= here becauce we don't want to get more than two points.
	if (*x > bounds.left && *x < bounds.right)
	{
		numberIntersections++;
	}
}

void LatticeView::renderSmallGens()
{
	double x = 0;
	double y = 0;
	Lattice::latticeToCartesian(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
	x -= originXDraw;
	y -= originYDraw;

	{
		double xx = lattice.gen1(0);
		double yy = lattice.gen1(1);

		m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGlowBrush, 4, NULL);

		double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
		angle += yy < 0 ? 180 : 0;  //Corect for phase

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);
		D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
		m_d2dContext->SetTransform(t2);
		m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGlowBrush);
		m_d2dContext->SetTransform(t);
	}

	{
		double xx = lattice.gen2(0);
		double yy = lattice.gen2(1);

		m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGlowBrush, 4, NULL);

		double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
		angle += yy < 0 ? 180 : 0;  //Corect for phase

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);
		D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
		m_d2dContext->SetTransform(t2);
		m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGlowBrush);
		m_d2dContext->SetTransform(t);

		m_d2dContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 5, 5), m_pGlowBrush);
	}
}

void LatticeView::renderArrows()
{
	if (intervalOriginTouch)
	{
		double x = intervalOrigin(0);
		double y = intervalOrigin(1);
		Lattice::latticeToCartesian(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
		x -= originXDraw;
		y -= originYDraw;

		boolean drawArrow = true;
		for (unsigned int i = 0; i < touchDataArray.size(); i++)
		{
			if (touchDataArray[i].intervalVector == &generatorVec)
				drawArrow = false;
		}

		if (drawArrow) //we don'w want to draw the arrow if it is being draged.
		{
			double xx = lattice.largeGen1(0) * generatorVec(0) +
				lattice.largeGen2(0) * generatorVec(1);
			double yy = lattice.largeGen1(1) * generatorVec(0) +
				lattice.largeGen2(1) * generatorVec(1);

			m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGeneratorBrush, 4, NULL);

			double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
			angle += yy < 0 ? 180 : 0;  //Corect for phase

			D2D1_MATRIX_3X2_F t;
			m_d2dContext->GetTransform(&t);
			D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
			m_d2dContext->SetTransform(t2);
			m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGeneratorBrush);
			m_d2dContext->SetTransform(t);
		}

		drawArrow = true;
		for (unsigned int i = 0; i < touchDataArray.size(); i++)
		{
			if (touchDataArray[i].intervalVector == &periodVec)
				drawArrow = false;
		}

		if (drawArrow)  //we don'w want to draw the arrow if it is being draged.
		{
			double xx = lattice.largeGen1(0) * periodVec(0) +
				lattice.largeGen2(0) * periodVec(1);
			double yy = lattice.largeGen1(1) * periodVec(0) +
				lattice.largeGen2(1) * periodVec(1);

			m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGeneratorBrush, 4, NULL);

			double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
			angle += yy < 0 ? 180 : 0;  //Corect for phase

			D2D1_MATRIX_3X2_F t;
			m_d2dContext->GetTransform(&t);
			D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
			m_d2dContext->SetTransform(t2);
			m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGeneratorBrush);
			m_d2dContext->SetTransform(t);

			m_d2dContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 5, 5), m_pGeneratorBrush);
		}


		if (dupKeys)
		{
			drawArrow = true;
			for (unsigned int i = 0; i < touchDataArray.size(); i++)
			{
				if (touchDataArray[i].intervalVector == &keyDuplicateVec)
					drawArrow = false;
			}

			if (drawArrow)  //we don'w want to draw the arrow if it is being draged.
			{
				double xx = lattice.largeGen1(0) * keyDuplicateVec(0) +
					lattice.largeGen2(0) * keyDuplicateVec(1);
				double yy = lattice.largeGen1(1) * keyDuplicateVec(0) +
					lattice.largeGen2(1) * keyDuplicateVec(1);

				m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGlowBrush, 4, NULL);

				double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
				angle += yy < 0 ? 180 : 0;  //Corect for phase

				D2D1_MATRIX_3X2_F t;
				m_d2dContext->GetTransform(&t);
				D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
				m_d2dContext->SetTransform(t2);
				m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGlowBrush);
				m_d2dContext->SetTransform(t);

				m_d2dContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 5, 5), m_pGlowBrush);
			}
		}


		for (unsigned int i = 0; i < touchDataArray.size(); i++) // draw vectors being draged;
		{
			TouchData* touchData = &touchDataArray[i];
			if (touchData->intervalVector != 0)
			{
				double xx = touchData->dragVector(0);
				double yy = touchData->dragVector(1);

				ID2D1SolidColorBrush* brush = touchData->intervalVector == &keyDuplicateVec ? m_pGlowBrush: m_pGeneratorBrush;

				m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), brush, 4, NULL);

				double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
				angle += yy < 0 ? 180 : 0;  //Corect for phase

				D2D1_MATRIX_3X2_F t;
				m_d2dContext->GetTransform(&t);
				D2D1_MATRIX_3X2_F t2 =  D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
				m_d2dContext->SetTransform(t2);
				m_d2dContext->FillGeometry(m_pArrowPathGeometry, brush);
				m_d2dContext->SetTransform(t);

			}

		}

	}

  {
    double x = 0;
    double y = 0;
    Lattice::latticeToCartesian(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
    x -= originXDraw;
    y -= originYDraw;
    double xx = lattice.largeGen1(0);
    double yy = lattice.largeGen1(1);

    m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGeneratorBrush, 4, NULL);

    double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
    angle += yy < 0 ? 180 : 0;  //Corect for phase

    D2D1_MATRIX_3X2_F t;
    m_d2dContext->GetTransform(&t);
    D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
    m_d2dContext->SetTransform(t2);
    m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGeneratorBrush);
    m_d2dContext->SetTransform(t);
  }

  {
    double x = 0;
    double y = 0;
    Lattice::latticeToCartesian(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
    x -= originXDraw;
    y -= originYDraw;
    double xx = lattice.largeGen2(0);
    double yy = lattice.largeGen2(1);

    m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGlowBrush, 4, NULL);

    double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
    angle += yy < 0 ? 180 : 0;  //Corect for phase

    D2D1_MATRIX_3X2_F t;
    m_d2dContext->GetTransform(&t);
    D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
    m_d2dContext->SetTransform(t2);
    m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGlowBrush);
    m_d2dContext->SetTransform(t);
  }
}

double LatticeView::getCents(double x, double y)
{
	//double x1, y1, x2, y2;
	//interval1ToCartesian(x1, y1);
	//interval2ToCartesian(x2, y2);

	//Lattice::cartesianToLattice(&x, &y, &Vector2d(x1, y1), &Vector2d(x2, y2));

	//return temperament.getCents(x, y);

	Vector2d pos;
	pos(0) = x;
	pos(1) = y;

	getGenPer(pos);

	return temperament.getCents(pos(0), pos(1), duplicateRatios);
}

double LatticeView::getDeltaCents(double dx, double dy)
{
	Vector2d dPos(dx, dy);
  switch(pitchAxis)
  {
    case HORIZONTAL:
      dPos(1) = 0;
      break;

    case VERTICAL:
      dPos(0) = 0;
      break;
  }

	Lattice::changeToBasis(&dPos, &lattice.largeGen1, &lattice.largeGen2);
	Lattice::changeToBasis(&dPos, &generatorVec, &periodVec);

	return temperament.genCents*dPos(0) + temperament.periodCents*dPos(1);
}


int LatticeView::getGenPer(Vector2d& pos)
{
	if (!dupKeys)
	{
		Lattice::changeToBasis(&pos, &lattice.largeGen1, &lattice.largeGen2);
		Lattice::changeToBasis(&pos, &generatorVec, &periodVec);
		return 0;
	}

	Vector2d posInit = pos;
	lattice.getCell(&pos(0), &pos(1));
	lattice.latticeToCartesian(&pos(0), &pos(1));          // show be able to remove these two steps
	Lattice::changeToBasis(&pos, &lattice.largeGen1, &lattice.largeGen2);  // should be able to remove these two steps

	VectorXI posI(2);
	posI(0) = lrint(pos(0));
	posI(1) = lrint(pos(1));

	MatrixXI A = MatrixXI::Zero(2,3);
	A(0, 0) = keyDuplicateVec(0);
	A(1, 0) = keyDuplicateVec(1);
	A(0, 1) = periodVec(0);
	A(1, 1) = periodVec(1);
	A(0, 2) = generatorVec(0);
	A(1, 2) = generatorVec(1);


	MatrixXI U, V;
	Temperament::diagonalize(U, A, V);

	Index rank = 0;
	while (rank < A.rows() && A(rank, rank) != 0)
		rank++;
	
	if (rank == 2)
	{
		VectorXI d = U*posI;
		VectorXI y(3);
		Index i = 0;
		for (; i < rank; i++)
		{
			if (d(i) % A(i, i))
			{
				pos = posInit;
				Lattice::changeToBasis(&pos, &lattice.largeGen1, &lattice.largeGen2);
				Lattice::changeToBasis(&pos, &generatorVec, &periodVec);
				return 0;
			}
			else
				y(i) = d(i) / A(i, i);
		}

		double y2 = -((double)(V(0, 0)*y(0) + V(0, 1)*y(1))) / V(0, 2);
		y(2) = lrint(y2);
		if (V(0, 2)>0)
		{
			if (y(2) < y2)
				y(2)++;
	    }
		else
		{
		   if (y(2) > y2)
			y(2)--;

		}

		VectorXI x = V*y;
		pos(1) = x(1);
		pos(0) = x(2);

		return x(0);
	}
	else
	{
		pos = posInit;
		Lattice::changeToBasis(&pos, &lattice.largeGen1, &lattice.largeGen2);
		Lattice::changeToBasis(&pos, &generatorVec, &periodVec);
		return 0;
	}
}

/** Helper method that converts hue to rgb */
double hueToRgb(double p, double q, double t) {
  if (t < 0.0)
    t += 1.0;
  if (t > 1.0)
    t -= 1.0;
  if (t < 1.0 / 6.0)
    return p + (q - p) * 6.0 * t;
  if (t < 1.0 / 2.0)
    return q;
  if (t < 2.0 / 3.0)
    return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
  return p;
}

void hslToRgb(double& h, double& s, double& l) {
   if (s == 0.0) {
    h = s = l = l; // achromatic
  }
  else {
    double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    double p = 2 * l - q;
    double hue = h;
    h = hueToRgb(p, q, hue + 1.0 / 3.0);
    s = hueToRgb(p, q, hue);
    l = hueToRgb(p, q, hue - 1.0 / 3.0);
  }
}

/*
void LatticeView::UpdateColor(Windows::UI::Color c)
{
  ////double num12StepsD = colorCents / 100;
  ////int num12Steps = round(num12StepsD);
  ////double A = num12StepsD - num12Steps;
  ////if (abs(A) < .0001)
  ////{
  ////  num12Steps = num12Steps % 12;
  ////  if (num12Steps < 0)
  ////    num12Steps += 12;

  ////  int colorOffset = 3;
  ////  int index = (7 * (num12Steps)+colorOffset) % 12;

  ////  colors[index][0] = c.R;
  ////  colors[index][1] = c.G;
  ////  colors[index][2] = c.B;

  ////}

//  r3 = (1 - A)*r1 + A * r2
//  r3' = (1 - A)*r1' + A * r2'

//  r3 + d = (1 - A)*(r1 + d1) + A * (r2 + d2) 
//  r3 + d = (1 - A)*r1 + A * r2 + (1 - A)*d1 + A * d2 
//  d = (1 - A)*d1 + A * d2 

//  if A = 0 then we only want to change r1, and when A= 0 we only want to change r2.
// Thus it seems that d1 should be proportonal to (1 - A) and d2 should be proportonal to A.
// Thus we have 
// //  d = (1 - A)*e, d2 =  A * e 
// so
// d = (1 - A)^2*e + A^2 * e =   ((1 - A)^2 + A^2 ) e
// so 
// e = d/((1 - A)^2 + A^2 ) 
 

  double num12StepsD = colorCents / 100;
  int num12Steps = floor(num12StepsD);
  double A = num12StepsD - num12Steps;
  num12Steps = num12Steps % 12;
  if (num12Steps < 0)
    num12Steps += 12;

  int colorOffset = 3;
  int index1 = (7 * (num12Steps)+colorOffset) % 12;


  double r1 = colors[index1][0];
  double g1 = colors[index1][1];
  double b1 = colors[index1][2];


  num12Steps = ceil(num12StepsD);
  num12Steps = num12Steps % 12;
  if (num12Steps < 0)
    num12Steps += 12;

  int index2 = (7 * (num12Steps)+colorOffset) % 12;

  double r2 = colors[index2][0];
  double g2 = colors[index2][1];
  double b2 = colors[index2][2];

  double r3 = (1 - A)*r1 + A * r2;
  double g3 = (1 - A)*g1 + A * g2;
  double b3 = (1 - A)*b1 + A * b2;


  double dr = c.R - r3;
  double dg = c.G - g3;
  double db = c.B - b3;

  colors[index1][0] += dr * (1 - A) / ((1 - A) * (1 - A) + A * A);
  colors[index1][1] += dg * (1 - A) / ((1 - A) * (1 - A) + A * A);
  colors[index1][2] += db * (1 - A) / ((1 - A) * (1 - A) + A * A);

  colors[index2][0] += dr * A / ((1 - A) * (1 - A) + A * A);
  colors[index2][1] += dg * A / ((1 - A) * (1 - A) + A * A);
  colors[index2][2] += db * A / ((1 - A) * (1 - A) + A * A);
}


Windows::UI::Color LatticeView::ColorFromCents(double cents)
{
  int edo = 12;
  double numStepsD = cents / (1200.0/edo);
  int numSteps = floor(numStepsD);
  double A = numStepsD - numSteps;
  numSteps = numSteps % edo;
  if (numSteps < 0)
    numSteps += edo;

  int colorOffset = 0;
  int index = (7 * (numSteps)+colorOffset) % edo;


  double r1 = colors[index][0];
  double g1 = colors[index][1];
  double b1 = colors[index][2];


  numSteps = ceil(numStepsD);
  numSteps = numSteps % edo;
  if (numSteps < 0)
    numSteps += edo;

  index = (7 * (numSteps)+colorOffset) % edo;

  double r2 = colors[index][0];
  double g2 = colors[index][1];
  double b2 = colors[index][2];

  r1 = (1 - A)*r1 + A * r2;
  g1 = (1 - A)*g1 + A * g2;
  b1 = (1 - A)*b1 + A * b2;

  Windows::UI::Color c = Windows::UI::ColorHelper::FromArgb(255, r1,g1,b1);
  return c;
}
*/

ID2D1SolidColorBrush* LatticeView::BrushFromCents(double cents)
{
  ID2D1SolidColorBrush*& brush = whiteBrushes[cents];

  if (!brush)
  {
    int edo = 24;
    int (*colors)[3];
    switch (colorMode)
    {
      case _12EDO_Rainbow:
        edo = 12;
        colors = colorsRanbow;
        break;

      default:
        edo = 24;
        colors = colorsBlackAndWhite;
        break;
    }

    double numStepsD = cents / (1200.0 / edo);
    int numSteps = floor(numStepsD);
    double A = numStepsD - numSteps;
    numSteps = numSteps % edo;
    if (numSteps < 0)
      numSteps += edo;

    int colorOffset = 0;
    int index = (7 * (numSteps)+colorOffset) % edo;


    double r1 = colors[index][0];
    double g1 = colors[index][1];
    double b1 = colors[index][2];


    numSteps = ceil(numStepsD);
    numSteps = numSteps % edo;
    if (numSteps < 0)
      numSteps += edo;

    index = (7 * (numSteps)+colorOffset) % edo;

    double r2 = colors[index][0];
    double g2 = colors[index][1];
    double b2 = colors[index][2];

    r1 = (1 - A)*r1 + A * r2;
    g1 = (1 - A)*g1 + A * g2;
    b1 = (1 - A)*b1 + A * b2;

//    a = .0*sin(a*2*PI);
//
//    if (a < 0)
//    {
//      a = -a;
//      r1 = (1 - a)*r1;
//      g1 = (1 - a)*g1;
//      b1 = (1 - a)*b1;
//    }
//    else
//    {
//      r1 = (1 - a)*r1 + a;
//      g1 = (1 - a)*g1 + a;
//      b1 = (1 - a)*b1 + a;
//    }

    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(r1/255, g1/255 , b1/255, 1.0f), &brush);
  }
  
  return brush;
}




//ID2D1SolidColorBrush* LatticeView::BrushFromCents(double cents)
//{
//  ID2D1SolidColorBrush*& brush = whiteBrushes[cents];
//
//  if (!brush)
//  {
//    int edo = 72;
//    double stepSize =1200.0/ edo;
////    int fifthSteps = round(701.955/stepSize);
//    int fifthSteps = 7;
//
//    //    double a = 2; // a must be between 0 and 2.
//    //    double c = .25; // c must be between 0 and 1. .25 is probobly around where it should be.
//    double b = 1 - a * (0.5 + 1.0 / (8 * PI) * (sin(4 * PI * (1 + c)) - sin(4 * PI*c)));
//    double C = -b * c - a * (c / 2 + 1.0 / (8 * PI)*sin(4 * PI*c));
//
//    double numStepsD = cents / (1200.0/edo);
//    int numSteps = floor(numStepsD);
//    double A = numStepsD - numSteps;
//    numSteps = numSteps % edo;
//    if (numSteps < 0)
//      numSteps += edo;
//
//    int colorOffset = 0;
//
//    double h = (double)((fifthSteps * numSteps + colorOffset) % edo) / edo;
//    h += shift;
//    h = h - floor(h);
//    h = a * (0.5  * (h + c) + 1.0 / (8 * PI) * sin(4 * PI * (h + c))) + b * (h + c) + C;
//    double s = saturation;
//    double var = .5;
//    double l = lightness* (var*(cos(2*PI* (numSteps%6)/5.0)/2+.5)+1-var);
//    hslToRgb(h, s, l);
//
//    double r1 = h;
//    double g1 = s;
//    double b1 = l;
//
//
//    numSteps = ceil(numStepsD);
//    numSteps = numSteps % edo;
//    if (numSteps < 0)
//      numSteps += edo;
//
//    h = (double)((fifthSteps * numSteps + colorOffset) % edo) / edo;
//    h = a * (0.5 * (h + c) + 1.0 / (8 * PI) * sin(4 * PI * (h + c))) + b * (h + c) + C;
//    h += shift;
//    h = h - floor(h);
//    s = saturation;
//    l = lightness * (var*(cos(2 * PI* (numSteps % 6) / 5.0) / 2 + .5) + 1 - var);
//    hslToRgb(h, s, l);
//
//    double r2 = h;
//    double g2 = s;
//    double b2 = l;
//
//    r1 = (1 - A)*r1 + A * r2;
//    g1 = (1 - A)*g1 + A * g2;
//    b1 = (1 - A)*b1 + A * b2;
//
//    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(r1, g1, b1, 1.0f), &brush);
//  }
//
//  return brush;
//}


ID2D1SolidColorBrush* LatticeView::getWhiteBrushFromCents(double cents)
{
  ID2D1SolidColorBrush*& brush = whiteBrushes[cents];

  if(! brush)
  { 
    double power = 1.2;
    double r,g,b;
    if(cents > 0)
    {
      if (cents <= 113.685006058 / 2)
      {
        double c = cents/(113.685006058 / 2);
        r = .5*pow(1-c, power) + .5;
        g = .5*pow(1-c, power) + .5;
        b = (1 - c) + .5*c;
      }
      else
      {
        double c = (cents- 113.685006058 / 2)/(113.685006058 / 2);
        c = c <= 1 ? c : 1;
        r = .5*(1 - c);
        g = .5*(1 - c);
        b = .5*(1 - c) + c;
      }
    }
    else
    {
      cents *= -1;
      if (cents <= 113.685006058 / 2)
      {
        double c = cents / (113.685006058 / 2);
        r = (1 - c) + .5*c;
        g = .5*pow(1 - c, power) + .5;
        b = .5*pow(1 - c, power) + .5;
      }
      else
      {
        double c = (cents - 113.685006058 / 2) / (113.685006058 / 2);
        c = c <= 1 ? c : 1;
        r = .5*(1 - c) + c;
        g = .5*(1 - c);
        b = .5*(1 - c);
      }
    }
     
    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(r, g, b, 1.0f)), &brush);
  }

  return brush;
}


ID2D1SolidColorBrush* LatticeView::getBlackBrushFromCents(double cents)
{

  ID2D1SolidColorBrush*& brush = blackBrushes[cents];

  if (!brush)
  {
    double power = 1.2;
    double r, g, b;
    if (cents > 0)
    {
      if (cents <= 113.685006058 / 2)
      {
        double c = cents / (113.685006058 / 2);
        r = .5*c;
        g = .5*c;
        b = 1 - (.5*pow(1 - c, power) + .5);
      }
      else
      {
        double c = (cents - 113.685006058 / 2) / (113.685006058 / 2);
        c = c <= 1 ? c : 1;
        r = .5*(1 - c);
        g = .5*(1 - c);
        b = .5*(1 - c) + c;
      }
    }
    else
    {
      cents *= -1;
      if (cents <= 113.685006058 / 2)
      {
        double c = cents / (113.685006058 / 2);
        r = 1 - (.5*pow(1 - c, power) + .5);
        g = .5*c;
        b = .5*c;
      }
      else
      {
        double c = (cents - 113.685006058 / 2) / (113.685006058 / 2);
        c = c <= 1 ? c : 1;
        r = .5*(1 - c) + c;
        g = .5*(1 - c);
        b = .5*(1 - c);
      }
    }

    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(r, g, b, 1.0f)), &brush);
  }

  return brush;
}

// This one uses images for the text, but draws the cells from scratch each time. This seems to be by far the fastest.
HRESULT LatticeView::drawCell(double x, double y)
{
	HRESULT hr = S_OK;

	Vector2d note;
	note(0) = x + originXDraw;
	note(1) = y + originYDraw;

	int keyNum = getGenPer(note);

	int gen = lrint(note(0));
	int per = lrint(note(1));


	NoteName realName;
	NoteName* name;
	if (abs(per - note(1)) < .00001 && abs(gen - note(0)) < .00001)
	//{
	//	realName = temperament.createNoteName(xNoteInt, yNoteInt);
	//	name = &realName;
	//}
	{
		name = temperament.borrowNoteName(gen, per, duplicateRatios,  showName,  scientificPitchNotation,  use53TETNotation);
	}
	else
	{
		name = &realName;
		name->ratio.num = 0;
	}

	D2D1_MATRIX_3X2_F t;
	m_d2dContext->GetTransform(&t);

//	int lowerGenLimit = -35; int upperGenLimit = 37; //72 tet
//	int lowerGenLimit = -1; int upperGenLimit = 10;//12 tet
//	int lowerPerLimit = -1000000; int upperPerLimit = 1000000;
	//	if (gen >= 21 || gen < -10) return name; //31 tet

//	if (!(gen >= lowerGenLimit && gen <= upperGenLimit && per >= lowerPerLimit && per <= upperPerLimit))
//		return hr;

	if (name->name.size() > 0 || playUnamedNotes)
	{
		double cents = temperament.getCents(gen, per, duplicateRatios) + temperament.getRootCents() - temperament.shiftCents;
		if (cents < 0 && cents > -.001)
			cents = 0;

    double ratioCents = 100000000;
    if(name->ratio.num != 0 && name->ratio.denom != 0)
		  ratioCents = 1200 * log2(static_cast<double>(name->ratio.num) / name->ratio.denom);

//		if (!temperament.scientificPitchNotation && (abs(cents - ratioCents) > 100 || name->name.size() == 0))
//		{
//		}
//		else
    {
		    //////new way using bitmap should be faster
		    //if(name->sharps == 0)
		    //  m_d2dContext->DrawImage(whiteCellBitMap.Get(), D2D1::Point2F(x + lattice.cellMinX - CELLBITMAPMARGIN, y + lattice.cellMinY - CELLBITMAPMARGIN));
		    //else
		    //  m_d2dContext->DrawImage(blackCellBitMap.Get(), D2D1::Point2F(x + lattice.cellMinX - CELLBITMAPMARGIN, y + lattice.cellMinY - CELLBITMAPMARGIN));
	

         ID2D1SolidColorBrush* brush = NULL;
         brush = BrushFromCents(cents);
    //    if (name->sharps == 0)
    //      brush = getWhiteBrushFromCents(name->accidentalCents);
    //    else
    //      brush = getBlackBrushFromCents(name->accidentalCents);

        D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Translation(x, y)*t;
        m_d2dContext->SetTransform(t2);

        // old way of rendering cell path directly
        m_d2dContext->FillGeometry(m_pCellPathGeometry, brush);
        m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pBorderBrush, 2.5);
    }

		D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Translation(x, y)*t;
		m_d2dContext->SetTransform(t2);

		if (lattice.cellWidth > 20 && lattice.cellHeight > 20)
		{
			wchar_t str[256];
      wchar_t* str_p  = &str[0];
			unsigned int nameLength = 0;
      swprintf_s(str, 256, L"");

      vector<wstring> centDev;
      for(int n = 0; n < name->name.size(); n++)
      {
        if (n > 0)
          swprintf_s(str, 256, L"%s\n%s%s", str, name->name[n].c_str(), name->accidental[n].c_str());
        else
          swprintf_s(str, 256, L"%s%s%s", str, name->name[n].c_str(), name->accidental[n].c_str());

        if (name->accidentalCents[n] < -.5)
     //     centDev.push_back( L"↓" + to_wstring((int)abs(round(name->accidentalCents[n]))));
          centDev.push_back(L"-" + to_wstring((int)round(abs(name->accidentalCents[n]))));
        else if (name->accidentalCents[n] > 0.5)
       //   centDev.push_back( L"↑" + to_wstring((int)abs(round(name->accidentalCents[n]))));
          centDev.push_back(L"+" + to_wstring((int)round(abs(name->accidentalCents[n]))));
        else
          centDev.push_back(L"");

        swprintf_s(str, 256, L"%s%s", str, centDev.back().c_str());
      }

      if (showRatio && ratioCents < 100000000)
				swprintf_s(str, 256, L"%s\n%d:%d", str, (int) name->ratio.num, (int)name->ratio.denom);


			nameLength = wcslen(str);

			wchar_t strKey[256];
      if (keyNum > 0)
        swprintf_s(strKey, 256, L"●%d●", keyNum);
			
      if(showCents)
      { 
        if(showRatio && ratioCents < 100000000)
          swprintf_s(str, 256, L"%s\nJI=%.2f\n¢=%.2f", str, ratioCents, cents);
        else 
          swprintf_s(str, 256, L"%s\n¢=%.2f", str, cents);
        //  swprintf_s(str, 256, L"%s\n¢=%.2f", str, name->accidentalCents);
      }

      if(showDelta && ratioCents < 100000000)
        swprintf_s(str, 256, L"%s\nΔ=%.2f", str, cents - ratioCents);
       
      if (keyNum > 0)
        swprintf_s(str, 256, L"%s\n%s", str, strKey);

 //     keyNum = 0;

       
      if(str[0] == L'\n')  // if string starts with a charage return, remove it.
      {
        str_p++;
        if(nameLength > 0)
          nameLength--;
      }

			if (name->pTextLayout.size() <= keyNum)
				name->pTextLayout.resize(keyNum+1, 0);

			if (name->pTextLayout[keyNum] == 0)
			{
				hr = m_dwriteFactory->CreateTextLayout(
          str_p,
					wcslen(str_p),
					m_pTextFormat,  // The text format to apply to the string (contains font information, etc).
			//		lattice.cellWidth,         // The width of the layout box.
			//		lattice.cellHeight,        // The height of the layout box.
					200,         // The width of the layout box.
					200,        // The height of the layout box.
					&name->pTextLayout[keyNum]  // The IDWriteTextLayout interface pointer.
					);

        if (name->name.size()> 0 && name->name[0].size() > 0)
        {
          DWRITE_TEXT_RANGE textRange = { 0, name->name[0].size() };
          //	DWRITE_TEXT_RANGE textRange2 = { name->name.length(), name->name.length()};
          if (SUCCEEDED(hr))
          {
            hr = name->pTextLayout[keyNum]->SetFontFamilyName(temperament.fontName.c_str(), textRange);
            hr = name->pTextLayout[keyNum]->SetFontSize(temperament.fontSize, textRange);
          }
        }

        if(wcslen(str_p) - nameLength - (keyNum > 0 ? 3 : 0))
        {
				  DWRITE_TEXT_RANGE textRange = { nameLength, wcslen(str_p) - nameLength - (keyNum > 0 ? 3 : 0) };
				  if (SUCCEEDED(hr))
				  {
					  hr = name->pTextLayout[keyNum]->SetFontSize(10, textRange);
				  }
        }

       if(name->accidental.size() > 0 )
       {
          double fontSize = 20;
          int index = 0;
          for (int n = 0; n < name->name.size(); n++)
          {
            if(n >0)
              fontSize *= .8;

            if (SUCCEEDED(hr))
            {
              DWRITE_TEXT_RANGE textRange = { index- (n>0),  name->name[n].length() + (n>0) };
              hr = name->pTextLayout[keyNum]->SetFontSize(fontSize, textRange); // make space small at begining of accidental string
            }

            index += name->name[n].length();

            if(name->accidental[n].length() >0)
            {
              if (SUCCEEDED(hr))
              {
                DWRITE_TEXT_RANGE textRange = { index, 1 };
                hr = name->pTextLayout[keyNum]->SetFontSize(fontSize*.25 , textRange); // make space small at begining of accidental string
              }
              
              index++;  //move passed space

              if(SUCCEEDED(hr) )
              {
                DWRITE_TEXT_RANGE textRange3 = { index, name->accidental[n].length() - 1 };
                hr = name->pTextLayout[keyNum]->SetFontCollection(customFonts, textRange3);
                hr = name->pTextLayout[keyNum]->SetFontFamilyName(L"Sagittal", textRange3);
                // hr = name->pTextLayout[keyNum]->SetFontFamilyName(L"Bravura", textRange2);

                hr = name->pTextLayout[keyNum]->SetFontSize(fontSize*1.65, textRange3);
              }

           
              index += name->accidental[n].length() - 1; // move passed accidental
            }
 
            if (centDev[n].length() >0)
            {
              if (SUCCEEDED(hr))
              {
                DWRITE_TEXT_RANGE textRange4 = { index, centDev[n].length() };
                hr = name->pTextLayout[keyNum]->SetFontSize(fontSize*.6, textRange4); // make space small at begining of accidental string
              }

              index+= centDev[n].length();
            }

            index++; // beacuse of line break
          }
        }	
			}
			name->pTextLayout[keyNum]->GetMaxHeight();
			name->pTextLayout[keyNum]->GetMaxWidth();

			for (int i = 0; i < lattice.textPos.size(); i++)
			{
        D2D1_MATRIX_3X2_F t;
        m_d2dContext->GetTransform(&t);
        D2D1_POINT_2F textPos = D2D1::Point2F(lattice.textPos[i](0)*lattice.largeGen2(0) + lattice.textPos[i](1)* lattice.largeGen1(0) - name->pTextLayout[keyNum]->GetMaxWidth() / 2, lattice.textPos[i](0)*lattice.largeGen2(1) + lattice.textPos[i](1)* lattice.largeGen1(1) - name->pTextLayout[keyNum]->GetMaxHeight() / 2);
        D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Translation(textPos.x, textPos.y)*D2D1::Matrix3x2F::Scale(D2D1::SizeF(textSize, textSize), D2D1::Point2F())*t;
        m_d2dContext->SetTransform(t2);
        m_d2dContext->DrawTextLayout(D2D1::Point2F(), name->pTextLayout[keyNum], (keyNum < 0 ? m_pGlowBrush : (lightness >.5 ? m_pBlackBrush : m_pWhiteBrush)));
        //m_d2dContext->DrawTextLayout(D2D1::Point2F(), name->pTextLayout[keyNum], (keyNum < 0 ? m_pGlowBrush : (name->sharps == 0 ? m_pBlackBrush : m_pWhiteBrush)));
        m_d2dContext->SetTransform(t);  
		  }
		}
	}

	m_d2dContext->SetTransform(t);
  return hr;
}


void LatticeView::startPlayingNote(TouchData* touchData)
{
	Vector2d pos = touchData->pos;
	double originXScreen = originXToScreen;
	double originYScreen = originYToScreen;

	double x = pos(0) + originXScreen;
	double y = pos(1) + originYScreen;


	lattice.getCell(&x, &y);
	lattice.latticeToCartesian(&x, &y);
	

	touchData->cellPos = Vector2d(x - originXScreen, y - originYScreen);
	touchData->cellPosOrig = touchData->cellPos;

  Vector2d p;
  p(0) = x;
  p(1) = y;

  getGenPer(p);

  touchData->cents = temperament.getCents(p(0), p(1), duplicateRatios);
/*
  {
    colorCents = touchData->cents + temperament.getRootCents();
    mainPage->mainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
    ref new Windows::UI::Core::DispatchedHandler([this]()
    {
      updateColor = false;
      mainPage->setColor(ColorFromCents(colorCents));
      updateColor = true;
    }));
  }
  */

  if (notesToMatch.size() > 0)
  {
 //   PRINT(L"touchData->cents = %f\n", touchData->cents);
 //   PRINT(L"notesToMatch.back().cents = %f\n", notesToMatch.back().cents);
 //   PRINT(L"1200 * ((int)round((notesToMatch.back().cents) / 1200) = %d\n", 1200 * ((int)round(notesToMatch.back().cents) / 1200));
 //   PRINT(L"lrint(touchData->cents) mod 1200 = %d\n\n", (int)round(touchData->cents) % 1200);
    int per = 1200 * ((int)round(notesToMatch.back().cents) / 1200);
    touchData->cents = (int)round(touchData->cents) % 1200;
    if(notesToMatch.back().cents < 0 && touchData->cents > 0)
      touchData->cents -= 1200;
  
    if (notesToMatch.back().cents > 0 && touchData->cents < 0)
      touchData->cents += 1200;

    touchData->cents += per;
  }

  touchData->name = temperament.borrowNoteName(lrint(p(0)), lrint(p(1)), duplicateRatios,  showName,  scientificPitchNotation,  use53TETNotation);
  touchData->midiNote = instrument.getMidiNoteFromFreq(temperament.frequencyFromCents(touchData->cents));    // MIDI note-on message: Key number (60 = middle C) 69 = A4



	if (!roundFirstNote && (afterTouchMode == SUSTAIN || afterTouchMode == VIBRATO))
	{
		double dx = pos(0) - touchData->cellPos(0);
		double dy = pos(1) - touchData->cellPos(1);
		double dCentes = getDeltaCents(dx, dy);
		touchData->cents += dCentes;
    touchData->origCents = touchData->cents;
	}
  else 	if (!roundFirstNote && afterTouchMode == BEND)
  {
    double dx = pos(0) - touchData->cellPos(0);
    double dy = pos(1) - touchData->cellPos(1);
    double dCentes = getDeltaCents(dx, dy);
    touchData->origCents = touchData->cents;
    touchData->cents += dCentes;

    touchData->deltaPitchPos(0) = dx;
    touchData->deltaPitchPos(1) = dy;
    touchData->deltaCents = dCentes;
  }
  else
  {
    touchData->origCents = touchData->cents;
  }


	double freq = temperament.frequencyFromCents(touchData->cents);

	int count = 0;
	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* td = &touchDataArray[i];
		double dx = td->pos(0) - touchData->pos(0);
		double dy = td->pos(1) - touchData->pos(1);
		if (td->noteID != -1 && td != touchData && (dx*dx + dy*dy) < touchDiameter*touchDiameter)
		{
			count++;
			touchData->noteID = td->noteID;
			touchData->cents = td->cents;
			touchData->origCents = td->origCents;
			touchData->cellPos = td->cellPos;
			touchData->cellPosOrig = td->cellPosOrig;
		}
	}

	if (count == 0)
	{
		count = 0;
		for (int i = 0; i < touchDataArray.size(); i++)
		{
			TouchData* td = &touchDataArray[i];
			if (td->noteID != -1 && td->origCents == touchData->origCents && td != touchData)
			{
				count++;
				touchData->noteID = td->noteID;
			}
		}

		if (count == 0 && (touchData->name->name.size() > 0 || playUnamedNotes))
		{
			if (notesToMatch.size() > 0 && notes.size() == 0)
			{
				Note& noteToMatch = notesToMatch.back();
				if (abs((int)noteToMatch.cents - (int)touchData->cents)%1200 > 5)
				{
          if(noteToMatch.noteToMatch)
            questions++;

          noteToMatch.noteToMatch = false;  //Signal that this note has been counted
				  Note note;
				  note.freq = noteToMatch.freq;
				  note.cents = noteToMatch.cents;
				  note.startTime = GetTickCount64();
				  note.endTime = note.startTime + 500;
				  note.noteToMatch = false;
				  notes.push_back(note);
				}
				else
				{
          if (noteToMatch.noteToMatch)
          {
            questions++;
            correct++;
          }

					notesToMatch.pop_back();     
          if(currentSequance.size() > 0)
            currentSequance.erase(currentSequance.begin());
				}
			}
			
		    touchData->noteID = instrument.playNote(freq);
		}
			
	}

	if (touchData->noteID == -1)
	{
		PRINT(_T("Note not played due to no noteID\n"));
	}
}


void LatticeView::updateBend(TouchData* touchData)
{
//  updateFrequancy(touchData);

  Vector2d pos = touchData->pos;
  double originXScreen = originXToScreen;
  double originYScreen = originYToScreen;

  Vector2d cellPos;
  cellPos(0) = pos(0) + originXScreen;
  cellPos(1) = pos(1) + originYScreen;

  lattice.getCell(&cellPos(0), &cellPos(1));
  lattice.latticeToCartesian(&cellPos(0), &cellPos(1));
  
  cellPos(0) -= originXScreen;
  cellPos(1) -= originYScreen;

  if ((touchData->cellPos - cellPos).norm() > .01)
  { 
    touchData->deltaPitchPos  = touchData->cellPos + touchData->deltaPitchPos - cellPos;
    touchData->cellPos = cellPos;
    touchData->origCents = getCents(cellPos(0)+ originXScreen, cellPos(1)+ originYScreen);
    touchData->deltaCents = touchData->cents - touchData->origCents;
  }

  //Vector2d dir = touchData->deltaPitchPos;
  //Lattice::changeToBasis(&dir, &lattice.largeGen1, &lattice.largeGen2);
  //Lattice::changeToBasis(&dir, &generatorVec, &periodVec);
  //touchData->deltaCents = temperament.genCents*dir(0) + temperament.periodCents*dir(1);



  //touchData->cents = touchData->origCents + getDeltaCents(touchData->pos(0) - touchData->posStart(0), touchData->pos(1) - touchData->posStart(1));

  //Vector2d pos = touchData->pos;
  //double originXScreen = originXToScreen;
  //double originYScreen = originYToScreen;

  //Vector2d cellPos;
  //cellPos(0) = pos(0) + originXScreen;
  //cellPos(1) = pos(1) + originYScreen;

  //lattice.getCell(&cellPos(0), &cellPos(1));
  //lattice.latticeToCartesian(&cellPos(0), &cellPos(1));
  //touchData->origCents = getCents(cellPos(0), cellPos(1));
  //
  //cellPos(0) -= originXScreen;
  //cellPos(1) -= originYScreen;
  //touchData->posStart = cellPos;

  //instrument.updateNoteFreq(touchData->noteID, temperament.frequencyFromCents(touchData->cents));

  //for (int i = 0; i < touchDataArray.size(); i++)
  //{
  //  TouchData* td = &touchDataArray[i];
  //  if (td->origCents == touchData->origCents && td != touchData)
  //  {
  //    td->cents = touchData->cents;
  //  }
  //}
}

void LatticeView::updateGlissando(TouchData* touchData)
{
	Vector2d pos = touchData->pos;
	double originXScreen = originXToScreen;
	double originYScreen = originYToScreen;

  Vector2d cellPos;
  cellPos(0) = pos(0) + originXScreen;
  cellPos(1) = pos(1) + originYScreen;

	lattice.getCell(&cellPos(0), &cellPos(1));
	lattice.latticeToCartesian(&cellPos(0), &cellPos(1));

  cellPos(0) -= originXScreen;
  cellPos(1) -= originYScreen;
  if((touchData->cellPos - cellPos).norm() > .01)
  { 
    TouchData td = *touchData;
    stopPlayingNote(touchData);
    *touchData = td;  
    startPlayingNote(touchData);
  }
}


//void LatticeView::updateGlissando(TouchData* touchData)
//{
//  Vector2d pos = touchData->pos;
//  double originXScreen = originXToScreen;
//  double originYScreen = originYToScreen;
//  double x = pos(0) + originXScreen;
//  double y = pos(1) + originYScreen;
//
//  lattice.getCell(&x, &y);
//  lattice.latticeToCartesian(&x, &y);
//
//  TouchData td = *touchData;
//  stopPlayingNote(touchData);
//  *touchData = td;
//  //  touchData->cents = getCents(x, y);
//  //	touchData->origCents = getCents(x, y);
//
//  startPlayingNote(touchData);
//
//  //if (vibrato) 
//  //{
//  //	updateVibrado(touchData);
//  //}
//  //else
//  //{
//  //	touchData->cents = touchData->origCents + getDeltaCents(x - originXScreen - touchData->posStart(0), y - originYScreen - touchData->posStart(1));
//
//  //	instrument.updateNoteFreq(touchData->noteID, temperament.frequencyFromCents(touchData->cents));
//
//  //	for (int i = 0; i < touchDataArray.size(); i++)
//  //	{
//  //		TouchData* td = &touchDataArray[i];
//  //		if (td->origCents == touchData->origCents && td != touchData)
//  //		{
//  //			td->cents = touchData->cents;
//  //		}
//  //	}
//  //}
//
//  touchData->cellPos(0) = x - originXScreen;
//  touchData->cellPos(1) = y - originYScreen;
//}
//
//void LatticeView::updateFrequancy(TouchData* touchData)
//{
//	touchData->cents = touchData->origCents + getDeltaCents(touchData->pos(0) - touchData->posStart(0), touchData->pos(1)- touchData->posStart(1));
//
//	instrument.updateNoteFreq(touchData->noteID, temperament.frequencyFromCents(touchData->cents));
//
//	for (int i = 0; i < touchDataArray.size(); i++)
//	{
//		TouchData* td = &touchDataArray[i];
//		if (td->origCents == touchData->origCents && td != touchData)
//		{
//			td->cents = touchData->cents;
//		}
//	}
//}



// Uses a single pole real time IIR high pass filter.
//void LatticeView::updateVibrado(TouchData* touchData)
//{
//	if (deltaT == 0)
//		return;
//
//	double triggerFactor = 1;
//
//	double deltaCents;
//	//if (usePitchAxis)
//	//{
//	//	Vector2d dPos(touchData->pos(0) - touchData->posStart(0), touchData->pos(1) - touchData->posStart(1));
//	//	double d = pitchAxis.dot(dPos);
//	//	Vector2d dPosPrev(touchData->prevVibratoPos.x - touchData->posStart(0), touchData->prevVibratoPos.y - touchData->posStart(1));
//	//	double dPrev = dPosPrev.dot(pitchAxis);
//	//	deltaCents =  getCents(touchData->posStart(0) + d*pitchAxis(0), touchData->posStart(1) + d*pitchAxis(1)) - getCents(touchData->posStart(0) + dPrev*pitchAxis(0), touchData->posStart(1) + dPrev*pitchAxis(1));
//	//}
//	//else
//	//{
//	//	deltaCents =  getCents(touchData->pos(0), touchData->pos(1)) - getCents(touchData->prevVibratoPos.x, touchData->prevVibratoPos.y);
//	//}
//
//	deltaCents = getDeltaCents(touchData->pos(0) - touchData->posStart(0), touchData->pos(1) - touchData->posStart(1)) - getDeltaCents(touchData->prevVibratoPos(0) - touchData->posStart(0), touchData->prevVibratoPos(1) - touchData->posStart(1));
//
//	if (vibratoTrigger)
//	{
//		double a = 1;
//		if (triggerResetRate != 0)
//		{
//			double RC = 1 / (2 * PI* triggerResetRate);
//			a = RC / (RC + deltaT);
//		}
//
//		touchData->trigMax = a*touchData->trigMax;
//		if ( deltaCents > touchData->trigMax)
//			touchData->trigMax = deltaCents;			
//
//		touchData->trigMin = a*touchData->trigMin;
//
//		if ( deltaCents < touchData->trigMin)
//			touchData->trigMin = deltaCents;
//
//		if (touchData->trigMax < 0)
//			touchData->trigMax = 0;
//
//		if (touchData->trigMin > 0)
//			touchData->trigMin = 0;
//
////		PRINT((to_wstring(touchData->trigMax) + L", " + to_wstring(touchData->trigMin) + L"\n").c_str());
//	}
//
//
//	{
//		double a = 1;
//		if (vibratoCuttoffFreq > 0)
//		{
//			double RC =  1 / (2 * PI* vibratoCuttoffFreq );
//			a = RC / (RC + deltaT);
//		}
//
//		if (!vibratoTrigger || (touchData->trigMin < -triggerThreshold && touchData->trigMax > triggerThreshold))
//			touchData->deltaCents = a*(touchData->deltaCents + vibratoAmplitude * deltaCents);
//		else
//			touchData->deltaCents = a*touchData->deltaCents;
//	}
//
//
//  Vector2d dir = touchData->pos - touchData->posStart;
//  Vector2d dir2 = dir;
//  Lattice::changeToBasis(&dir2, &lattice.largeGen1, &lattice.largeGen2);
//  Lattice::changeToBasis(&dir2, &generatorVec, &periodVec);
//  double centsLong = temperament.genCents*dir2(0) + temperament.periodCents*dir2(1);
//  if (abs(centsLong) > .01)
//    touchData->deltaPitchPos = dir*touchData->deltaCents / centsLong;
//
//	touchData->cents = touchData->origCents + touchData->deltaCents;
//
//	touchData->prevVibratoPos = touchData->pos;
//
//	instrument.updateNoteFreq(touchData->noteID, temperament.frequencyFromCents(touchData->cents));
//
//
//	for (int i = 0; i < touchDataArray.size(); i++)
//	{
//		TouchData* td = &touchDataArray[i];
//		if (td->origCents == touchData->origCents && td != touchData)
//		{
//			td->cents = touchData->cents;
//		}
//	}
//}


void LatticeView::updateVibratoFreq(TouchData* touchData)
{
  if (deltaT == 0)
    return;

  double triggerFactor = 1;

  //if (usePitchAxis)
  //{
  //	Vector2d dPos(touchData->pos(0) - touchData->posStart(0), touchData->pos(1) - touchData->posStart(1));
  //	double d = pitchAxis.dot(dPos);
  //	Vector2d dPosPrev(touchData->prevVibratoPos.x - touchData->posStart(0), touchData->prevVibratoPos.y - touchData->posStart(1));
  //	double dPrev = dPosPrev.dot(pitchAxis);
  //	deltaCents =  getCents(touchData->posStart(0) + d*pitchAxis(0), touchData->posStart(1) + d*pitchAxis(1)) - getCents(touchData->posStart(0) + dPrev*pitchAxis(0), touchData->posStart(1) + dPrev*pitchAxis(1));
  //}
  //else
  //{
  //	deltaCents =  getCents(touchData->pos(0), touchData->pos(1)) - getCents(touchData->prevVibratoPos.x, touchData->prevVibratoPos.y);
  //}

  Vector2d deltaPos = touchData->pos - touchData->prevVibratoPos;

  switch(pitchAxis)
  {
    case HORIZONTAL:
      deltaPos(1) = 0;
      break;
  
    case VERTICAL:
      deltaPos(0) = 0;
      break;
  }


  Vector2d dir = deltaPos;
  Lattice::changeToBasis(&dir, &lattice.largeGen1, &lattice.largeGen2);
  Lattice::changeToBasis(&dir, &generatorVec, &periodVec);
  double deltaCents = temperament.genCents*dir(0) + temperament.periodCents*dir(1);

  if (vibratoTrigger)
  {
    double a = 1;
    if (triggerResetRate != 0)
    {
      double RC = 1 / (2 * PI* triggerResetRate);
      a = RC / (RC + deltaT);
    }

    touchData->trigMax = a*touchData->trigMax;
    if (deltaCents > touchData->trigMax)
      touchData->trigMax = deltaCents;

    touchData->trigMin = a*touchData->trigMin;

    if (deltaCents < touchData->trigMin)
      touchData->trigMin = deltaCents;

    if (touchData->trigMax < 0)
      touchData->trigMax = 0;

    if (touchData->trigMin > 0)
      touchData->trigMin = 0;

    //		PRINT((to_wstring(touchData->trigMax) + L", " + to_wstring(touchData->trigMin) + L"\n").c_str());
  }

  
  double x0 = .7;
  double vibratoCuttoffFreq = 10*(x0-1.2)/x0 * vibratoCuttoff/(vibratoCuttoff-1.2);

  {
    double a = 1;
    if (vibratoCuttoffFreq > 0.01)
    {
      double RC = 1 / (2 * PI* vibratoCuttoffFreq);
      a = RC / (RC + deltaT);
    }

    if (!vibratoTrigger || (touchData->trigMin < -triggerThreshold && touchData->trigMax > triggerThreshold))
      touchData->deltaPitchPos = a*(touchData->deltaPitchPos + vibratoAmplitude * deltaPos);
   	else
    	touchData->deltaPitchPos = a*touchData->deltaPitchPos;
  }

  {
    Vector2d dir = touchData->deltaPitchPos;
    Lattice::changeToBasis(&dir, &lattice.largeGen1, &lattice.largeGen2);
    Lattice::changeToBasis(&dir, &generatorVec, &periodVec);
    touchData->deltaCents = temperament.genCents*dir(0) + temperament.periodCents*dir(1);
  }

  touchData->cents = touchData->origCents + touchData->deltaCents;

  touchData->prevVibratoPos = touchData->pos;

  instrument.updateNoteFreq(touchData->noteID, temperament.frequencyFromCents(touchData->cents));


  for (int i = 0; i < touchDataArray.size(); i++)
  {
    TouchData* td = &touchDataArray[i];
    if (td->origCents == touchData->origCents && td != touchData)
    {
      td->cents = touchData->cents;
    }
  }
}

void LatticeView::updateBendFreq(TouchData* touchData)
{
  if (deltaT == 0)
    return;
 
  Vector2d deltaPos = touchData->pos - touchData->prevVibratoPos;

  switch (pitchAxis)
  {
  case HORIZONTAL:
    deltaPos(1) = 0;
    break;

  case VERTICAL:
    deltaPos(0) = 0;
    break;
  }


  double x0 = .7;
  bendCuttoff = 10 * (x0 - 1.2) / x0 * bendCuttoff / (bendCuttoff - 1.2);

  {
    double a = 1;
    if (bendCuttoff > 0.01)
    {
      double RC = 1 / (2 * PI* bendCuttoff);
      a = RC / (RC + deltaT);
    }


    touchData->deltaPitchPos = a*(touchData->deltaPitchPos + vibratoAmplitude * deltaPos);
  }

  {
    Vector2d dir = touchData->deltaPitchPos;
    Lattice::changeToBasis(&dir, &lattice.largeGen1, &lattice.largeGen2);
    Lattice::changeToBasis(&dir, &generatorVec, &periodVec);
    touchData->deltaCents = temperament.genCents*dir(0) + temperament.periodCents*dir(1);
  }

  touchData->cents = touchData->origCents + touchData->deltaCents;
  touchData->prevVibratoPos = touchData->pos;
  instrument.updateNoteFreq(touchData->noteID, temperament.frequencyFromCents(touchData->cents));


  for (int i = 0; i < touchDataArray.size(); i++)
  {
    TouchData* td = &touchDataArray[i];
    if (td->origCents == touchData->origCents && td != touchData)
    {
      td->cents = touchData->cents;
    }
  }
}

unsigned int LatticeView::updateTouchTimes()
{
	QueryPerformanceFrequency(&Freq);
	LARGE_INTEGER prevEnd = EndingTime;
	QueryPerformanceCounter(&EndingTime);

	deltaT = (EndingTime.QuadPart - StartingTime.QuadPart) *1.0/ Freq.QuadPart;
	if (deltaT > 0.01)
		StartingTime = prevEnd;
	else
		deltaT = 0;
	
	unsigned int numActive = 0;
	unsigned long long time = GetTickCount64();
	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* td = &touchDataArray[i];
		if (td->phase == TOUCH_UP)
		{
			//if ( td->killTime < 10000)
			//{
			//	stopPlayingNote(td);
			//	td->clear();
   //         }
			//else
			if (playMusic)
			{
				if ((time - td->killTime) >= touchLingerDuration)
				{
				  stopPlayingNote(td);
					td->clear();
				}
				else
				{
					numActive++;
				}
			}
			else
			{
				if ((time - td->killTime) >= 200 && !td->permanent) // These linger to detect double taps
				{
					td->clear();
				}
				else
				{
					numActive++;
				}
			}
		}
		else if (td->phase == TOUCH_MOVE && afterTouchMode == VIBRATO)
		{
			updateVibratoFreq(td);
		}
    else if(afterTouchMode == BEND)
    {
      updateBendFreq(td);
    }
	}


	for (int i = notes.size() - 1; i >= 0; i--)
	{
		Note& note = notes[i];

		if (note.startTime > 0 && time > note.startTime)
		{
			note.startTime = 0;
			note.noteID = instrument.playNote(note.freq);
		}
		else if(time > note.endTime)
		{
			instrument.stopNote(note.noteID);
			notes.erase(notes.begin() + i);
		}
	}

	return numActive;
}

void LatticeView::startKillTimer(TouchData* touchData)
{
	touchData->killTime = GetTickCount64();
//	stopPlayingNote(touchData);
}

void LatticeView::stopPlayingAllNotes()
{
	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* touchData = &touchDataArray[i];
		int noteID = touchData->noteID;
		if (noteID != -1)
		  instrument.stopNote(noteID);

		touchData->clear();
	}
}

void LatticeView::stopPlayingNote(TouchData* touchData)
{
	int noteID = touchData->noteID;

	//PRINT(_T("stopPlayingNote : noteID =  %d\n"), noteID);

	touchData->phase == NOTHING;

	if (noteID != -1)
	{
		int count = 0;
		for (int i = 0; i < touchDataArray.size(); i++)
		{
			TouchData* td = &touchDataArray[i];
			if (td->noteID == touchData->noteID && td != touchData)
				count++;
		}

		if (count == 0)
		{
			instrument.stopNote(noteID);
			noteID = -1;

			if(notesToMatch.size() == 0)
      {
        currentSequance.clear();
		    playRandomNotes();
      }
		}
	}
	else
	{
		//    NSLog(@"touchUp with no touch down.");
	}

	touchData->clear();
}

void LatticeView::playRandomNotes()
{
  if(currentSequance.size() == 0 && sequenceLength > 0)
  {
	  RECT bounds;
	  bounds.left = MARGIN / m_dpi * 96.0f / scale;
	  bounds.right = (m_d3dRenderTargetSize.Width - MARGIN) / m_dpi * 96.0f / scale;
	  bounds.top = MARGIN / m_dpi * 96.0f / scale;
	  bounds.bottom = (m_d3dRenderTargetSize.Height - MARGIN) / m_dpi * 96.0f / scale;
	
	  random_device rd;   // non-deterministic generator  
	  mt19937 gen(rd());  // to seed mersenne twister
	  std::uniform_int_distribution<int> randX(bounds.left, bounds.right);
	  std::uniform_int_distribution<int> randY(bounds.top, bounds.bottom);
	  std::uniform_int_distribution<int> randPer(-octavesBelow, octavesAbove);

	  if (instruments.size())
	  {
		  std::uniform_int_distribution<int> randInst(0, instruments.size()-1);
		  instrument.midi_program = instruments[randInst(gen)];
		  instrument.updateMidiProgram();
	  }

	  unsigned long long startTime = GetTickCount64();
	  startTime += 200;
	  int dur = 500;
	  for (int i = 0; i < sequenceLength; i++)
	  {
		  NoteName* name;
		  double cents;
		  int per;
      int count = 0;
		  do
		  {
        count++;
        if(count > 100)
          return;

			  POINT pos;
			  pos.x = randX(gen);
			  pos.y = randY(gen);
			  double originXScreen = originXToScreen;
			  double originYScreen = originYToScreen;

			  double x = pos.x + originXScreen;
			  double y = pos.y + originYScreen;

			  lattice.getCell(&x, &y);
			  lattice.latticeToCartesian(&x, &y);

			  Vector2d p;
			  p(0) = x;
			  p(1) = y;

			  getGenPer(p);

			  per = randPer(gen);
			  cents = temperament.getCents(p(0), p(1), duplicateRatios) + 1200 * per;
			  name = temperament.borrowNoteName(lrint(p(0)), lrint(p(1)), duplicateRatios,  showName,  scientificPitchNotation,  use53TETNotation);
		  } while (name->name.size() == 0);

	  //	int midiNote = instrument.getMidiNoteFromFreq(temperament.frequencyFromCents(cents));    // MIDI note-on message: Key number (60 = middle C) 69 = A4

		  Note note;
		  note.freq = temperament.frequencyFromCents(cents);
		  note.cents = cents;
		  note.startTime = startTime;
		  startTime += duration;
		  note.endTime = startTime;
		  note.noteToMatch = false;
      currentSequance.push_back(note);
      note.noteToMatch = true;
      notes.push_back(note);

      notesToMatch.insert(notesToMatch.begin(), note);
	  }
  }
  else
  {
	  if (currentSequance.size() > 0 && notes.size() == 0)
	  {
		  unsigned long long oldStartTime = currentSequance[0].startTime;
		  unsigned long long startTime = GetTickCount64();
		  for (int i = 0; i < currentSequance.size(); i++)
		  {
			  Note note = currentSequance[i];
			  note.startTime = startTime + note.startTime - oldStartTime;
			  note.endTime = startTime + note.endTime - oldStartTime;
			  notes.push_back(note);
		  }
	  }
  }
}

TouchData* LatticeView::newTouchDown(PointerEventArgs^ args)
{
	Vector2d pos;
	pos(0) = args->CurrentPoint->Position.X/scale;
	pos(1) = args->CurrentPoint->Position.Y/scale;

	double originXScreen = originXToScreen;
	double originYScreen = originYToScreen;

	double x = pos(0) + originXScreen;
	double y = pos(1) + originYScreen;

	Lattice::cartesianToLattice(&x, &y, &lattice.largeGen1, &lattice.largeGen2);

	TouchData* touchData = 0;
  if(oneTouchEdit && !playMusic)
  {
    for (int i = 0; i < touchDataArray.size(); i++)
    {
      TouchData* td = &touchDataArray[i];
      double dx = td->pos(0) - pos(0);
      double dy = td->pos(1) - pos(1);
      if ((dx*dx + dy*dy) < touchDiameter*touchDiameter && td->permanent)
      {
        touchData = td;
        touchData->permanent = false;
      }
    }

    if (!touchData)
    {
      touchData = newTouchData(args->CurrentPoint->PointerId);
      touchData->permanent = true;
    }

    touchData->dwID = args->CurrentPoint->PointerId;
  }
  else
  {
    touchData = newTouchData(args->CurrentPoint->PointerId);
  }
  

	if (touchData->phase != NOTHING)  // This can happen with the mouse pointer
	{
		if (touchData->noteID != -1)
		{
			stopPlayingNote(touchData);
			touchData->clear();
			touchData->dwID = args->CurrentPoint->PointerId;
		}
	}

	touchData->latticeCoordsStart = Vector2d(x, y);
	touchData->posStart = pos;
	touchData->pos = pos;
	touchData->prevMovePos = pos;
	touchData->prevVibratoPos = pos;
//	touchData->prevTime = 0;
//	touchData->time = args->CurrentPoint->Timestamp;
	touchData->phase = TOUCH_DOWN;

	if (lattice.keyMode == CUSTOM && !playMusic)
	{
		double x = touchData->pos(0) + originXScreen;
		double y = touchData->pos(1) + originYScreen;

    double cellX = temperament.rootGens;
    double cellY = temperament.rootPers;
    Vector2d genVec = generatorVec;
    Vector2d perVec = periodVec;
    Lattice::latticeToCartesian(genVec, &lattice.largeGen1, &lattice.largeGen2);
    Lattice::latticeToCartesian(perVec, &lattice.largeGen1, &lattice.largeGen2);
    Lattice::latticeToCartesian(&cellX, &cellY, &genVec, &perVec);

    x -= cellX;
    y -= cellY;

		touchData->bounderyPoint = lattice.getBounderyPoint(x, y);
		if (touchData->bounderyPoint)
		{
			touchData->bounderyPointStart = *touchData->bounderyPoint;
			invalidateCellPath();
      touchData->permanent = false;
		}

		bool text = false;
		for (int i = 0; i < lattice.textCustomLoc.size(); i++)
		{
			if (touchData->bounderyPoint == &lattice.textCustomLoc[i])
      {
				text = true;
        touchData->permanent = false;
      }
		}

		if (text) // check for double tap. if so, create new text
		{
			for (int i = 0; i < touchDataArray.size(); i++)
			{
				TouchData* td = &touchDataArray[i];
				double dx = td->pos(0) - touchData->pos(0);
				double dy = td->pos(1) - touchData->pos(1);
				if (td->phase == TOUCH_UP && (dx*dx + dy*dy) < touchDiameter*touchDiameter)
				{
					lattice.textCustomLoc.push_back(*touchData->bounderyPoint);
					touchData->bounderyPoint = &lattice.textCustomLoc[lattice.textCustomLoc.size()-1];
          touchData->permanent = false;
					break;
				}
			}
		}
	} 
	else
	{
		touchData->bounderyPoint = 0;
	}

	return touchData;
}

void LatticeView::touchUp(PointerEventArgs^ args)
{
	TouchData* touchData = getTouchData(args->CurrentPoint->PointerId);
	if (!touchData)
		return;

	if (playMusic)
	{
		touchData->phase = TOUCH_UP;
		startKillTimer(touchData);
	}
	else
	{
		if (lattice.keyMode == CUSTOM && touchData->bounderyPoint)
		{
			if(lattice.maybeRemoveBounderyPoint(touchData->bounderyPoint))
				invalidateCellPath();
		}


		///////////////////////////////////////////////////////
		// using kill timmer for boundery points causes wied effect where two points get deleted. Now sure why
		// Anyhow, for now, lets just use kill timmer for touches which have a textposition
		bool isText = false;
		for (int i = 0; i < lattice.textCustomLoc.size(); i++)
		{
			if (touchData->bounderyPoint == &lattice.textCustomLoc[i])
			{
				isText = true;
				break;
			}
		}

		if (isText)
		{
			// use kill timmer instead to detect for double tap.
			touchData->phase = TOUCH_UP;
			startKillTimer(touchData);
		}
		else if(!touchData->permanent)
		{
			touchData->clear();  // youse kill timmer instead to detect for double tap.
		}
    else
    {
      touchData->dwID = UINT_MAX;
    }
    ///////////////////////////////////////



		if (touchData == intervalOriginTouch && !touchData->permanent)
		{
			intervalOriginTouch = 0;
			intervalOrigin = Vector2d(0, 0);
		}
			
		touchBegin_latticeGen1 = Vector2d(lattice.largeGen1(0), lattice.largeGen1(1));
		touchBegin_latticeGen2 = Vector2d(lattice.largeGen2(0), lattice.largeGen2(1));
	}
}

void LatticeView::touchDown(PointerEventArgs^ args)
{
  double width = m_d3dRenderTargetSize.Width;
	double height = m_d3dRenderTargetSize.Height;

	double originXScreen = originXToScreen;
	double originYScreen = originYToScreen;

	double tolerance = 50;
	if (playMusic)
	{
		TouchData* touchData = newTouchDown(args);
		if (touchData->pos(0) < 20 && touchData->pos(1) < 20)
		{
//				ShowWindow(this->  enharmonic->hTemperamentDlg, SW_SHOW);
		}
		else
		//turn off drag from edge causing dragging screen
		//      if(pos.x > 10 && pos.x < width - 10 && pos.y > 10 && pos.y < height - 10)
		{
			startPlayingNote(touchData);
		}

		////turn off drag from edge causing dragging screen
		//if (false) //if( || pos.x <= 10 || pos.x >= width - 10)
		//{
		//	if (!touchData->dragHorizontal && !touchData->dragVertical)
		//	{
		//	  //maybeBeginDrag(ti->dwID);
		//    }

		//	touchData->dragHorizontal = true;

		//	if (touchData->posStart(1) <= 100 || touchData->posStart(1) >= height - 100)
		//	{
		//		touchData->dragVertical = true;
		//	}
		//}

		////turn off drag from edge causing dragging screen
		//if (false) //if(pos.y <= 1 || pos.y >= height - 7)
		//{
		//	if (!touchData->dragHorizontal && !touchData->dragVertical)
		//	{
		//		//maybeBeginDrag(ti->dwID);
		//	}

		//	touchData->dragVertical = true;

		//	if (touchData->posStart(0) <= 100 || touchData->posStart(0) >= width - 100)
		//	{
		//		touchData->dragHorizontal = true;
		//	}
		//}
	

//		InvalidateRect(hwnd, NULL, FALSE);
	}
	else
	{
		TouchData* touchData = newTouchDown(args);

		int numDown = 0;
		for (int i = 0; i < touchDataArray.size();i++)
		{
			if (touchDataArray[i].phase == TOUCH_DOWN || touchDataArray[i].phase == TOUCH_MOVE)
				numDown++;
		}

    	if (numDown == 1 && touchData->bounderyPoint == 0)
		{
			double x = touchData->pos(0) + originXScreen;
			double y = touchData->pos(1) + originYScreen;

	        lattice.getCell(&x, &y);
			lattice.latticeToCartesian(&x, &y);
			Lattice::cartesianToLattice(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
			intervalOrigin = Vector2d(x, y);
			intervalOriginTouch = touchData;
		}
		else if (numDown == 2 && intervalOriginTouch)
		{
		/*	if (touchData == intervalOriginTouch)
			{
				ti = &pInputs[1];
				touchData = getTouchData(ti->dwID);
			}
			*/

			double x = intervalOrigin(0);
			double y = intervalOrigin(1);
			Lattice::latticeToCartesian(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
			x -= originXScreen;
			y -= originYScreen;

			double x1, y1, x2, y2, x3, y3;
			interval1ToCartesian(x1, y1);
			interval2ToCartesian(x2, y2);
			keyDupToCartesian(x3, y3);

			x1 += x;
			y1 += y;
			x2 += x;
			y2 += y;
			x3 += x;
			y3 += y;
			if (touchData->pos(0) > x1 - tolerance
				&& touchData->pos(0) < x1 + tolerance
				&& touchData->pos(1) > y1 - tolerance
				&& touchData->pos(1) < y1 + tolerance)
			{
				touchData->intervalVector = &generatorVec;
        touchData->dragVector = *touchData->intervalVector;
        lattice.latticeToCartesian(touchData->dragVector, &lattice.largeGen1, &lattice.largeGen2);
			}
			else if( touchData->pos(0)  > x2 - tolerance
				&& touchData->pos(0)  < x2 + tolerance
				&& touchData->pos(1) > y2 - tolerance
				&&touchData->pos(1) < y2 + tolerance)
			{
				touchData->intervalVector = &periodVec;
        touchData->dragVector = *touchData->intervalVector;
        lattice.latticeToCartesian(touchData->dragVector, &lattice.largeGen1, &lattice.largeGen2);
			}
			else if (dupKeys
				&& touchData->pos(0)  > x3 - tolerance
				&& touchData->pos(0)  < x3 + tolerance
				&& touchData->pos(1) > y3 - tolerance
				&&touchData->pos(1) < y3 + tolerance)
			{
				touchData->intervalVector = &keyDuplicateVec;
        touchData->dragVector = *touchData->intervalVector;
        lattice.latticeToCartesian(touchData->dragVector, &lattice.largeGen1, &lattice.largeGen2);
			}
		}
		//   else
		//   {
		//     intervalOrigin = nil;
		//   }

		touchBegin_latticeGen1 = lattice.largeGen1;
		touchBegin_latticeGen2 = lattice.largeGen2;
	}
}


void LatticeView::touchMoved(PointerEventArgs^ args)
{
	TouchData* touchData = getTouchData(args->CurrentPoint->PointerId);
	if (!touchData)
		return;

	if (touchData->phase != TOUCH_DOWN && touchData->phase != TOUCH_MOVE)
		return;

	touchData->updateTouchData(args, scale);

	touchData->phase = TOUCH_MOVE;

	if (playMusic)
	{
		{
			double D2 = touchDiameter*touchDiameter;
			double V2 = D2 / (touchDiameterEscapeTime*touchDiameterEscapeTime);  //velocity^2 needed to move a distance of 2*radius in time radiusEscapeTime

			double dx = touchData->pos(0) - touchData->prevMovePos(0);
			double dy = touchData->pos(1) - touchData->prevMovePos(1);
			double d2 = dx*dx + dy*dy;
			double t = touchData->time - touchData->prevTime;

			// Check for phantum movment. If the distance bewteen teh new and old position is greater than 2*radius, then a it is likley
			// that a touch up and touch down event were probobly mistaken for a movment. We need to do a velocity test to make sure.
			// If the new possition is within 2*radius from the old one, then whether it is a new point or not is largly irrelivant since it will be mapped to the same note (what about vibrado)?
			if (touchDiameterEscapeTime > 0 && d2 > D2 && d2 / (t*t) > V2)
			{
				touchData->phase = TOUCH_UP;
				touchData->pos = touchData->prevMovePos;
				startKillTimer(touchData);

				touchData = newTouchDown(args);
				startPlayingNote(touchData);
			}
      else if (dragKeyboard)
      {
        drag(touchData);
      }
			else if (afterTouchMode == BEND)
			{
				updateBend(touchData);
			}
			else if (afterTouchMode == GLISSANDO)
			{
				updateGlissando(touchData);
			}


			//for (id key in _touchData)
			//{
			//	TouchData* touchData = [_touchData objectForKey : key];
			//	if (touchData.speed > self.maxSpeed)
			//	{
			//		double dx = newPoint(0) - touchData.posStart(0);
			//		double dy = newPoint(1) - touchData.posStart(1);

			//		double dis = sqrtf(dx*dx + dy*dy);

			//		NSLog(@"speed = %.2f : dis = %2.f", touchData.speed, dis);

			//		UITouch* touch = touchData.touch;
			//		[self stopPlayingNote : touch];
			//		[self startPlayingNote : touch];
			//	}
			//}

		}
	}
	else
	{
//    if(oneTouchEdit && touchData->bounderyPoint == 0)
//      touchData->permanent = true;
    if((touchData->posStart - touchData->pos).norm() > 20)
      touchData->permanent = false;

		switch (this->mode)
		{
				case EDIT_ROTATE:
				case EDIT_ROTATE_SCALE:
				case EDIT_SCALE:
				case EDIT_SQUEEZE:
				case EDIT_SHEAR:
					TouchData* tds[3] = { 0, 0, 0 };
					int cInputs = 0;
		
					for (int i = 0; i < touchDataArray.size(); i++)
					{
						TouchData* td = &touchDataArray[i];
						if (td->phase != NOTHING && td->phase != TOUCH_UP)
						{
							if (cInputs >= 3)
							{
								cInputs = 0;
								break;
							}
							
							tds[cInputs] = &touchDataArray[i];
							cInputs++;
						}
					}

					if (cInputs == 1)
					{
						if (lattice.keyMode == CUSTOM && touchData->bounderyPoint)
						{
							Vector2d Dp = Vector2d(touchData->pos(0) - touchData->posStart(0), touchData->pos(1) - touchData->posStart(1));
							Lattice::cartesianToLattice(&Dp(0), &Dp(1), &lattice.largeGen2, &lattice.largeGen1);
							*touchData->bounderyPoint = touchData->bounderyPointStart + Dp;

							invalidateCellPath();
						}
					  else
						{
							drag(touchData);
						}
					}
					else if (cInputs == 2)
					{
						TouchData *touchData1 = 0;
						TouchData *touchData2 = 0;
						for (int i = 0; i < cInputs; i++)
						{
							TouchData* touchData = tds[i];

							if (touchData->intervalVector != 0)
							{
								touchData2 = touchData;
							}
							else
							{
								touchData1 = touchData;
							}
						}

						if (touchData2 != 0)
						{
							double originXScreen = touchData1->latticeCoordsStart(0)*lattice.largeGen1(0) + touchData1->latticeCoordsStart(1)*lattice.largeGen2(0) - touchData1->pos(0);
							double originYScreen = touchData1->latticeCoordsStart(0)*lattice.largeGen1(1) + touchData1->latticeCoordsStart(1)*lattice.largeGen2(1) - touchData1->pos(1);

							{
								critical_section::scoped_lock lock(this->criticalSection);
								originX = screenToOriginX;
								originY = screenToOriginY;
							}

							double x1 = intervalOrigin(0);
							double y1 = intervalOrigin(1);
							Lattice::latticeToCartesian(&x1, &y1, &lattice.largeGen1, &lattice.largeGen2);

							double x2 = originXScreen + touchData2->pos(0);
							double y2 = originYScreen + touchData2->pos(1);

							double x3 = x2 - x1;
							double y3 = y2 - y1;

							touchData2->dragVector(0) = x3;
							touchData2->dragVector(1) = y3;
              
              double xPrime = intervalOrigin(0);
              double yPrime = intervalOrigin(1);
              Lattice::latticeToCartesian(&xPrime, &yPrime, &lattice.largeGen1, &lattice.largeGen2);
              xPrime -= originXScreen;
              yPrime -= originYScreen;
  
              Vector2d v = intervalOrigin;
							Lattice::changeToBasis(&v, &generatorVec, &periodVec);
							//// now we have invervalOriginexpressed as a sum of periodVec and generatorVec

							lattice.getCell(&x3, &y3);
							lattice.latticeToCartesian(&x3, &y3);
							Lattice::cartesianToLattice(&x3, &y3, &lattice.largeGen1, &lattice.largeGen2);
              Vector2d newVec;
              newVec(0) = round(x3);
              newVec(1) = round(y3);
              Vector2d* otherGen = touchData2->intervalVector == &generatorVec ? &periodVec : &generatorVec;
           //   PRINT(L"cross = %f/n", abs(cross(newVec, *otherGen)));
              if(newVec.norm() > .001 && (abs(cross(newVec, *otherGen)) > .001))
              {
							  (*touchData2->intervalVector) = newVec;
              }

							// Now either peridoVec or generatorVec have changed. we want intervalOrigin to be the same sum of
							// periodVec and generatorVec
							intervalOrigin(0) = v(0)*generatorVec(0) + v(1)*periodVec(0);
							intervalOrigin(1) = v(0)*generatorVec(1) + v(1)*periodVec(1);
              
              originXScreen = intervalOrigin(0);
              originYScreen = intervalOrigin(1);
              Lattice::latticeToCartesian(&originXScreen, &originYScreen, &lattice.largeGen1, &lattice.largeGen2);
              originXScreen -= xPrime;
              originYScreen -= yPrime;

			  {
				  critical_section::scoped_lock lock(this->criticalSection);
				  originX = screenToOriginX;
				  originY = screenToOriginY;
			  }

              {
                double x = touchData1->pos(0) + originXScreen;
                double y = touchData1->pos(1) + originYScreen;
                Lattice::cartesianToLattice(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
                touchData1->latticeCoordsStart = Vector2d(x, y);
              }

              {
                double x = touchData2->pos(0) + originXScreen;
                double y = touchData2->pos(1) + originYScreen;
                Lattice::cartesianToLattice(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
                touchData2->latticeCoordsStart = Vector2d(x, y);
              }
						}
						else if (this->twoTouchEdit)
						{
							if (this->mode == EDIT_ROTATE_SCALE)
							{
								double Q, R, S, T, n1, n2, m1, m2, A, B, C, D, s, c;

								TouchData* touchData1 = tds[0];
								TouchData* touchData2 = tds[1];

								n1 = touchData1->latticeCoordsStart(0);
								m1 = touchData1->latticeCoordsStart(1);
								n2 = touchData2->latticeCoordsStart(0);
								m2 = touchData2->latticeCoordsStart(1);

								A = n1*touchBegin_latticeGen1(0) + m1*touchBegin_latticeGen2(0);
								B = n1*touchBegin_latticeGen1(1) + m1*touchBegin_latticeGen2(1);
								C = n2*touchBegin_latticeGen1(0) + m2*touchBegin_latticeGen2(0);
								D = n2*touchBegin_latticeGen1(1) + m2*touchBegin_latticeGen2(1);

								Q = touchData1->pos(0);
								R = touchData2->pos(0);
								S = touchData1->pos(1);
								T = touchData2->pos(1);

								double denom = (A*A - 2 * A*C + B*B - 2 * B*D + C*C + D*D);
                if (abs(denom) < .01)
                  break;

								c = -(-A*Q + A*R - B*S + B*T + C*Q - C*R + D*S - D*T) / denom;
								s = -(-A*S + A*T + B*Q - B*R + C*S - C*T - D*Q + D*R) / denom;

								//lattice.largeGen1(0) = touchBegin_latticeGen1(0)*c - touchBegin_latticeGen1(1)*s;
								//lattice.largeGen2(0) = touchBegin_latticeGen2(0)*c - touchBegin_latticeGen2(1)*s;
								//lattice.largeGen1(1) = touchBegin_latticeGen1(0)*s + touchBegin_latticeGen1(1)*c;
								//lattice.largeGen2(1) = touchBegin_latticeGen2(0)*s + touchBegin_latticeGen2(1)*c;

								double x1 = touchBegin_latticeGen1(0)*c - touchBegin_latticeGen1(1)*s;
								double x2 = touchBegin_latticeGen2(0)*c - touchBegin_latticeGen2(1)*s;
								double y1 = touchBegin_latticeGen1(0)*s + touchBegin_latticeGen1(1)*c;
								double y2 = touchBegin_latticeGen2(0)*s + touchBegin_latticeGen2(1)*c;

								{
									double originXScreen = (A*A*(-R) + A*C*Q + A*C*R - A*D*S + A*D*T - B*B*R + B*C*S - B*C*T + B*D*Q + B*D*R - C*C*Q - D*D*Q) / denom;
									double originYScreen = (A*A*(-T) + A*C*S + A*C*T + A*D*Q - A*D*R - B*B*T - B*C*Q + B*C*R + B*D*S + B*D*T - C*C*S - D*D*S) / denom;
									critical_section::scoped_lock lock(this->criticalSection);
									if (maybeUpdateLattice(x1, y1, x2, y2))
									{
										originX = screenToOriginX;
										originY = screenToOriginY;
									}
								}
							}
							else if (this->mode == EDIT_ROTATE)
							{
								double Q, R, S, T, n1, n2, m1, m2, A, B, C, D, s, c;

								TouchData* touchData1 = tds[0];
								TouchData* touchData2 = tds[1];

								n1 = touchData1->latticeCoordsStart(0);
								m1 = touchData1->latticeCoordsStart(1);
								n2 = touchData2->latticeCoordsStart(0);
								m2 = touchData2->latticeCoordsStart(1);

								A = n1*touchBegin_latticeGen1(0) + m1*touchBegin_latticeGen2(0);
								B = n1*touchBegin_latticeGen1(1) + m1*touchBegin_latticeGen2(1);
								C = n2*touchBegin_latticeGen1(0) + m2*touchBegin_latticeGen2(0);
								D = n2*touchBegin_latticeGen1(1) + m2*touchBegin_latticeGen2(1);

								Q = touchData1->pos(0);
								R = touchData2->pos(0);
								S = touchData1->pos(1);
								T = touchData2->pos(1);

								double denom = (A*A - 2 * A*C + B*B - 2 * B*D + C*C + D*D);
                if (abs(denom) < .01)
                  break;

								c = -(-A*Q + A*R - B*S + B*T + C*Q - C*R + D*S - D*T) / denom;
								s = -(-A*S + A*T + B*Q - B*R + C*S - C*T - D*Q + D*R) / denom;

								double norm = sqrt(c*c + s*s);
								c /= norm;
								s /= norm;

								//lattice.largeGen1(0) = touchBegin_latticeGen1(0)*c - touchBegin_latticeGen1(1)*s;
								//lattice.largeGen2(0) = touchBegin_latticeGen2(0)*c - touchBegin_latticeGen2(1)*s;
								//lattice.largeGen1(1) = touchBegin_latticeGen1(0)*s + touchBegin_latticeGen1(1)*c;
								//lattice.largeGen2(1) = touchBegin_latticeGen2(0)*s + touchBegin_latticeGen2(1)*c;

								double x1 = touchBegin_latticeGen1(0)*c - touchBegin_latticeGen1(1)*s;
								double x2 = touchBegin_latticeGen2(0)*c - touchBegin_latticeGen2(1)*s;
								double y1 = touchBegin_latticeGen1(0)*s + touchBegin_latticeGen1(1)*c;
								double y2 = touchBegin_latticeGen2(0)*s + touchBegin_latticeGen2(1)*c;

								{
									critical_section::scoped_lock lock(this->criticalSection);
									if (maybeUpdateLattice(x1, y1, x2, y2))
									{
										double originXScreen = ((n1 + n2)*lattice.largeGen1(0) + (m1 + m2)*lattice.largeGen2(0) - (Q + R)) / 2;
										double originYScreen = ((n1 + n2)*lattice.largeGen1(1) + (m1 + m2)*lattice.largeGen2(1) - (S + T)) / 2;
										originX = screenToOriginX;
										originY = screenToOriginY;
									}
								}
							}
							else if (this->mode == EDIT_SCALE)
							{
								double Q, R, S, T, n1, n2, m1, m2, A, B, C, D, s, c;

								TouchData* touchData1 = tds[0];
								TouchData* touchData2 = tds[1];

								n1 = touchData1->latticeCoordsStart(0);
								m1 = touchData1->latticeCoordsStart(1);
								n2 = touchData2->latticeCoordsStart(0);
								m2 = touchData2->latticeCoordsStart(1);

								A = n1*touchBegin_latticeGen1(0) + m1*touchBegin_latticeGen2(0);
								B = n1*touchBegin_latticeGen1(1) + m1*touchBegin_latticeGen2(1);
								C = n2*touchBegin_latticeGen1(0) + m2*touchBegin_latticeGen2(0);
								D = n2*touchBegin_latticeGen1(1) + m2*touchBegin_latticeGen2(1);

								Q = touchData1->pos(0);
								R = touchData2->pos(0);
								S = touchData1->pos(1);
								T = touchData2->pos(1);

								double denom = (A*A - 2 * A*C + B*B - 2 * B*D + C*C + D*D);
                if (abs(denom) < .01)
                  break;

								c = -(-A*Q + A*R - B*S + B*T + C*Q - C*R + D*S - D*T) / denom;
								s = -(-A*S + A*T + B*Q - B*R + C*S - C*T - D*Q + D*R) / denom;
								double norm = sqrt(c*c + s*s);

								//lattice.largeGen1(0) = touchBegin_latticeGen1(0)*norm;
								//lattice.largeGen2(0) = touchBegin_latticeGen2(0)*norm;
								//lattice.largeGen1(1) = touchBegin_latticeGen1(1)*norm;
								//lattice.largeGen2(1) = touchBegin_latticeGen2(1)*norm;

								double x1 = touchBegin_latticeGen1(0)*norm;
								double x2 = touchBegin_latticeGen2(0)*norm;
								double y1 = touchBegin_latticeGen1(1)*norm;
								double y2 = touchBegin_latticeGen2(1)*norm;

								{
									critical_section::scoped_lock lock(this->criticalSection);
									if (maybeUpdateLattice(x1, y1, x2, y2))
									{
										double originXScreen = ((n1 + n2)*lattice.largeGen1(0) + (m1 + m2)*lattice.largeGen2(0) - (Q + R)) / 2;
										double originYScreen = ((n1 + n2)*lattice.largeGen1(1) + (m1 + m2)*lattice.largeGen2(1) - (S + T)) / 2;
										originX = screenToOriginX;
										originY = screenToOriginY;
									}
								}
							}
							else if (this->mode == EDIT_SQUEEZE)
							{
								double n1, n2, m1, m2, A, B, C, D, s;

								TouchData* touchData1 = tds[0];
								TouchData* touchData2 = tds[1];

								n1 = touchData1->latticeCoordsStart(0);
								m1 = touchData1->latticeCoordsStart(1);
								n2 = touchData2->latticeCoordsStart(0);
								m2 = touchData2->latticeCoordsStart(1);

								A = n1*touchBegin_latticeGen1(0) + m1*touchBegin_latticeGen2(0);
								B = n2*touchBegin_latticeGen1(0) + m2*touchBegin_latticeGen2(0);
								C = touchData1->pos(0);
								D = touchData2->pos(0);

                if (abs(A - B) < .001)
                   break;
              
								s = (C - D) / (A - B);
								double ox = (B*C - A*D) / (A - B);

								//lattice.largeGen1(0) = touchBegin_latticeGen1(0)*s;
								//lattice.largeGen2(0) = touchBegin_latticeGen2(0)*s;
                
                if(s<.01)
                  break;

								double x1 = touchBegin_latticeGen1(0)*s;
								double x2 = touchBegin_latticeGen2(0)*s;

								A = n1*touchBegin_latticeGen1(1) + m1*touchBegin_latticeGen2(1);
								B = n2*touchBegin_latticeGen1(1) + m2*touchBegin_latticeGen2(1);
								C = touchData1->pos(1);
								D = touchData2->pos(1);

								s = (C - D) / (A - B);
                if (abs(s)<.01)
                  break;

								double oy = (B*C - A*D) / (A - B);

								//lattice.largeGen1(1) = touchBegin_latticeGen1(1)*s;
								//lattice.largeGen2(1) = touchBegin_latticeGen2(1)*s;

								double y1 = touchBegin_latticeGen1(1)*s;
								double y2 = touchBegin_latticeGen2(1)*s;

								{
									critical_section::scoped_lock lock(this->criticalSection);
									if (maybeUpdateLattice(x1, y1, x2, y2))
									{
										double originXScreen = ox;
										double originYScreen = oy;
										originX = screenToOriginX;
										originY = screenToOriginY;
									}
								}
							}
							else if (this->mode == EDIT_SHEAR)
							{
								double x, y, z, n1, n2, n3, m1, m2, m3, c1, c2, c3;

								TouchData* touchData1 = tds[0];
								TouchData* touchData2 = tds[1];

								n1 = touchData1->latticeCoordsStart(0);
								m1 = touchData1->latticeCoordsStart(1);
								n2 = touchData2->latticeCoordsStart(0);
								m2 = touchData2->latticeCoordsStart(1);

								n3 = n1 + 1;
								m3 = m1;

								c1 = touchData1->pos(0);
								c2 = touchData2->pos(0);
								c3 = c1 + touchBegin_latticeGen1(0);

								double denom = (-m2*n1 + m3*n1 - m3*n2 + m2*n3 + m1*n2 - m1*n3);
                if(abs(denom) < .01)
                  break;

								x = -(c1*m3*n2 - c1*m2*n3 - c2*m3*n1 + c2*m1*n3 + c3*m2*n1 - c3*m1*n2) / denom;
								y = -(c1*m2 - c1*m3 + c2*m3 - c2*m1 - c3*m2 + c3*m1) / denom;
								z = (c1*n2 - c1*n3 - c2*n1 + c2*n3 + c3*n1 - c3*n2) / denom;

								double x1 = y;
								double x2 = z;
								double ox = -x;

								c1 = touchData1->pos(1);
								c2 = touchData2->pos(1);
								c3 = c1 + touchBegin_latticeGen1(1);

								x = -(c1*m3*n2 - c1*m2*n3 - c2*m3*n1 + c2*m1*n3 + c3*m2*n1 - c3*m1*n2) / denom;
								y = -(c1*m2 - c1*m3 + c2*m3 - c2*m1 - c3*m2 + c3*m1) / denom;
								z = (c1*n2 - c1*n3 - c2*n1 + c2*n3 + c3*n1 - c3*n2) / denom;

								double y1 = y;
								double y2 = z;
								double oy = -x;

								{
									critical_section::scoped_lock lock(this->criticalSection);
									if (maybeUpdateLattice(x1, y1, x2, y2))
									{
										double originXScreen = ox;
										double originYScreen = oy;
										originX = screenToOriginX;
										originY = screenToOriginY;
									}
								}
							}
						}
					}
					else if (cInputs == 3 && this->threeTouchEdit)
					{
						double x, y, z, n1, n2, n3, m1, m2, m3, c1, c2, c3;

						TouchData* touchData1 = tds[0];
						TouchData* touchData2 = tds[1];
						TouchData* touchData3 = tds[2];

						n1 = touchData1->latticeCoordsStart(0);
						m1 = touchData1->latticeCoordsStart(1);
						n2 = touchData2->latticeCoordsStart(0);
						m2 = touchData2->latticeCoordsStart(1);
						n3 = touchData3->latticeCoordsStart(0);
						m3 = touchData3->latticeCoordsStart(1);

						c1 = touchData1->pos(0);
						c2 = touchData2->pos(0);
						c3 = touchData3->pos(0);

						double denom = (-m2*n1 + m3*n1 - m3*n2 + m2*n3 + m1*n2 - m1*n3);
            if (abs(denom) < .01)
              break;

						x = -(c1*m3*n2 - c1*m2*n3 - c2*m3*n1 + c2*m1*n3 + c3*m2*n1 - c3*m1*n2) / denom;
						y = -(c1*m2 - c1*m3 + c2*m3 - c2*m1 - c3*m2 + c3*m1) / denom;
						z = (c1*n2 - c1*n3 - c2*n1 + c2*n3 + c3*n1 - c3*n2) / denom;

						double x1 = y;
						double x2 = z;
						double ox = -x;

						c1 = touchData1->pos(1);
						c2 = touchData2->pos(1);
						c3 = touchData3->pos(1);

						x = -(c1*m3*n2 - c1*m2*n3 - c2*m3*n1 + c2*m1*n3 + c3*m2*n1 - c3*m1*n2) / denom;
						y = -(c1*m2 - c1*m3 + c2*m3 - c2*m1 - c3*m2 + c3*m1) / denom;
						z = (c1*n2 - c1*n3 - c2*n1 + c2*n3 + c3*n1 - c3*n2) / denom;

						double y1 = y;
						double y2 = z;
						double oy = -x;

						{
							critical_section::scoped_lock lock(this->criticalSection);
							if (maybeUpdateLattice(x1, y1, x2, y2))
							{
								double originXScreen = ox;
								double originYScreen = oy;
								originX = screenToOriginX;
								originY = screenToOriginY;
							}
						}
					}
			
					break;
		}
	}
}


bool LatticeView::maybeUpdateLattice(double x1, double y1, double x2, double y2)
{
  Vector2d v1;
  v1 << x1,y1;
  Vector2d v2;
  v2 << x2,y2;
  return maybeUpdateLattice(v1,v2);
}

bool LatticeView::maybeUpdateLattice(Vector2d& v1, Vector2d& v2)
{
  Vector2d v3 = v2-v2.dot(v1)/v1.norm()*v1/v1.norm();

  double minLength = 10;
  double length1 = v1.norm();
  if (v1.norm() < minLength || v3.norm() < minLength)
  {
    return false;
  }

	double area = abs(cross(v1,v3));
  double minArea = 1000;
  if (area < minArea)
  {
    double scale = sqrt(minArea / area);
    v1 *= scale;
    v2 *= scale;
  }

  largeGen1 = v1;
  largeGen2 = v2;
  invalidateCellPath();
  return true;
}

void LatticeView::drag(TouchData* touchData)
{
  critical_section::scoped_lock lock(criticalSection);
	latticeBitMapValid = false;
	double dx = 0;
	double dy = 0;
	double dxAvg = 0;
	double dyAvg = 0;
	int numPointers = 0;

	double dvxAvg = 0;
	double dvyAvg = 0;

	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* touchData = &touchDataArray[i];
		if(touchData->phase == TOUCH_MOVE)
		{
		//	if (touchData->speed < maxSpeed)
			{
				//dx = -touchData->pos(0) + touchData->prevMovePos(0);
				//dy = -touchData->pos(1) + touchData->prevMovePos(1);

				//dxAvg += dx;
				//dyAvg += dy;

				numPointers++;
			}
			//else
			//{
			//	PRINT(_T("didn't drag touchData.speed = %.02fms\n"), touchData->speed);
			//	continue;
			//}

			//dvxAvg += touchData->velocity(0);
			//dvyAvg += touchData->velocity(1);
		}
		//else //if(phase !=0)
		//{
		//	dx = -touchData->pos(0) + touchData->prevPos(0);
		//	dy = -touchData->pos(1) + touchData->prevPos(1);

		//	PRINT(_T("didn include touch : %d\n"), touchData->phase);
		////	PRINT(_T("didn include touch dx = %.02f, dy = %.02f\n"), dx, dy);
		//	numPointers++;
		//	continue;
		//}
	}

	if (numPointers == 0)
		return;


	dx = -touchData->pos(0) + touchData->prevMovePos(0);
	dy = -touchData->pos(1) + touchData->prevMovePos(1);

	dxAvg += dx;
	dyAvg += dy;

	dxAvg /= numPointers;
	dyAvg /= numPointers;

	double originXScreen = originXToScreen + dxAvg;
	double originYScreen = originYToScreen + dyAvg;
  
	originX = screenToOriginX;
	originY = screenToOriginY;  

	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* touchData = &touchDataArray[i];
		if (touchData->phase == TOUCH_MOVE || touchData->phase == TOUCH_DOWN)
		{
			touchData->cellPos(0) -= dxAvg;
			touchData->cellPos(1) -= dyAvg;
		}
	}
}


task<void> LatticeView::loadKeyboardAsync(StorageFile^ file)
{
  return create_task(FileIO::ReadLinesAsync(file)).then([this, file](IVector<Platform::String^>^ lines)
  {
    critical_section::scoped_lock lock(criticalSection);

    initialized = false;
    this->clearTouchData();

	  this->keyboardName = convertFileNameToString(file->Name->Data());
	  trimFileExtension(this->keyboardName);

	  Vector2d g1(600, 0);
	  Vector2d g2(0, 600);

	  dupKeys = false;
	  keyDuplicateVec = Vector2d::Zero();
   // scale = 1;
    textSize = 1;
    questions = 0;
    correct = 0;

    shiftPercent = 0;
    keySlope = 0;
    columns = 7;
    minEDO = 12;
    maxEDO = 72;
    EDO = 12;
    periodInCents = 1200;
    lockKeyPattern = false;
    dynamicTemperament = false;
    EDOs = false;

    notes.clear();
    notesToMatch.clear();
    currentSequance.clear();

	  lattice.textCustomLoc.clear();
	  lattice.cellVerticies1.clear();
	  lattice.cellVerticies2.clear();
	  lattice.cellVerticies3.clear();

    scientificPitchNotation = false;
    use53TETNotation = false;
    showName = true;
    showRatio = true;
    showCents = true;
    showDelta = true;
    playUnamedNotes = false;
    duplicateRatios = true;
	  for (unsigned int i = 0; i < lines->Size; i++)
	  {
		  Platform::String^ str = lines->GetAt(i);
		  if (str->Length() == 0)
			  continue;

		  // read an entire line into memory
		  wchar_t buf[MAX_CHARS_PER_LINE] = { 0 };
		  wcscpy_s(buf, str->Data());

    	  // parse the line into blank-delimited tokens
		  int n = 0; // a for-loop index

		  LPTSTR next_token = NULL;
		  LPTSTR token = wcstok_s(buf, _T(" "), &next_token); // first token
		  if (!token)
		  {
			  // do nothing
		  }
      else if (!wcscmp(token, _T("scientificPitchNotation")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"true"))
          scientificPitchNotation = true;
      }
      else if (!wcscmp(token, _T("use53TETNotation")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"true"))
          use53TETNotation = true;
      }
      else if (!wcscmp(token, _T("showName")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"false"))
          showName = false;
      }
      else if (!wcscmp(token, _T("showRatio")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"false"))
          showRatio = false;
      }
      else if (!wcscmp(token, _T("showCents")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"false"))
          showCents = false;
      }
      else if (!wcscmp(token, _T("showDelta")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"false"))
          showDelta = false;
      }
      else if (!wcscmp(token, _T("duplicateRatios")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"false"))
          duplicateRatios = false;
      }
      else if (!wcscmp(token, _T("playUnamedNotes")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"true"))
          playUnamedNotes = true;
      }
		  else if (!wcscmp(token, _T("name")))
		  {
			  wstring name;
			  for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
			  {
				  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
				  if (!token) break; // no more tokens

				  if (n > 1)
					  name += _T(" ");

				  name += token;
			  }
		  }
      else if (!wcscmp(token, _T("textSize")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        double num = wcstod(token, &end);
        if (end != token)
        {
          textSize = num;
        }
      }
      else if (!wcscmp(token, _T("screenPosition")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (token)
        {
          LPTSTR end = token;
          double num = wcstod(token, &end);
          if (end != token)
          {
            originX = num;
          }
        }

        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (token)
        {
          LPTSTR end = token;
          double num = wcstod(token, &end);
          if (end != token)
          {
            originY = num;
          }
        }
      }
		  else if (!wcscmp(token, _T("origin"))) // old miscalculated origin
		  {
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  double num = wcstod(token, &end);
				  if (end != token)
				  {
            originX = num*1534;
				  }
		      }

			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  double num = wcstod(token, &end);
				  if (end != token)
				  {
            originY = num*904;
				  }
			  }
		  }
		  else if (!wcscmp(token, _T("generator")))
		  {
			  int x = 0;
			  int y = 0;
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  int num = wcstol(token, &end, 10);
				  if (end != token)
				  {
					  x = num;
				  }
			  }

			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  int num = wcstol(token, &end, 10);
				  if (end != token)
				  {
					  y = num;
				  }
			  }

			  this->generatorVec = Vector2d(x, y);
		  }
		  else if (!wcscmp(token, _T("period")))
		  {
			  int x = 0;
			  int y = 0;
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  int num = wcstol(token, &end, 10);
				  if (end != token)
				  {
					  x = num;
				  }
			  }

			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  int num = wcstol(token, &end, 10);
				  if (end != token)
				  {
					  y = num;
				  }
			  }

			  this->periodVec = Vector2d(x, y);
		  }
		  else if (!wcscmp(token, _T("duplicateKeys")))
		  {
			  dupKeys = true;

			  int x = 0;
			  int y = 0;
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  int num = wcstol(token, &end, 10);
				  if (end != token)
				  {
					  x = num;
				  }
			  }

			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  int num = wcstol(token, &end, 10);
				  if (end != token)
				  {
					  y = num;
				  }
			  }

			  this->keyDuplicateVec = Vector2d(x, y);
		  }
		  else if (!wcscmp(token, _T("basis1")))
		  {
			  double x = 0;
			  double y = 0;
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  double num = wcstod(token, &end);
				  if (end != token)
				  {
					  x = num;
				  }
			  }

			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  double num = wcstod(token, &end);
				  if (end != token)
				  {
					  y = num;
				  }
			  }

			  g1 = Vector2d(x, y);
		  }
		  else if (!wcscmp(token, _T("basis2")))
		  {
			  double x = 0;
			  double y = 0;
			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  double num = wcstod(token, &end);
				  if (end != token)
				  {
					  x = num;
				  }
			  }

			  token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
			  if (token)
			  {
				  LPTSTR end = token;
				  double num = wcstod(token, &end);
          if (end != token)
          {
            y = num;
          }
        }

        g2 = Vector2d(x, y);
      }
      else if (!wcscmp(token, _T("keyShape")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (token)
        {
          if (!wcscmp(token, _T("HEXAGON")) || !wcscmp(token, _T("VORONOI")))
          {
            lattice.keyMode = VORONOI;
          }
          else if (!wcscmp(token, _T("CUSTOM")))
          {
            lattice.keyMode = CUSTOM;
          }
          else if (!wcscmp(token, _T("RECTANGLE")))
          {
            lattice.keyMode = RECTANGLE;
          }
        }
      }
      else if (!wcscmp(token, _T("textPos")))
      {
        Vector2dList& list = lattice.textPos;
        loadVectorList(token, next_token, lattice.textCustomLoc);
      }
      else if (!wcscmp(token, _T("verticies1")))
      {
        Vector2dList& list = lattice.textPos;
        loadVectorList(token, next_token, lattice.cellVerticies1);
      }
      else if (!wcscmp(token, _T("verticies2")))
      {
        Vector2dList& list = lattice.textPos;
        loadVectorList(token, next_token, lattice.cellVerticies2);
      }
      else if (!wcscmp(token, _T("verticies3")))
      {
        Vector2dList& list = lattice.textPos;
        loadVectorList(token, next_token, lattice.cellVerticies3);
      }
      else if (!wcscmp(token, _T("temperament")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (token)
        {
          temperament.name = token;
          trimFileExtension(temperament.name);
          temperament.name = convertFileNameToString(temperament.name.c_str());
        }
      }
      else if (!wcscmp(token, _T("lockKeyPattern")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"true"))
          lockKeyPattern = true;
      }
      else if (!wcscmp(token, _T("dynamicTemperament")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"true"))
          dynamicTemperament = true;
      }
      else if (!wcscmp(token, _T("EDOs")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) continue; // no more tokens
        if (!wcscmp(token, L"true"))
          EDOs = true;
      }
      else if (!wcscmp(token, _T("EDO")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        double num = wcstod(token, &end);
        if (end != token)
        {
          EDO = num;
        }
      }
      else if (!wcscmp(token, _T("periodInCents")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        double num = wcstod(token, &end);
        if (end != token)
        {
          periodInCents = num;
        }
      }
      else if (!wcscmp(token, _T("keySlope")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        double num = wcstod(token, &end);
        if (end != token)
        {
          keySlope = num;
        }
      }
      else if (!wcscmp(token, _T("Columns")))
       {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (token)
        {
          LPTSTR end = token;
          int num = wcstol(token, &end, 10);
          if (end != token)
          {
            columns = num;
          }
        }
      }
      else if (!wcscmp(token, _T("minEDO")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (token)
        {
          LPTSTR end = token;
          int num = wcstol(token, &end, 10);
          if (end != token)
          {
            minEDO = num;
          }
        }
      }
      else if (!wcscmp(token, _T("maxEDO")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (token)
        {
          LPTSTR end = token;
          int num = wcstol(token, &end, 10);
          if (end != token)
          {
            maxEDO = num;
          }
        }
      }
    }

    fitRatios.push_back(Ratio((Int)3,(Int)2));
    fitRatios.push_back(Ratio((Int)5, (Int)4));
    fitRatios.push_back(Ratio((Int)6, (Int)5));

    //if (keyDuplicateVec == Vector2d::Zero())
    //{
    //	keyDuplicateVec = periodVec + generatorVec;
    //}

    if (lattice.textCustomLoc.size() == 0)
    {
      lattice.textCustomLoc.push_back(Vector2d(0, 0));
    }

    maybeUpdateLattice(g1(0), g1(1), g2(0), g2(1));
    return temperament.loadTemperamentAsync(temperament.localTemperamentFolder);
  }).then([this](bool tempsLoaded)
  {
     if(tempsLoaded)
       return create_task([]() -> bool {return true;});
     else
       return temperament.loadTemperamentAsync(temperament.preloadedTemperamentFolder);
  }).then([this](bool tempsLoaded)
  {
    if (!tempsLoaded)
    {
      MessageDialog^ dialog = ref new MessageDialog(ref new Platform::String((L"Cannot open temperament file: " + temperament.name).c_str()), "");
      create_task(dialog->ShowAsync());
    }
    critical_section::scoped_lock lock(criticalSection);
    if(dynamicTemperament)
    {
      generateKeyboard(EDO);
      keyHeight12EDO = largeGen1.norm() * (EDO/12);

      mainPage->initializeKeyboardControls();
      mainPage->setKeyboardControllsVisibility(Windows::UI::Xaml::Visibility::Visible);
    }

    initialized = true;
  });
}

void LatticeView::loadVectorList(LPTSTR& token, LPTSTR& next_token, Vector2dList& list)
{
	while (token)
	{
		double x = 0;
		double y = 0;
		token = wcstok_s(NULL, _T(" (),"), &next_token); // subsequent tokens
		if (!token)
			break;

		{
			LPTSTR end = token;
			double num = wcstod(token, &end);
			if (end != token)
			{
				x = num;
			}
		}

		token = wcstok_s(NULL, _T(" (),"), &next_token); // subsequent tokens
		if (!token)
			break;

		{
			LPTSTR end = token;
			double num = wcstod(token, &end);
			if (end != token)
			{
				y = num;
			}
		}

		list.push_back(Vector2d(x, y));
	}
}


task<void> LatticeView::saveKeyboard()
{
  wstring fileNameBase = convertStringToFileName(keyboardName.c_str());
  wstring fileName = fileNameBase + _T(".kb");
  return create_task(localKeyboardFolder->CreateFileAsync(ref new Platform::String(fileName.c_str()), CreationCollisionOption::ReplaceExisting))
  .then([this](StorageFile^ file)
  {
    wstringstream content;

    content << "name " << keyboardName.c_str() << endl;
    content << "temperament " << convertStringToFileName(temperament.name.c_str()) << L".tpmt" << endl;
    content << "screenPosition " << originX << L" " << originY << endl;
    content << "basis1 " << lattice.largeGen1(0) << " " << lattice.largeGen1(1) << endl;
    content << "basis2 " << lattice.largeGen2(0) << " " << lattice.largeGen2(1) << endl;
    if (lattice.keyMode == VORONOI)
      content << "keyShape VORONOI" << endl;
    else if (lattice.keyMode == RECTANGLE)
      content << "keyShape RECTANGLE" << endl;
    else if (lattice.keyMode == CUSTOM)
      content << "keyShape CUSTOM" << endl;

    //  if (scale != 1)
    //    content << "zoom " << scale  << endl;

    if (textSize != 1)
      content << "textSize " << textSize << endl;

    if (scientificPitchNotation == true)
      content << "scientificPitchNotation true" << endl;

    if (use53TETNotation == true)
      content << "use53TETNotation true" << endl;

    if (showName == false)
      content << "showName false" << endl;

    if (showRatio == false)
      content << "showRatio false" << endl;

    if (showCents == false)
      content << "showCents false" << endl;

    if (showDelta == false)
      content << "showDelta false" << endl;

    if (duplicateRatios == false)
      content << "duplicateRatios false" << endl;

    if (playUnamedNotes == true)
      content << "playUnamedNotes true" << endl;

    content << "period " << lrint(periodVec(0)) << " " << lrint(periodVec(1)) << endl;
    content << "generator " << lrint(generatorVec(0)) << " " << lrint(generatorVec(1)) << endl;

    if (dupKeys && (keyDuplicateVec != Vector2d::Zero()))
      content << "duplicateKeys " << lrint(keyDuplicateVec(0)) << " " << lrint(keyDuplicateVec(1)) << endl;

    content << "textPos ";
    for (int i = 0; i < lattice.textCustomLoc.size(); i++)
    {
      content << "(" << lattice.textCustomLoc[i](0) << ", " << lattice.textCustomLoc[i](1) << ") ";
    }
    content << endl;

    content << "verticies1 ";
    for (int i = 0; i < lattice.cellVerticies1.size(); i++)
    {
      content << "(" << lattice.cellVerticies1[i](0) << ", " << lattice.cellVerticies1[i](1) << ") ";
    }
    content << endl;

    content << "verticies2 ";
    for (int i = 0; i < lattice.cellVerticies2.size(); i++)
    {
      content << "(" << lattice.cellVerticies2[i](0) << ", " << lattice.cellVerticies2[i](1) << ") ";
    }
    content << endl;

    content << "verticies3 ";
    for (int i = 0; i < lattice.cellVerticies3.size(); i++)
    {
      content << "(" << lattice.cellVerticies3[i](0) << ", " << lattice.cellVerticies3[i](1) << ") ";
    }
    content << endl;

    if (dynamicTemperament)
      content << "dynamicTemperament true" << endl;

    if (lockKeyPattern)
      content << "lockKeyPattern true" << endl;

    if (EDOs)
      content << "EDOs true" << endl;

    content << "keySlope " << keySlope << endl;
    content << "Columns " << columns << endl;
    content << "minEDO " << minEDO << endl;
    content << "maxEDO " << maxEDO << endl;
    content << "EDO " << EDO << endl;
    content << "periodInCents " << periodInCents << endl;

    wstring str = content.str();
    return FileIO::WriteTextAsync(file, ref new Platform::String(str.c_str()));
  });
}

void LatticeView::restartTrainingExercise()
 {
  notesToMatch.clear();
  currentSequance.clear();
  questions = 0;
  correct = 0;
}

void LatticeView::clearEarTrainingExercise()
{
  restartTrainingExercise();

  this->exerciseName = L"";
  octavesBelow = 2;
  octavesAbove = 3;
  duration = 500;
  sequenceLength = 0;
  instruments.clear();
}

void LatticeView::deleteDuplicateFiles(StorageFolder^ folder1, StorageFolder^ folder2)
{
  create_task(folder1->GetItemsAsync()).then([this, folder1, folder2](IVectorView<IStorageItem^>^ items)
  {
    for (unsigned int i = 0; i < items->Size; i++)
    {
      StorageFile^ file1 = (StorageFile^)items->GetAt(i);      
      create_task(folder2->GetFileAsync(file1->Name)).then([this, file1, folder2](StorageFile^ file2)
      {
        PRINT(L"~~~ file1 = %s\n", file1->Name->Data());
        PRINT(L"~~~ file2 = %s\n", file2->Name->Data());
        create_task(FileIO::ReadLinesAsync(file1)).then([this, file2](IVector<Platform::String^>^ lines1)
        {
          create_task(FileIO::ReadLinesAsync(file2)).then([this, file2, lines1](IVector<Platform::String^>^ lines2)
          {
            if (lines1->Size != lines2->Size)
              return;

            for (unsigned int i = 0; i < lines1->Size; i++)
            {
              Platform::String^ str1 = lines1->GetAt(i);
              Platform::String^ str2 = lines2->GetAt(i);
              if (Platform::String::CompareOrdinal(str1, str2))
                return;
            }

            create_task(file2->DeleteAsync()).then([=](task<void> t)
            {
              try
              {
                t.get();
              }
              catch (Platform::Exception^ e)  // file dose not exist
              {
              }
            });
          });
        });
      }).then([=](task<void> t)
      {
        try
        {
          t.get();
        }
        catch (Platform::Exception^ e)  // file dose not exist
        {
        }
      });
    }
  });
}


task<void> LatticeView::loadEarTrainingExerciseAsync(StorageFile^ file)
{
  return create_task(FileIO::ReadLinesAsync(file))
  .then([this, file](IVector<Platform::String^>^ lines)
  {
    critical_section::scoped_lock lock(this->criticalSection);

    clearEarTrainingExercise();

    this->exerciseName = convertFileNameToString(file->Name->Data());
    trimFileExtension(this->exerciseName);

    for (unsigned int i = 0; i < lines->Size; i++)
    {
      Platform::String^ str = lines->GetAt(i);
      if (str->Length() == 0)
        continue;

      // read an entire line into memory
      wchar_t buf[MAX_CHARS_PER_LINE] = { 0 };
      wcscpy_s(buf, str->Data());

      // parse the line into blank-delimited tokens
      int n = 0; // a for-loop index

      LPTSTR next_token = NULL;
      LPTSTR token = wcstok_s(buf, _T(" "), &next_token); // first token
      if (!token)
      {
        // do nothing
      }
      else if (!wcscmp(token, _T("name")))
      {
        wstring name;
        for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
        {
          token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
          if (!token) break; // no more tokens

          if (n > 1)
            name += _T(" ");

          name += token;
        }
      }
      else if (!wcscmp(token, _T("instruments")))
      {
        while (true)
        {
          token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
          if (!token) break; // no more tokens

          LPTSTR end = token;

          int num = wcstol(token, &end, 10);
          if (*end == _T('\0'))
          {
            instruments.push_back(num);
          }
        }
      }
      else if (!wcscmp(token, _T("octavesBelow")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        int num = wcstol(token, &end, 10);
        if (end != token)
        {
          octavesBelow = num;
        }
      }
      else if (!wcscmp(token, _T("octavesAbove")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        int num = wcstol(token, &end, 10);
        if (end != token)
        {
          octavesAbove = num;
        }
      }
      else if (!wcscmp(token, _T("duration")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        int num = wcstol(token, &end, 10);
        if (end != token)
        {
          duration = num;
        }
      }
      else if (!wcscmp(token, _T("sequenceLength")))
      {
        token = wcstok_s(NULL, _T(" "), &next_token); // subsequent tokens
        if (!token) break; // no more tokens

        LPTSTR end = token;

        int num = wcstol(token, &end, 10);
        if (end != token)
        {
          sequenceLength = num;
        }
      }
    }  
  });
}


int LatticeView::saveEarTrainingExercise()
{
  critical_section::scoped_lock lock(this->criticalSection);
  wstring fileNameBase = convertStringToFileName(exerciseName.c_str());
  wstring fileName = fileNameBase + _T(".etr");

  wstringstream content;

  content << "name " << exerciseName.c_str() << endl;
  content << "octavesBelow " << octavesBelow << endl;
  content << "octavesAbove " << octavesAbove << endl;
  content << "duration " << duration << endl;
  content << "sequenceLength " << sequenceLength << endl;

  content << "instruments";
  for (int i = 0; i < instruments.size(); i++)
  {
    content << " " << instruments[i] ;
  }
  content << endl;

  wstring str = content.str();

  StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
  create_task(localFolder->GetFolderAsync("EarTraining")).then([=](StorageFolder^ folder)
  {
    create_task(folder->CreateFileAsync(ref new Platform::String(fileName.c_str()), CreationCollisionOption::ReplaceExisting)).then([=](StorageFile^ file)
    {
      create_task(FileIO::WriteTextAsync(file, ref new Platform::String(str.c_str())));
    });
  });

  return 1;
}

double LatticeView::getBestKeySlope(double periodInCents, int Cols, double EDO)
{
  double bestSlope = 0;
  double bestDif = 100000;
  double xCents = periodInCents / Cols;
  double yCents = periodInCents / EDO;
  double colStepSize = periodInCents / Cols;
  for (int i = 0; i < Cols; i++)
  {
    double slope = static_cast<double>(i) / Cols;
    double currDelta = 0;
    for (int j = 0; j < fitRatios.size(); j++)
    {
      double target = 1200 * log2(static_cast<double>(fitRatios[j].num) / fitRatios[j].denom);
      int col = round(target / xCents);
      double y = slope * col;
      double minDelta = abs(col* xCents - y * yCents - target);
      double delta;
      while (true)
      {
        y++;
        delta = abs(col* xCents - y * yCents - target);
        if (delta < minDelta)
          minDelta = delta;
        else
          break;
      }

      y = slope * col;
      while (true)
      {
        y--;
        delta = abs(col* xCents - y * yCents - target);
        if (delta < minDelta)
          minDelta = delta;
        else
          break;
      }

      currDelta += minDelta;
    }  


    if (currDelta < bestDif)
    {
      bestDif = currDelta;
      bestSlope = slope;
    }
  }

  return bestSlope;
}


double LatticeView::getBestKeySlopeEDO(double periodInCents, int Cols, double EDO)
{
  double slopes[] = {floor(EDO) / Cols, ceil(EDO) / Cols };
  double bestSlope = 0;
  double bestDif = 100000;
  double xCents = periodInCents / Cols;
  double yCents = periodInCents / EDO;
  double colStepSize = periodInCents / Cols;
  for (int i = 0; i <= 1; i++)
  {
    double slope = slopes[i];
    double currDelta = 0;
    for (int j = 0; j < fitRatios.size(); j++)
    {
      double target = 1200 * log2(static_cast<double>(fitRatios[j].num) / fitRatios[j].denom);
      int col = round(target / xCents);
      double y = slope * col;
      double minDelta = abs(col* xCents - y * yCents - target);
      double delta;
      while (true)
      {
        y++;
        delta = abs(col* xCents - y * yCents - target);
        if (delta < minDelta)
          minDelta = delta;
        else
          break;
      }

      y = slope * col;
      while (true)
      {
        y--;
        delta = abs(col* xCents - y * yCents - target);
        if (delta < minDelta)
          minDelta = delta;
        else
          break;
      }

      currDelta += minDelta;
    }


    if (currDelta < bestDif)
    {
      bestDif = currDelta;
      bestSlope = slope;
    }
  }

  return bestSlope;
}

double LatticeView::getMaxY(double periodInCents, int Cols, double EDO, double slope)
{
  double xCents = periodInCents / Cols;
  double yCents = periodInCents / EDO;
  double colStepSize = periodInCents / Cols;
  double maxY = 0;
  for (int j = 0; j < fitRatios.size(); j++)
  {
    double target = 1200 * log2(static_cast<double>(fitRatios[j].num) / fitRatios[j].denom);
    int col = round(target / xCents);
    double y = slope * col;
    double minDelta = abs(col* xCents - y * yCents - target);
    double currY = 0;
    double delta;
    while (true)
    {
      y++;
      delta = abs(col* xCents - y * yCents - target);
      if (delta < minDelta)
      {
        minDelta = delta;
        currY = y;
      }
      else
        break;
    }

    y = slope * col;
    while (true)
    {
      y--;
      delta = abs(col* xCents - y * yCents - target);
      if (delta < minDelta)
      {
        minDelta = delta;
        currY = y;
      }
      else
        break;
    }
    
    currY = abs(currY);
    if (currY > maxY)
    {
      maxY = currY;
    }
  }

  return maxY;
}

void LatticeView::mapRatios()
{
  double xCents = periodInCents / columns;
  double yCents = periodInCents / EDO;
  double colStepSize = periodInCents / columns;
  for (int j = 0; j < temperament.ratioGeneratorsIn.size(); j++)
  {
    double target = 1200 * log2(static_cast<double>(temperament.ratioGeneratorsIn[j].num) / temperament.ratioGeneratorsIn[j].denom);
    int col = round(target / xCents);
    int currCol = col;
    double y = keySlope * col;
    double minDelta = abs(col* xCents - y * yCents - target);
    double currY = y;
    double delta;
    while (true)
    {
      y++;
      delta = abs(col* xCents - y * yCents - target);
      if (delta < minDelta)
      {
        minDelta = delta;
        currY = y;
      }
      else
        break;
    }

    y = keySlope * col;
    while (true)
    {
      y--;
      delta = abs(col* xCents - y * yCents - target);
      if (delta < minDelta)
      {
        minDelta = delta;
        currY = y;
      }
      else
        break;
    }


    // check the next column over to see if there is a better aproximation.
    double yThreshold = 3; // deturnings the distance at which the next column can be mapped if it is better than the first.
                           // value of  0 means never. A value of 1 means if it is closer, a value of .5 means if it is half the distance, etc.

    col += (col * xCents - target > 0 ? -1 : 1);
    y = keySlope * col;
    delta = abs(col * xCents - y * yCents - target);
    double stopDelta = delta;
    if (delta < minDelta && abs(y/currY) < yThreshold)
    {
      minDelta = delta;
      currY = y;
      currCol = col;
    }

    while (true)
    {
      y++;
      delta = abs(col* xCents - y * yCents - target);
      if (delta < minDelta && abs(y / currY) < yThreshold)
      {
        minDelta = delta;
        stopDelta = delta;
        currY = y;
        currCol = col;
      }
      else if(delta >= stopDelta)
        break;
    }

    y = keySlope * col;
    while (true)
    {
      y--;
      delta = abs(col* xCents - y * yCents - target);
      if (delta < minDelta && abs(y / currY) < yThreshold)
      {
        minDelta = delta;
        stopDelta = delta;
        currY = y;
        currCol = col;
      }
      else if (delta >= stopDelta)
        break;
    }


    temperament.persIn[j] = -round(currY - keySlope * currCol);
    temperament.gensIn[j] = -currCol;
    temperament.shiftCents =  yCents*shiftPercent;

    temperament.calcMatricies();
    temperament.clearNoteNames();
  }
}

void LatticeView::generateKeyboard(double newEDO, bool resizeKey)
{
  restartTrainingExercise();
  double newkeySlope = keySlope;
  if(!lockKeyPattern)
  {
     double oldMaxY = getMaxY( periodInCents, columns,  EDO, keySlope);
     double newkeySlope = 0;
     if(EDOs)
     {
       newEDO = round(newEDO);
       newkeySlope = newEDO / columns; // replace this with some soret of badness calculation based on some limit.
     }
     else
     { 
       newkeySlope = getBestKeySlope( periodInCents, columns, newEDO);
     }
     double newMaxY = getMaxY(periodInCents, columns, newEDO, newkeySlope);

     Vector2d hat1 = largeGen1 / largeGen1.norm();

     //bool fixedYCentsScale = false;

     //if (abs(1 - newkeySlope / keySlope) > .001 || fixedYCentsScale)
     {
       keySlope = newkeySlope;
      // another mode of key resize
      if (resizeKey)
      { 
         if(fixedYCentsScale)
//           largeGen1 = largeGen1 * EDO / newEDO;
           largeGen1 = hat1 * keyHeight12EDO * 12 / newEDO;
         else
           largeGen1 = largeGen1 * (oldMaxY + 0.5) / (newMaxY + 0.5);
//           largeGen1 = hat1 * keyHeight12EDO *  (maxY12EDO + 0.5) / (newMaxY + 0.5);
      }
        
           
       largeGen2 = largeGen2 - (largeGen2.dot(hat1) - largeGen1.norm()*(keySlope))*hat1;

       lattice.setBasis(largeGen1, largeGen2);
       mapRatios();
       invalidateCellPath();
     }
  }
  else
  {
    mapRatios();
    invalidateCellPath();
  }

  EDO = newEDO;
  periodVec = Vector2d(1, 0);
  generatorVec = Vector2d(0, 1);

  temperament.generateTemperament(periodInCents, columns, EDO, keySlope);
  invalidateBitmap();
}