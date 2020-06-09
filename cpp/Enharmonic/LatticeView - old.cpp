#include "pch.h"
#include "LatticeView.h"
#include "Common.h"

#include <fstream>
#include <string>
#include <sstream>

using namespace Windows::UI::Core;
using namespace Windows::Foundation::Collections;
using namespace SDKTemplate;
using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::UI::Popups;

#include <eigen/Eigen/Dense>
using  Eigen::Vector2d;
//using namespace Eigen;

//using namespace std;
//using namespace arma;

#define interval1ToCartesian(x1,y1)  x1 = lattice.largeGen1(0) * generatorVec(0) +\
                                          lattice.largeGen2(0) * generatorVec(1);\
                                     y1 = lattice.largeGen1(1) * generatorVec(0) +\
                                          lattice.largeGen2(1) * generatorVec(1);
#define interval2ToCartesian(x1,y1)  x1 = lattice.largeGen1(0) * periodVec(0) +\
                                          lattice.largeGen2(0) * periodVec(1);\
                                     y1 = lattice.largeGen1(1) * periodVec(0) +\
                                          lattice.largeGen2(1) * periodVec(1);

#define CELLBITMAPMARGIN 4
#define PATHBITMAPMARGIN 20

//
//#define originXToPix -(0.5+originX)*m_d3dRenderTargetSize.Width / m_dpi * 96.0f
//#define originYToPix -(0.5+originY)*m_d3dRenderTargetSize.Height / m_dpi * 96.0f

//#define pixToOriginX -originXPix / (m_d3dRenderTargetSize.Width / m_dpi * 96.0f)-0.5
//#define pixToOriginY -originYPix / (m_d3dRenderTargetSize.Height / m_dpi * 96.0f)-0.5

//#define originXToPix -(0.5+originX + 0.5*(1/scale-1))*m_d3dRenderTargetSize.Width / m_dpi * 96.0f
//#define originYToPix -(0.5+originY + 0.5*(1/scale-1))*m_d3dRenderTargetSize.Height / m_dpi * 96.0f

#define originXToPix -(originX + 0.5/scale)*m_d3dRenderTargetSize.Width / m_dpi * 96.0f
#define originYToPix -(originY + 0.5/scale)*m_d3dRenderTargetSize.Height / m_dpi * 96.0f

#define pixToOriginX -originXPix / (m_d3dRenderTargetSize.Width / m_dpi * 96.0f)-0.5/scale
#define pixToOriginY -originYPix / (m_d3dRenderTargetSize.Height / m_dpi * 96.0f)-0.5/scale


LatticeView::LatticeView() :
	m_pWhiteBrush(NULL),
	m_pBlackBrush(NULL),
	m_pBorderBrush(NULL),
	m_pCellPathGeometry(NULL),
	m_pArrowPathGeometry(NULL),
	m_pTextFormat(NULL),
	m_pGeneratorBrush(NULL),
	m_pGlowBrush(NULL),
	latticeBitMap(nullptr),
	latticeBitMapValid(false),
	instrument()

{
		//mat A = randu<mat>(4, 5);
		//mat B = randu<mat>(4, 5);

		//cout << A*B.t() << endl;

	//MatrixXd m(2, 2);
	//m(0, 0) = 3;
	//m(1, 0) = 2.5;
	//m(0, 1) = -1;
	//m(1, 1) = m(1, 0) + m(0, 1);
	//std::cout << m << std::endl;



	//Matrix<long, 4, 4> m;
	//m << 1, 2, 3, 55,
	//	4, 5, 6, 45,
	//	7, 8, 45, 9;


	CreateDeviceIndependentResources();
	CreateDeviceDependentResources();

	QueryPerformanceFrequency(&Freq);
	QueryPerformanceCounter(&EndingTime);
	
	this->temperament.criticalSection = &criticalSection;
//	StorageFolder^ folder = PerformSynchronously(localFolder->CreateFolderAsync("Temperaments", Windows::Storage::CreationCollisionOption::OpenIfExists));
//    StorageFile^ temperamentFile = PerformSynchronously(folder->GetFileAsync(ref new Platform::String(L"!meantone.tpmt")));
//	this->temperament->loadTemperament(temperamentFile);


	this->vibratoCuttoffFreq = 10;
	//this->vibratoCuttoffFreq = 1000;
	this->vibratoAmplitude = 1;
	this->vibrato = false;
	this->vibrato = true;
//	this->roundFirstNote = false;
	this->roundFirstNote = true;
	this->dragMode = SUSTAIN;
//	this->dragMode = GLISSANDO;
//	this->dragMode = CONTINUUM;


	this->vibratoTrigger = true;
	this->triggerResetRate = 2;
	this->triggerThreshold = 5;

	this->usePitchAxis = true;
	this->pitchAxis(0) = 0;
	this->pitchAxis(1) = 1;

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

	StorageFolder^ folder = PerformSynchronously(localFolder->CreateFolderAsync("Keyboards", Windows::Storage::CreationCollisionOption::OpenIfExists));
	StorageFolder^ defaultFolder = PerformSynchronously(installedLocation->GetFolderAsync(L"Keyboards"));
	IVectorView<IStorageItem^>^ items = PerformSynchronously(defaultFolder->GetItemsAsync());
	for (unsigned int i = 0; i < items->Size; i++)
	{
		IStorageItem^ item = items->GetAt(i);
		IStorageItem^ destItem = PerformSynchronously(folder->TryGetItemAsync(item->Name));
		if (destItem == nullptr)  //If file doen't already exist, copy it
			PerformSynchronously(((StorageFile^)item)->CopyAsync(folder));
	}

	StorageFile^ keyboardFile = PerformSynchronously(folder->GetFileAsync(ref new Platform::String(L"!meantone_(2-rank)_-_!wicki-!hayden.kb")));
	this->loadKeyboard(keyboardFile);
	
	//double pi = 3.14159;
	//double size = 110;
	////double size = 120;

	////  Vector2d* g1 = [[Vector2d alloc] init_X:0 Y:150];
	////  Vector2d* g1 = [[Vector2d alloc] init_X:50 Y:100];
	////Vector2d* g1 = [[Vector2d alloc] init_X:30 Y:70];
	////  Vector2d* g1 = [[Vector2d alloc] init_X:100 Y:100];
	//Vector2d g1(size * sin(pi / 3), size * cos(pi / 3));
	//Vector2d g2(-size * sin(pi / 3), size * cos(pi / 3));
	//double theta = 0 * pi;
	//g1 = Vector2d(g1(0)*cos(theta) - g1(1)*sin(theta), g1(0)*sin(theta) + g1(1)*cos(theta));
	//g2 = Vector2d(g2(0)*cos(theta) - g2(1)*sin(theta), g2(0)*sin(theta) + g2(1)*cos(theta));
	//maybeUpdateLattice(g1(0), g1(1), g2(0),g2(1));
	//generatorVec = Vector2d(1, 0);
	//periodVec = Vector2d(1, -1);
	

//	lattice.keyMode = RECTANGLE;
//	g1 = Vector2d(50,10);
//	g2 = Vector2d(100,0);
//  maybeUpdateLattice(g1(0), g1(1), g2(0),g2(1));

//	temperament.periodCents = 1200;
//	temperament.baseFreq = 261.625565;

//Skysmatic <1 0 15|, <0 1 -8|  with mapping generator = 3
//	temperament.setJustMapping(Val({1, 1, 7}), Val({0, 1, -8})); temperament.genCents = 701.736;

//Skysmatic Garibaldi <1 0 15 25|, <0 1 -8 -14| with mapping generator = 3
//	temperament.setJustMapping(Val({ 1, 1, 7, 11 }), Val({ 0, 1, -8, -14 })); temperament.genCents = 702.085;

//Skysmatic Garibaldi 13 limit <1 0 15 25 -33 -28|, <0 1 -8 -14 23 20| with mapping generator = 3
//	temperament.setJustMapping(Val({ 1, 1, 7, 11, -10, -8 }), Val({ 0, 1, -8, -14, 23, 20 })); temperament.genCents = 702.113;

//Meantone  <1 0 -4|, <0 1 4| with mapping generator = 3
//	temperament.setJustMapping(Val({1, 1, 0}), Val({0, 1, 4}));     temperament.genCents = 696.578;

// Septimal Meantone <1 0 -4 -13|, <0 1 4 10| with mapping generator = 3
//	temperament.setJustMapping(Val({ 1, 1 , 0, -3 }), Val({ 0, 1, 4, 10 })); temperament.genCents = 696.495;

// Tridecimal meantone: <1 0 -4 -13 -25 -20| , <0 1 4 10 18 15| with mapping generator = 3
//	temperament.setJustMapping(Val({ 1, 1 , 0, -3, -7, -5 }), Val({ 0, 1, 4, 10, 18, 15 })); temperament.genCents = 696.642;

// Flat Tone Meantone <1 0 -4 17|, <0 1 4 -9| with mapping generator = 3
//	temperament.setJustMapping(Val({ 1, 1 , 0, 8}), Val({ 0, 1, 4, -9 })); temperament.genCents = 696.779;

// Magic <1 0 2|, <0 5 1|
//	temperament.setJustMapping(Val({ 1, 0, 2 }), Val({ 0, 5, 1 })); temperament.genCents = 696.779;

// Temperament with 3/2 period and 9/8 generator
//	temperament.setJustMapping(Val({ 2, 3 }), Val({ -1, -1 })); temperament.genCents = 200;

//	temperament.calculatePrimes();
//	temperament.calcMatricies();

//	temperament.loadTemperament(_T("mean_septimal.tpmt"));
//	temperament.loadTemperament(_T("skysmatic_garbaldi_13limit.tpmt"));
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
}


void LatticeView::CreateDeviceIndependentResources()
{
	DirectXBase::CreateDeviceIndependentResources();

	static const WCHAR msc_fontName[] = L"Verdana";
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
			));


	// Center the text horizontally and vertically.
	DX::ThrowIfFailed(m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	DX::ThrowIfFailed(m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
}


void LatticeView::invalidateCellPath()
{
	latticeBitMapValid = false;
	cellPathValid = false;
}

void LatticeView::DrawLatticeBitmap()
{
		if (latticeBitMapValid)
			return;

		temperament.clearNoteNames();

		latticeBitMapValid = true;

		DX::ThrowIfFailed(CreateCellPath());

		ComPtr<ID2D1Image> oldTarget;
		m_d2dContext->GetTarget(&oldTarget);

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);

		// Draw onto the input bitmap instead of the window's surface.
		m_d2dContext->SetTarget(latticeBitMap.Get());


		RECT bounds;
		bounds.left = 0;
		bounds.right = m_d3dRenderTargetSize.Width / m_dpi * 96.0f / scale;
		bounds.top = 0;
		bounds.bottom = m_d3dRenderTargetSize.Height / m_dpi * 96.0f / scale;


		m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::SizeF(scale, scale), D2D1::Point2F(0, 0)));

		m_d2dContext->BeginDraw();
		m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f));
		try
		{
			renderLattice(bounds);
		}
		catch (...)
		{
			PRINT(L"crap");
		}

	  // D2D1_RECT_F boundsRect = D2D1::RectF(bounds.left, bounds.top, bounds.right, bounds.bottom);
	  //  m_d2dContext->DrawRectangle(&boundsRect, m_pGlowBrush);

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		HRESULT hr = m_d2dContext->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET)
		{
			DX::ThrowIfFailed(hr);
		}

		m_d2dContext->SetTarget(oldTarget.Get());
		m_d2dContext->SetTransform(t);

}


HRESULT LatticeView::CreateCellPath()
{
	HRESULT hr = S_OK;
	if (cellPathValid)
		return hr;

	if(m_pCellPathGeometry)
	  SafeRelease(&m_pCellPathGeometry);

	lattice.setBasis(largeGen1, largeGen2);
	lattice.calculateCellShape();

	hr = m_d2dFactory->CreatePathGeometry(&m_pCellPathGeometry);

	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink *pSink = NULL;
		hr = m_pCellPathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

			int numPoints = lattice.cellVerticiesDraw.size();
			D2D1_POINT_2F* points = new D2D1_POINT_2F[numPoints];
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

	ComPtr<ID2D1Image> oldTarget;
	m_d2dContext->GetTarget(&oldTarget);

	float cellBitMapWidth = lattice.cellWidth + 2 * CELLBITMAPMARGIN;
	float cellBitMapHeight = lattice.cellHeight + 2 * CELLBITMAPMARGIN;
	///////////////////////////////////////////////////////////////////////////////////////
	// create white cell bitmap
	{
		// Convert from DIPs to pixels, since bitmaps are created in units of pixels.
		D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
			static_cast<UINT32>(cellBitMapWidth / 96.0f * m_dpi),
			static_cast<UINT32>(cellBitMapHeight / 96.0f * m_dpi)
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
				&whiteCellBitMap
				)
			);

		// Draw onto the input bitmap instead of the window's surface.
		m_d2dContext->SetTarget(whiteCellBitMap.Get());

		m_d2dContext->BeginDraw();

		// Clear the bitmap with transparent white.
		m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);
		D2D1_MATRIX_3X2_F t2 = t*D2D1::Matrix3x2F::Translation(-lattice.cellMinX + CELLBITMAPMARGIN, -lattice.cellMinY + CELLBITMAPMARGIN);
		m_d2dContext->SetTransform(t2);
		m_d2dContext->FillGeometry(m_pCellPathGeometry, m_pWhiteBrush);
		m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pBorderBrush, 2.5);
		m_d2dContext->SetTransform(t);

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		hr = m_d2dContext->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET)
		{
			DX::ThrowIfFailed(hr);
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////
	// create black cell bitmap
	{
		// Convert from DIPs to pixels, since bitmaps are created in units of pixels.
		D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
			static_cast<UINT32>(cellBitMapWidth / 96.0f * m_dpi),
			static_cast<UINT32>(cellBitMapHeight / 96.0f * m_dpi)
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
				&blackCellBitMap
				)
			);

		// Draw onto the input bitmap instead of the window's surface.
		m_d2dContext->SetTarget(blackCellBitMap.Get());

		m_d2dContext->BeginDraw();

		// Clear the bitmap with transparent white.
		m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);
		D2D1_MATRIX_3X2_F t2 = t*D2D1::Matrix3x2F::Translation(-lattice.cellMinX + CELLBITMAPMARGIN, -lattice.cellMinY + CELLBITMAPMARGIN);
		m_d2dContext->SetTransform(t2);
		m_d2dContext->FillGeometry(m_pCellPathGeometry, m_pBlackBrush);
		m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pBorderBrush, 2.5);
		m_d2dContext->SetTransform(t);

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		hr = m_d2dContext->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET)
		{
			DX::ThrowIfFailed(hr);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// create grey cell bitmap
	{
		// Convert from DIPs to pixels, since bitmaps are created in units of pixels.
		D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
			static_cast<UINT32>(cellBitMapWidth / 96.0f * m_dpi),
			static_cast<UINT32>(cellBitMapHeight / 96.0f * m_dpi)
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
				&greyCellBitMap
				)
			);

		// Draw onto the input bitmap instead of the window's surface.
		m_d2dContext->SetTarget(greyCellBitMap.Get());

		m_d2dContext->BeginDraw();

		// Clear the bitmap with transparent white.
		m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));

		D2D1_MATRIX_3X2_F t;
		m_d2dContext->GetTransform(&t);
		D2D1_MATRIX_3X2_F t2 = t*D2D1::Matrix3x2F::Translation(-lattice.cellMinX + CELLBITMAPMARGIN, -lattice.cellMinY + CELLBITMAPMARGIN);
		m_d2dContext->SetTransform(t2);
//		m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pBorderBrush, 2.5);
//		m_d2dContext->FillGeometry(m_pCellPathGeometry, m_pBorderBrush);
		m_d2dContext->SetTransform(t);

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		hr = m_d2dContext->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET)
		{
			DX::ThrowIfFailed(hr);
		}
	}

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
				)
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
			DX::ThrowIfFailed(hr);
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
				)
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
			DX::ThrowIfFailed(hr);
		}
	}

	m_d2dContext->SetTarget(oldTarget.Get());

	float blurAmount = 6.0f;
	DX::ThrowIfFailed(m_d2dContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pathBlurEffect)	);
	DX::ThrowIfFailed(m_pathBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, blurAmount));
	m_pathBlurEffect->SetInput(0, pathBitMap.Get());

	DX::ThrowIfFailed(m_d2dContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_touchBlurEffect));
	DX::ThrowIfFailed(m_touchBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 10.0f));
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

	DX::ThrowIfFailed(CreateArrowHead());

	// Create some brushes.
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pWhiteBrush));
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),&m_pBlackBrush));
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray),	&m_pBorderBrush));
//	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0), &m_pBorderBrush));
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pGeneratorBrush));
	DX::ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &m_pGlowBrush));

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

	latticeBitMapValid = false;

	D2D1_SIZE_U bitmapSizeInPixels = D2D1::SizeU(
		static_cast<UINT32>(m_d3dRenderTargetSize.Width),
		static_cast<UINT32>((m_d3dRenderTargetSize.Height))
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
			&latticeBitMap
			)
		);
	//DX::ThrowIfFailed(
	//	m_d2dContext->CreateBitmap(
	//		bitmapSizeInPixels,
	//		nullptr,
	//		0,
	//		D2D1::BitmapProperties(
	//			D2D1_BITMAP_OPTIONS_TARGET,
	//			D2D1::PixelFormat(
	//				DXGI_FORMAT_B8G8R8A8_UNORM,
	//				D2D1_ALPHA_MODE_PREMULTIPLIED
	//				),
	//			m_dpi,
	//			m_dpi
	//			),
	//		&latticeBitMap)
	//		)
	//	);
}


TouchData* LatticeView::newTouchData(unsigned dwID)
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
	HRESULT hr = S_OK;

	if (SUCCEEDED(hr))
	{
		if (playMusic)
		{
			DrawLatticeBitmap();
			m_d2dContext->BeginDraw();
			m_d2dContext->DrawBitmap(latticeBitMap.Get());
		}
		else  // This previously causes a crash on secon keyboard reload for some reason. the issues seems is the (intersects.y1 != intersects.y2) in render rows. THis throws an eception on line 808. Seemes like this is fixed now.
		{
			DX::ThrowIfFailed(CreateCellPath());

			RECT bounds;
			bounds.left = 0;
			bounds.right = m_d3dRenderTargetSize.Width / m_dpi * 96.0f / scale;
			bounds.top = 0;
			bounds.bottom = m_d3dRenderTargetSize.Height / m_dpi * 96.0f / scale;

			D2D1_MATRIX_3X2_F t;
			m_d2dContext->GetTransform(&t);
			m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::SizeF(scale, scale), D2D1::Point2F(0, 0)));

			m_d2dContext->BeginDraw();
			m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f));

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

		if (playMusic)
		{
			for (int i = 0; i < touchDataArray.size(); i++)
			{
				TouchData* td = &touchDataArray[i];
				//if (td->noteID != -1)
				if (td->phase != NOTHING)
				{
					m_d2dContext->DrawImage(m_pathBlurEffect.Get(), D2D1::Point2F(td->cellPos(0) + lattice.cellMinX - PATHBITMAPMARGIN, td->cellPos(1) + lattice.cellMinY - PATHBITMAPMARGIN));
					m_d2dContext->DrawImage(m_touchBlurEffect.Get(), D2D1::Point2F(td->pos.x - touchBitMapWidth / 2, td->pos.y - touchBitMapHeight / 2));

				}
			}
		}
		else
		{
			renderArrows();

			if (lattice.keyMode == CUSTOM)
			{
				double x = 0;
				double y = 0;
				lattice.latticeToCartesian(&x, &y);
				x -= originXDraw;
				y -= originYDraw;

				D2D1_MATRIX_3X2_F t;
				m_d2dContext->GetTransform(&t);
				D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Translation(x, y)*t;
				m_d2dContext->SetTransform(t2);
				m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pGlowBrush, 2.5);
				m_d2dContext->SetTransform(t);

				for (int i = 0; i < lattice.cellVerticies.size(); i++)
				{
					D2D1_ELLIPSE dot = D2D1::Ellipse(
						D2D1::Point2F(x + lattice.cellVerticies[i](0), y + lattice.cellVerticies[i](1)),
						5.f,
						5.f
						);
					m_d2dContext->FillEllipse(dot, m_pGlowBrush);
				}

				for (int i = 0; i < lattice.textPos.size(); i++)
				{
					D2D1_ELLIPSE dot = D2D1::Ellipse(
						D2D1::Point2F(x + lattice.textPos[i](0)*lattice.right(0) + lattice.textPos[i](1)* lattice.down(0), y + lattice.textPos[i](0)*lattice.right(1) + lattice.textPos[i](1)* lattice.down(1)),
						5.f,
						5.f
						);
					m_d2dContext->FillEllipse(dot, m_pGlowBrush);
				}

			}
			
	
		}

		m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

		//Call the render target's EndDraw method.
		//The EndDraw method returns an HRESULT to indicate whether the drawing operations were successful.
		hr = m_d2dContext->EndDraw();
	}

	// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	// is lost. It will be handled during the next call to Present.
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr);
		hr = S_OK;
	}
}

//void LatticeView::OnResize(UINT width, UINT height)
//{
//	//if (m_d2dContext)
//	//{
//	//	// Note: This method can fail, but it's okay to ignore the
//	//	// error here, because the error will be returned again
//	//	// the next time EndDraw is called.
//	//	m_d2dContext->Resize(D2D1::SizeU(width, height));
//	//}
//
//	UpdateForWindowSizeChange();
//}


HRESULT LatticeView::renderLattice(const RECT& invalidRect)
{
	HRESULT hr = S_OK;

	int margin = 0;
	
	originXDraw = originXToPix;
	originYDraw = originYToPix;


	RECT bounds;
	bounds.left = static_cast<int>(margin + invalidRect.left + originXDraw - lattice.cellWidth / 2);
	bounds.right = static_cast<int>(invalidRect.right - margin + originXDraw + lattice.cellWidth / 2) + 1;
	bounds.top = static_cast<int>(margin + invalidRect.top + originYDraw - lattice.cellHeight / 2);
	bounds.bottom = static_cast<int>(invalidRect.bottom - margin + originYDraw + lattice.cellHeight / 2) + 1;

	double x = (bounds.left + bounds.right) / 2;
	double y = (bounds.top + bounds.bottom) / 2;
	lattice.getCell(&x, &y);
	lattice.latticeToCartesian(&x, &y);

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

//		lattice.getCell(&intersects.x1, &intersects.y1);
//		lattice.getCell(&intersects.x2, &intersects.y2);
//		lattice.latticeToCartesian(&intersects.x1, &intersects.y1);
//		lattice.latticeToCartesian(&intersects.x2, &intersects.y2);

		// xIntersection and its freinds now contain the latice coordinates
		// of the cells where the raster line intersects the oversized view rectangle.

		if (intersects.y1 != intersects.y2)
		{
			//Throw(_T("Invalide state : yIntersect1 != yIntersect2 : yIntersect1 = %f, yIntersect2 = %f"), intersects.y1, intersects.y2);
			throw Platform::Exception::CreateException(S_FALSE);
			return 1;
		}

		double yFirst, yLast;
	//	if (intersects.y1 < intersects.y2)
		{
			yFirst = intersects.y1;
			yLast = intersects.y2;
		}
	//	else
	//	{
	//		yFirst = intersects.y2;
	//		yLast = intersects.y1;
	//	}

		double yCell = yFirst;
	//	for (double yCell = yFirst; yCell <= yLast; yCell++)
	//	{
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
				int midiNote = instrument.getMidiNoteFromFreq(frequencyFromCents(cents));    // MIDI note-on message: Key number (60 = middle C) 69 = A4
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

//	    }

		b += bInc;
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

void LatticeView::renderArrows()
{
	//Vector2d right = Vector2d::subtract(lattice.largeGen2, lattice.largeGen1.hat().scale(Vector2d::dot(lattice.largeGen2, lattice.largeGen1.hat())));
	//Vector2d down1 = lattice.largeGen1;
	//Vector2d down2 = lattice.largeGen1.hat().scale(Vector2d::dot(lattice.largeGen2, lattice.largeGen1.hat()));

	//while (Vector2d::dot(down2, lattice.largeGen1) < 0)
	//	down2 = Vector2d::add(down2, down1);

	//while (down2.norm() > down1.norm())
	//	down2 = Vector2d::subtract(down2, down1);

	//Vector2d d1 = Vector2d::subtract(down1, down2);
	//Vector2d d2 = down2;
	//Vector2d r = Vector2d::subtract(lattice.largeGen2, lattice.largeGen1.hat().scale(Vector2d::dot(lattice.largeGen2, lattice.largeGen1.hat())));

	//{
	//	double x = 0;
	//	double y = 0;
	//	Lattice::latticeToCartesian(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
	//	x -= originXDraw;
	//	y -= originYDraw;

	//	double xx = r(0);
	//	double yy = r(1);

	//	m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGeneratorBrush, 4, NULL);

	//	double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
	//	angle += yy < 0 ? 180 : 0;  //Corect for phase

	//	D2D1_MATRIX_3X2_F t;
	//	m_d2dContext->GetTransform(&t);
	//	D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
	//	m_d2dContext->SetTransform(t2);
	//	m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGeneratorBrush);
	//	m_d2dContext->SetTransform(t);
	//}

	//{
	//	double x = 4;
	//	double y = 4;
	//	Lattice::latticeToCartesian(&x, &y, &lattice.largeGen1, &lattice.largeGen2);
	//	x -= originXDraw;
	//	y -= originYDraw;

	//	double xx = r.hat()(0);
	//	double yy = r.hat()(1);

	//	m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGeneratorBrush, 4, NULL);

	//	double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
	//	angle += yy < 0 ? 180 : 0;  //Corect for phase

	//	D2D1_MATRIX_3X2_F t;
	//	m_d2dContext->GetTransform(&t);
	//	D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
	//	m_d2dContext->SetTransform(t2);
	//	m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGeneratorBrush);
	//	m_d2dContext->SetTransform(t);
	//}
	//return;


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

		for (unsigned int i = 0; i < touchDataArray.size(); i++) // draw vectors being draged;
		{
			TouchData* touchData = &touchDataArray[i];
			if (touchData->intervalVector != 0)
			{
				double xx = touchData->dragVector(0);
				double yy = touchData->dragVector(1);

				m_d2dContext->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(xx + x, yy + y), m_pGeneratorBrush, 4, NULL);

				double angle = -std::atan(xx / yy) * 180 / PI; //angle needs to be in degrees
				angle += yy < 0 ? 180 : 0;  //Corect for phase

				D2D1_MATRIX_3X2_F t;
				m_d2dContext->GetTransform(&t);
				D2D1_MATRIX_3X2_F t2 =  D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0))*D2D1::Matrix3x2F::Translation(xx + x, yy + y)*t;
				m_d2dContext->SetTransform(t2);
				m_d2dContext->FillGeometry(m_pArrowPathGeometry, m_pGeneratorBrush);
				m_d2dContext->SetTransform(t);

			}

		}

	}
}

double LatticeView::getCents(double x, double y)
{
	double x1, y1, x2, y2;
	interval1ToCartesian(x1, y1);
	interval2ToCartesian(x2, y2);

	Lattice::cartesianToLattice(&x, &y, &Vector2d(x1, y1), &Vector2d(x2, y2));

	return temperament.getCents(x, y);
}


double LatticeView::frequencyFromCents(double cents)
{
	return temperament.baseFreq*pow(2, cents  / 1200);
}

double LatticeView::centsFromFrequance(double freq)
{
	return 1200 * log2(freq/ temperament.baseFreq);
}

// This one uses images for the text, but draws the cells from scratch each time. This seems to be by far the fastest.
HRESULT LatticeView::drawCell(double x, double y)
{
	HRESULT hr = S_OK;

	double xNote = x + originXDraw;
	double yNote = y + originYDraw;

	double x1, y1, x2, y2;
	interval1ToCartesian(x1, y1);
	interval2ToCartesian(x2, y2);

	Lattice::cartesianToLattice(&xNote, &yNote, &Vector2d(x1, y1), &Vector2d(x2, y2));


	int yNoteInt = round(yNote);
	int xNoteInt = round(xNote);

	NoteName realName;
	NoteName* name;
	if (abs(yNoteInt - yNote) < .00001 && abs(xNoteInt - xNote) < .00001)
	//{
	//	realName = temperament.createNoteName(xNoteInt, yNoteInt);
	//	name = &realName;
	//}
	{
		name = temperament.borrowNoteName(xNoteInt, yNoteInt);
	}
	else
	{
		name = &realName;
		name->num = 0;
	}

	D2D1_MATRIX_3X2_F t;
	m_d2dContext->GetTransform(&t);

	if (name->num > 0 && name->denom > 0)
	{
		double cents = temperament.getCents(xNoteInt, yNoteInt);
		if (cents < 0 && cents > -.001)
			cents = 0;


		double ratioCents = 1200 * log2(static_cast<double>(name->num) / name->denom);
		if (abs(cents - ratioCents) > 100)
		{
			m_d2dContext->DrawImage(greyCellBitMap.Get(), D2D1::Point2F(x + lattice.cellMinX- CELLBITMAPMARGIN, y - lattice.cellMinY - CELLBITMAPMARGIN));
		}
		else
		//new way using bitmap should be faster
		if(name->sharps == 0)
		  m_d2dContext->DrawImage(whiteCellBitMap.Get(), D2D1::Point2F(x + lattice.cellMinX - CELLBITMAPMARGIN, y + lattice.cellMinY - CELLBITMAPMARGIN));
		else
		  m_d2dContext->DrawImage(blackCellBitMap.Get(), D2D1::Point2F(x + lattice.cellMinX - CELLBITMAPMARGIN, y + lattice.cellMinY - CELLBITMAPMARGIN));

	//	return hr;

		D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Translation(x, y)*t;
		m_d2dContext->SetTransform(t2);

		// old way of rendering cell path directly
		//		m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pBorderBrush, 2.5);
		//		m_d2dContext->FillGeometry(m_pCellPathGeometry, name.sharps == 0 ? m_pWhiteBrush : m_pBlackBrush);

		if (lattice.cellWidth > 20 && lattice.cellHeight > 20)
		{
			wstring sharpStr;
			if (name->sharps > 0)
			{
				if (name->sharps < 8)
				{
					for (int i = 0; i < name->sharps; i++)
						sharpStr.push_back(L'♯');
				}

			}
			else if (name->sharps < 0)
			{
				if (name->sharps > -8)
					for (int i = 0; i < -name->sharps; i++)
						sharpStr.push_back(L'♭');
			}

			wchar_t strCents[256];
			swprintf_s(strCents, 256, L" %.0f", cents);

			wchar_t str[256];
			unsigned int nameLength = 0;
	//		if (lattice.cellWidth > 80 && lattice.cellHeight > 100)
			{
	//			double ratioCents = 1200 * log2(static_cast<double>(name->num) / name->denom);

				wchar_t strRatio[256];
				swprintf_s(strRatio, 256, L"%d:%d = %.0f",(int) name->num, (int)name->denom, ratioCents);

				wchar_t strDiff[256];
				swprintf_s(strDiff, 256, L"Δ = %.2f", cents - ratioCents);

				//	wchar_t str[256];
				//	swprintf_s(str, 256, L"%s\n%s\n%s\n%s", strName, strCents, strRatio, strDiff);

				swprintf_s(str, 256, L"%s%s\n%d:%d", name->name.c_str(), sharpStr.c_str(),(int) name->num, (int)name->denom);

				nameLength = wcslen(str);

				swprintf_s(str, 256, L"%s\nJI=%.2f\n¢=%.2f\nΔ=%.2f", str, ratioCents, cents, cents - ratioCents);
			}
			//else if (lattice.cellWidth > 75 && lattice.cellHeight > 55)
			//{
			//	swprintf_s(str, 256, L"%s%s\n%d:%d", name->name.c_str(), sharpStr.c_str(), (int)name->num, (int)name->denom);
			//	nameLength = wcslen(str);
			//}
			//else
			//{
			//	swprintf_s(str, 256, L"%s%s", name->name.c_str(), sharpStr.c_str());
			//	nameLength = wcslen(str);
			//}

			if (name->pTextLayout == 0)
			{
				hr = m_dwriteFactory->CreateTextLayout(
					str,
					wcslen(str),
					m_pTextFormat,  // The text format to apply to the string (contains font information, etc).
			//		lattice.cellWidth,         // The width of the layout box.
			//		lattice.cellHeight,        // The height of the layout box.
					200,         // The width of the layout box.
					200,        // The height of the layout box.
					&name->pTextLayout  // The IDWriteTextLayout interface pointer.
					);

				DWRITE_TEXT_RANGE textRange = { nameLength, wcslen(str) - nameLength };

				if (SUCCEEDED(hr))
				{
					hr = name->pTextLayout->SetFontSize(10, textRange);
				}
			}
			name->pTextLayout->GetMaxHeight();
			name->pTextLayout->GetMaxWidth();

			for (int i = 0; i < lattice.textPos.size(); i++)
			{
				m_d2dContext->DrawTextLayout(D2D1::Point2F(lattice.textPos[i](0)*lattice.right(0) + lattice.textPos[i](1)* lattice.down(0) - name->pTextLayout->GetMaxWidth() / 2, lattice.textPos[i](0)*lattice.right(1) + lattice.textPos[i](1)* lattice.down(1) - name->pTextLayout->GetMaxHeight() / 2), name->pTextLayout,

					//		m_pBorderBrush);
					name->sharps == 0 ? m_pBlackBrush : m_pWhiteBrush);
			}
		}
	}
	else
	{

		D2D1_MATRIX_3X2_F t2 = D2D1::Matrix3x2F::Translation(x, y)*t;
		m_d2dContext->SetTransform(t2);

//		m_d2dContext->DrawGeometry(m_pCellPathGeometry, m_pBorderBrush, 2.5);
//		m_d2dContext->FillGeometry(m_pCellPathGeometry, m_pBorderBrush);

		if (lattice.cellWidth > 80 && lattice.cellHeight > 100)
		{
			double cents = getCents(x + originXDraw, y + originYDraw);
			if (cents < 0 && cents > -.001)
				cents = 0;

			wchar_t str[256];
			swprintf_s(str, 256, L"¢=%.2f", cents);

			if (name->pTextLayout == 0)
			{
				hr = m_dwriteFactory->CreateTextLayout(
					str,
					wcslen(str),
					m_pTextFormat,  // The text format to apply to the string (contains font information, etc).
									//		lattice.cellWidth,         // The width of the layout box.
									//		lattice.cellHeight,        // The height of the layout box.
					200,         // The width of the layout box.
					200,        // The height of the layout box.
					&name->pTextLayout  // The IDWriteTextLayout interface pointer.
					);

				DWRITE_TEXT_RANGE textRange = { 0 , wcslen(str) };

				if (SUCCEEDED(hr))
				{
					hr = name->pTextLayout->SetFontSize(10, textRange);
				}
			}

			for (int i = 0; i < lattice.textPos.size(); i++)
			{
				m_d2dContext->DrawTextLayout(D2D1::Point2F(lattice.textPos[i](0)*lattice.right(0) + lattice.textPos[i](1)* lattice.down(0) - name->pTextLayout->GetMaxWidth() / 2, lattice.textPos[i](0)*lattice.right(1) + lattice.textPos[i](1)* lattice.down(1) - name->pTextLayout->GetMaxHeight() / 2), name->pTextLayout,
					//		m_pBorderBrush);
					name->sharps == 0 ? m_pBlackBrush : m_pWhiteBrush);
			}
		}
	}

	m_d2dContext->SetTransform(t);
    return hr;
}


void LatticeView::startPlayingNote(TouchData* touchData)
{
	POINT pos = touchData->pos;
	double originXPix = originXToPix;
	double originYPix = originYToPix;

	double x = pos.x + originXPix;
	double y = pos.y + originYPix;

	if (roundFirstNote)
	{
		lattice.getCell(&x, &y);
		lattice.latticeToCartesian(&x, &y);
	}

	touchData->cellPos = Vector2d(x - originXPix, y - originYPix);
	touchData->cellPosOrig = touchData->cellPos;


	touchData->cents = getCents(x, y);
	touchData->origCents = touchData->cents;
	double freq = frequencyFromCents(touchData->cents);

	int count = 0;
	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* td = &touchDataArray[i];
		double dx = td->pos.x - touchData->pos.x;
		double dy = td->pos.y - touchData->pos.y;
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

		if (count == 0)
			touchData->noteID = instrument.playNote(freq);
	}

	if (touchData->noteID == -1)
	{
		PRINT(_T("Note not played due to no noteID\n"));
	}
}


void LatticeView::updateGlissando(TouchData* touchData)
{
	POINT pos = touchData->pos;
	double originXPix = originXToPix;
	double originYPix = originYToPix;
	double x = pos.x + originXPix;
	double y = pos.y + originYPix;

	lattice.getCell(&x, &y);
	lattice.latticeToCartesian(&x, &y);

	touchData->cellPos(0) = x - originXPix;
	touchData->cellPos(1) = y - originYPix;

	touchData->origCents = getCents(x, y);

	//if (vibrato) 
	//{
	//	updateVibrado(touchData);
	//}
	//else
	{
		touchData->cents = getCents(x, y);

		instrument.updateNoteFreq(touchData->noteID, frequencyFromCents(touchData->cents));

		for (int i = 0; i < touchDataArray.size(); i++)
		{
			TouchData* td = &touchDataArray[i];
			if (td->origCents == touchData->origCents && td != touchData)
			{
				td->cents = touchData->cents;
			}
		}
	}
}

void LatticeView::updateFrequancy(TouchData* touchData)
{
	double x, y;
	double originXPix = originXToPix;
	double originYPix = originYToPix;
	if (usePitchAxis)
	{
		Vector2d dPos(touchData->pos.x - touchData->posStart.x, touchData->pos.y - touchData->posStart.y);
		double d = pitchAxis.dot(dPos);
		x = touchData->cellPosOrig(0) + d*pitchAxis(0) + originXPix;
		y = touchData->cellPosOrig(1) + d*pitchAxis(1) + originYPix;
	}
	else
	{
		x = touchData->cellPosOrig(0) + touchData->pos.x - touchData->posStart.x + originXPix;
		y = touchData->cellPosOrig(1) + touchData->pos.y - touchData->posStart.y + originYPix;
	}


	touchData->cents = getCents(x, y);

	touchData->cellPos(0) = x - originXPix;
	touchData->cellPos(1) = y - originYPix;

	instrument.updateNoteFreq(touchData->noteID, frequencyFromCents(touchData->cents));

	for (int i = 0; i < touchDataArray.size(); i++)
	{
		TouchData* td = &touchDataArray[i];
		if (td->origCents == touchData->origCents && td != touchData)
		{
			td->cents = touchData->cents;
		}
	}
}



// Uses a single pole real time IIR high pass filter.
void LatticeView::updateVibrado(TouchData* touchData)
{
	if (deltaT == 0)
		return;

	double triggerFactor = 1;

	double deltaCents;
	if (usePitchAxis)
	{
		Vector2d dPos(touchData->pos.x - touchData->posStart.x, touchData->pos.y - touchData->posStart.y);
		double d = pitchAxis.dot(dPos);
		Vector2d dPosPrev(touchData->prevVibratoPos.x - touchData->posStart.x, touchData->prevVibratoPos.y - touchData->posStart.y);
		double dPrev = dPosPrev.dot(pitchAxis);
		deltaCents =  getCents(touchData->posStart.x + d*pitchAxis(0), touchData->posStart.y + d*pitchAxis(1)) - getCents(touchData->posStart.x + dPrev*pitchAxis(0), touchData->posStart.y + dPrev*pitchAxis(1));
	}
	else
	{
		deltaCents =  getCents(touchData->pos.x, touchData->pos.y) - getCents(touchData->prevVibratoPos.x, touchData->prevVibratoPos.y);
	}

	if (vibratoTrigger)
	{
		double a = 1;
		if (triggerResetRate != 0)
		{
			double RC = 1 / (2 * PI* triggerResetRate);
			a = RC / (RC + deltaT);
		}

		touchData->trigMax = a*touchData->trigMax;
		if ( deltaCents > touchData->trigMax)
			touchData->trigMax = deltaCents;			

		touchData->trigMin = a*touchData->trigMin;

		if ( deltaCents < touchData->trigMin)
			touchData->trigMin = deltaCents;

		if (touchData->trigMax < 0)
			touchData->trigMax = 0;

		if (touchData->trigMin > 0)
			touchData->trigMin = 0;

//		PRINT((to_wstring(touchData->trigMax) + L", " + to_wstring(touchData->trigMin) + L"\n").c_str());
	}


	{
		double a = 1;
		if (vibratoCuttoffFreq > 0)
		{
			double RC =  1 / (2 * PI* vibratoCuttoffFreq );
			a = RC / (RC + deltaT);
		}

		if (!vibratoTrigger || (touchData->trigMin < -triggerThreshold && touchData->trigMax > triggerThreshold))
			touchData->deltaCents = a*(touchData->deltaCents + vibratoAmplitude * deltaCents);
		else
			touchData->deltaCents = a*touchData->deltaCents;
	}

	touchData->cents = touchData->origCents + touchData->deltaCents;


	touchData->prevVibratoPos.x = touchData->pos.x;
	touchData->prevVibratoPos.y = touchData->pos.y;

	instrument.updateNoteFreq(touchData->noteID, frequencyFromCents(touchData->cents));


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
				if ((time - td->killTime) >= 200) // These linger to detect double taps
				{
					td->clear();
				}
				else
				{
					numActive++;
				}
			}
		}
		else if (td->phase == TOUCH_MOVE && vibrato)
		{
			updateVibrado(td);
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
		}
	}
	else
	{
		//    NSLog(@"touchUp with no touch down.");
	}

	touchData->clear();
}


TouchData* LatticeView::newTouchDown(PointerEventArgs^ args)
{
	POINT pos;
	pos.x = args->CurrentPoint->Position.X/scale;
	pos.y = args->CurrentPoint->Position.Y/scale;

	double originXPix = originXToPix;
	double originYPix = originYToPix;

	double x = pos.x + originXPix;
	double y = pos.y + originYPix;

	Lattice::cartesianToLattice(&x, &y, &lattice.largeGen1, &lattice.largeGen2);

	TouchData* touchData = newTouchData(args->CurrentPoint->PointerId);
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
		double x = touchData->pos.x + originXPix;
		double y = touchData->pos.y + originYPix;

		touchData->bounderyPoint = lattice.getBounderyPoint(x, y);
		if (touchData->bounderyPoint)
		{
			touchData->bounderyPointStart = *touchData->bounderyPoint;
			invalidateCellPath();
		}

		bool text = false;
		for (int i = 0; i < lattice.textCustomLoc.size(); i++)
		{
			if (touchData->bounderyPoint == &lattice.textCustomLoc[i])
				text = true;
		}

		if (text) // check for double tap. if so, create new text
		{
			for (int i = 0; i < touchDataArray.size(); i++)
			{
				TouchData* td = &touchDataArray[i];
				double dx = td->pos.x - touchData->pos.x;
				double dy = td->pos.y - touchData->pos.y;
				if (td->phase == TOUCH_UP && (dx*dx + dy*dy) < touchDiameter*touchDiameter)
				{
					lattice.textCustomLoc.push_back(*touchData->bounderyPoint);
					touchData->bounderyPoint = &lattice.textCustomLoc[lattice.textCustomLoc .size()-1];
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
		else
		{
			touchData->clear();  // youse kill timmer instead to detect for double tap.
		}
        ///////////////////////////////////////



		if (touchData == intervalOriginTouch)
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

	double originXPix = originXToPix;
	double originYPix = originYToPix;

	double tolerance = 50;
	if (playMusic)
	{
		TouchData* touchData = newTouchDown(args);
		if (touchData->pos.x < 20 && touchData->pos.y < 20)
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

    	if (numDown == 1)
		{
			double x = touchData->pos.x + originXPix;
			double y = touchData->pos.y + originYPix;

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
			x -= originXPix;
			y -= originYPix;

			double x1, y1, x2, y2;
			interval1ToCartesian(x1, y1);
			interval2ToCartesian(x2, y2);

			x1 += x;
			y1 += y;
			x2 += x;
			y2 += y;
			if (touchData->pos.x > x1 - tolerance
				&& touchData->pos.x < x1 + tolerance
				&& touchData->pos.y > y1 - tolerance
				&& touchData->pos.y < y1 + tolerance)
			{
				touchData->intervalVector = &generatorVec;
			}
			else if( touchData->pos.x  > x2 - tolerance
				&& touchData->pos.x  < x2 + tolerance
				&& touchData->pos.y > y2 - tolerance
				&&touchData->pos.y < y2 + tolerance)
			{
				touchData->intervalVector = &periodVec;
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

			double dx = touchData->pos.x - touchData->prevMovePos.x;
			double dy = touchData->pos.y - touchData->prevMovePos.y;
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
			else if (dragMode == DragMode::CONTINUUM)
			{
				updateFrequancy(touchData);
			}
			else if (dragMode == DragMode::GLISSANDO)
			{
				updateGlissando(touchData);
			}
			else if (dragMode == DragMode::DRAG)
			{
				drag(touchData);
			}

			//else if (vibrato)
			//{
			//	updateVibrado(touchData);
			//}




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
							Vector2d hat1 = lattice.largeGen1 / lattice.largeGen1.norm();
							Vector2d d1 = lattice.down - lattice.downShift;
							Vector2d d2 = lattice.downShift;
							Vector2d d3 = lattice.down;
							Vector2d r = lattice.largeGen2 - lattice.largeGen2.dot(hat1) * hat1;

							Vector2d d;
							for (int i = 0; i < lattice.cellVerticies1.size(); i++)
							{
								if (touchData->bounderyPoint == &lattice.cellVerticies1[i])
									d = d1;
							}

							for (int i = 0; i < lattice.cellVerticies2.size(); i++)
							{
								if (touchData->bounderyPoint == &lattice.cellVerticies2[i])
									d = d2;
							}
							
							for (int i = 0; i < lattice.cellVerticies3.size(); i++)
							{
								if (touchData->bounderyPoint == &lattice.cellVerticies3[i])
									d = d3;
							}

							for (int i = 0; i < lattice.textCustomLoc.size(); i++)
							{
								if (touchData->bounderyPoint == &lattice.textCustomLoc[i])
									d = d3;
							}

							Vector2d D = Vector2d(touchData->pos.x - touchData->posStart.x, touchData->pos.y - touchData->posStart.y);
							Vector2d rHat = r/r.norm();
							Vector2d dHat = d1/d1.norm();

							Vector2d Dp;
							Dp(0) = D.dot(rHat) / r.dot(rHat);
		                    Dp(1) =  D.dot(dHat) / d.dot(dHat);
							*touchData->bounderyPoint = touchData->bounderyPointStart + Dp;

							invalidateCellPath();
						}
					    else
						{
							drag(touchData);
						}
					}
					else if (cInputs == 2 && this->twoTouchEdit)
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
							double originXPix = touchData1->latticeCoordsStart(0)*lattice.largeGen1(0) + touchData1->latticeCoordsStart(1)*lattice.largeGen2(0) - touchData1->pos.x;
							double originYPix = touchData1->latticeCoordsStart(0)*lattice.largeGen1(1) + touchData1->latticeCoordsStart(1)*lattice.largeGen2(1) - touchData1->pos.y;

							double x1 = intervalOrigin(0);
							double y1 = intervalOrigin(1);
							Lattice::latticeToCartesian(&x1, &y1, &lattice.largeGen1, &lattice.largeGen2);

							double x2 = originXPix + touchData2->pos.x;
							double y2 = originYPix + touchData2->pos.y;

							double x3 = x2 - x1;
							double y3 = y2 - y1;

							touchData2->dragVector(0) = x3;
							touchData2->dragVector(1) = y3;

							int i = 1;
							int minI = 1;

							lattice.getCell(&x3, &y3);
							lattice.latticeToCartesian(&x3, &y3);
							Lattice::cartesianToLattice(&x3, &y3, &lattice.largeGen1, &lattice.largeGen2);
							(*touchData2->intervalVector)(0) = x3;
							(*touchData2->intervalVector)(1) = y3;

							originX = pixToOriginX;
							originY = pixToOriginY;
						}
						else if (this->mode == EDIT_ROTATE_SCALE)
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

							Q = touchData1->pos.x;
							R = touchData2->pos.x;
							S = touchData1->pos.y;
							T = touchData2->pos.y;

							double denom = (A*A - 2 * A*C + B*B - 2 * B*D + C*C + D*D);
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

							if (maybeUpdateLattice(x1, y1, x2, y2))
							{
								double originXPix = (A*A*(-R) + A*C*Q + A*C*R - A*D*S + A*D*T - B*B*R + B*C*S - B*C*T + B*D*Q + B*D*R - C*C*Q - D*D*Q) / denom;
								double originYPix = (A*A*(-T) + A*C*S + A*C*T + A*D*Q - A*D*R - B*B*T - B*C*Q + B*C*R + B*D*S + B*D*T - C*C*S - D*D*S) / denom;

								originX = pixToOriginX;
								originY = pixToOriginY;
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

							Q = touchData1->pos.x;
							R = touchData2->pos.x;
							S = touchData1->pos.y;
							T = touchData2->pos.y;

							double denom = (A*A - 2 * A*C + B*B - 2 * B*D + C*C + D*D);
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

							if (maybeUpdateLattice(x1, y1, x2, y2))
							{
								double originXPix = ((n1 + n2)*lattice.largeGen1(0) + (m1 + m2)*lattice.largeGen2(0) - (Q + R)) / 2;
								double originYPix = ((n1 + n2)*lattice.largeGen1(1) + (m1 + m2)*lattice.largeGen2(1) - (S + T)) / 2;

								originX = pixToOriginX;
								originY = pixToOriginY;
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

							Q = touchData1->pos.x;
							R = touchData2->pos.x;
							S = touchData1->pos.y;
							T = touchData2->pos.y;

							double denom = (A*A - 2 * A*C + B*B - 2 * B*D + C*C + D*D);
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

							if (maybeUpdateLattice(x1, y1, x2, y2))
							{
								double originXPix = ((n1 + n2)*lattice.largeGen1(0) + (m1 + m2)*lattice.largeGen2(0) - (Q + R)) / 2;
								double originYPix = ((n1 + n2)*lattice.largeGen1(1) + (m1 + m2)*lattice.largeGen2(1) - (S + T)) / 2;

								originX = pixToOriginX;
								originY = pixToOriginY;
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
							C = touchData1->pos.x;
							D = touchData2->pos.x;

							s = (C - D) / (A - B);
							double ox = (B*C - A*D) / (A - B);

							//lattice.largeGen1(0) = touchBegin_latticeGen1(0)*s;
							//lattice.largeGen2(0) = touchBegin_latticeGen2(0)*s;

							double x1 = touchBegin_latticeGen1(0)*s;
							double x2 = touchBegin_latticeGen2(0)*s;

							A = n1*touchBegin_latticeGen1(1) + m1*touchBegin_latticeGen2(1);
							B = n2*touchBegin_latticeGen1(1) + m2*touchBegin_latticeGen2(1);
							C = touchData1->pos.y;
							D = touchData2->pos.y;

							s = (C - D) / (A - B);
							double oy = (B*C - A*D) / (A - B);

							//lattice.largeGen1(1) = touchBegin_latticeGen1(1)*s;
							//lattice.largeGen2(1) = touchBegin_latticeGen2(1)*s;

							double y1 = touchBegin_latticeGen1(1)*s;
							double y2 = touchBegin_latticeGen2(1)*s;

							if (maybeUpdateLattice(x1, y1, x2, y2))
							{
								double originXPix = ox;
								double originYPix = oy;

								originX = pixToOriginX;
								originY = pixToOriginY;
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

							n3 = n1+1;
							m3 = m1;

							c1 = touchData1->pos.x;
							c2 = touchData2->pos.x;
							c3 = c1 + touchBegin_latticeGen1(0);

							double denom = (-m2*n1 + m3*n1 - m3*n2 + m2*n3 + m1*n2 - m1*n3);

							x = -(c1*m3*n2 - c1*m2*n3 - c2*m3*n1 + c2*m1*n3 + c3*m2*n1 - c3*m1*n2) / denom;
							y = -(c1*m2 - c1*m3 + c2*m3 - c2*m1 - c3*m2 + c3*m1) / denom;
							z = (c1*n2 - c1*n3 - c2*n1 + c2*n3 + c3*n1 - c3*n2) / denom;

							double x1 = y;
							double x2 = z;
							double ox = -x;

							c1 = touchData1->pos.y;
							c2 = touchData2->pos.y;
							c3 = c1 + touchBegin_latticeGen1(1);

							x = -(c1*m3*n2 - c1*m2*n3 - c2*m3*n1 + c2*m1*n3 + c3*m2*n1 - c3*m1*n2) / denom;
							y = -(c1*m2 - c1*m3 + c2*m3 - c2*m1 - c3*m2 + c3*m1) / denom;
							z = (c1*n2 - c1*n3 - c2*n1 + c2*n3 + c3*n1 - c3*n2) / denom;

							double y1 = y;
							double y2 = z;
							double oy = -x;

							if (maybeUpdateLattice(x1, y1, x2, y2))
							{
								double originXPix = ox;
								double originYPix = oy;

								originX = pixToOriginX;
								originY = pixToOriginY;
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

						c1 = touchData1->pos.x;
						c2 = touchData2->pos.x;
						c3 = touchData3->pos.x;

						double denom = (-m2*n1 + m3*n1 - m3*n2 + m2*n3 + m1*n2 - m1*n3);

						x = -(c1*m3*n2 - c1*m2*n3 - c2*m3*n1 + c2*m1*n3 + c3*m2*n1 - c3*m1*n2) / denom;
						y = -(c1*m2 - c1*m3 + c2*m3 - c2*m1 - c3*m2 + c3*m1) / denom;
						z = (c1*n2 - c1*n3 - c2*n1 + c2*n3 + c3*n1 - c3*n2) / denom;

						double x1 = y;
						double x2 = z;
						double ox = -x;

						c1 = touchData1->pos.y;
						c2 = touchData2->pos.y;
						c3 = touchData3->pos.y;

						x = -(c1*m3*n2 - c1*m2*n3 - c2*m3*n1 + c2*m1*n3 + c3*m2*n1 - c3*m1*n2) / denom;
						y = -(c1*m2 - c1*m3 + c2*m3 - c2*m1 - c3*m2 + c3*m1) / denom;
						z = (c1*n2 - c1*n3 - c2*n1 + c2*n3 + c3*n1 - c3*n2) / denom;

						double y1 = y;
						double y2 = z;
						double oy = -x;

						if (maybeUpdateLattice(x1, y1, x2, y2))
						{
							double originXPix = ox;
							double originYPix = oy;

							originX = pixToOriginX;
							originY = pixToOriginY;
						}
					}
			
					break;
		}
	}
}


bool LatticeView::maybeUpdateLattice(double x1, double y1, double x2, double y2)
{
	double area = fabs(x1*y2 - y1*x2);

	if (area > 200)
	{
		largeGen1(0) = x1;
		largeGen2(0) = x2;
		largeGen1(1) = y1;
		largeGen2(1) = y2;
		invalidateCellPath();
		return true;
	}
	else
	{
		return false;
	}

	//if (abs(x1 - x2) > 50 && abs(y1 - y2) > 50)
	//{
	//	lattice.largeGen1(0) = x1;
	//	lattice.largeGen2(0) = x2;
	//	lattice.largeGen1(1) = y1;
	//	lattice.largeGen2(1) = y2;
	//	return true;
	//}
	//else
	//	return false;
}


void LatticeView::drag(TouchData* touchData)
{
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
				//dx = -touchData->pos.x + touchData->prevMovePos(0);
				//dy = -touchData->pos.y + touchData->prevMovePos(1);

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
		//	dx = -touchData->pos.x + touchData->prevPos(0);
		//	dy = -touchData->pos.y + touchData->prevPos(1);

		//	PRINT(_T("didn include touch : %d\n"), touchData->phase);
		////	PRINT(_T("didn include touch dx = %.02f, dy = %.02f\n"), dx, dy);
		//	numPointers++;
		//	continue;
		//}
	}

	if (numPointers == 0)
		return;


	dx = -touchData->pos.x + touchData->prevMovePos.x;
	dy = -touchData->pos.y + touchData->prevMovePos.y;

	dxAvg += dx;
	dyAvg += dy;

	dxAvg /= numPointers;
	dyAvg /= numPointers;

	double originXPix = originXToPix + dxAvg;
	double originYPix = originYToPix + dyAvg;

	originX = pixToOriginX;
	originY = pixToOriginY;

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

/*
-(void)maybeBeginDrag:(UITouch*)touch
{
	//  [NSTimer scheduledTimerWithTimeInterval:.1 target:self selector:@selector(maybeBeginDragTimeUp:) userInfo:touch repeats:NO];
}


-(void)maybeBeginDragTimeUp : (NSTimer*)timer
{
	UITouch* touch = [timer userInfo];
	if (touch.phase == UITouchPhaseEnded || touch.phase == UITouchPhaseCancelled)
		return;

	TouchData* touchData = [self getTouchDataForTouch : touch];
	if (!touchData.dragConfirmed)
	{
		touchData.dragHorizontal = NO;
		touchData.dragVertical = NO;
		[self startPlayingNote : touch];
		NSLog(@"!!! Timer - to slow to drag");
	}
}


-(void)dragPlay:(NSSet *)touches withEvent : (UIEvent *)event
{
	//  NSLog(@"dragPlay");

	double dx = 0;
	double dy = 0;
	double dxAvg = 0;
	double dyAvg = 0;
	int numPointersHorizontal = 0;
	int numPointersVertical = 0;
	touches = [event touchesForView:self];

	for (UITouch* touch in touches)
	{
		if (touch.phase == UITouchPhaseMoved)
		{
			TouchData* touchData = [self getTouchDataForTouch : touch];
			//      if(touchData.speed < _maxSpeed )
			{
				//        NSLog(@"touchData.speed = %.02fms", touchData.speed);

				CGPoint pos = [touch locationInView : self];
				CGPoint prevPos = [touch previousLocationInView : self];

				if (touchData.dragHorizontal)
				{
					//        if(touchData.dragConfirmed)
					{
						dx = -pos.x + prevPos(0);
						dxAvg += dx;
						numPointersHorizontal++;
					}
					//      else if(touchData.speed > dragConfSpeed)
					//{
					//dx = -pos.x + prevPos(0);
					//dxAvg += dx;
					//numPointersHorizontal++;

					//touchData.dragConfirmed = YES;
					//NSLog(@"quick enough to drag :  %f", touchData.speed);
					//}
					//else
					//{
					//touchData.dragHorizontal = NO;
					//touchData.dragVertical = NO;
					//[self startPlayingNote:touch];
					//NSLog(@"to slow to drag :  %f", touchData.speed);
					//}
				}

				if (touchData.dragVertical)
				{
					//          if(touchData.dragConfirmed)
					{
						dy = -pos.y + prevPos(1);
						dyAvg += dy;
						numPointersVertical++;
					}
					//else if(touchData.speed > dragConfSpeed)
					//{
					//dy = -pos.y + prevPos(1);
					//dyAvg += dy;
					//numPointersVertical++;

					//touchData.dragConfirmed = YES;
					//NSLog(@"quick enough to drag :  %f", touchData.speed);
					//}
					//else
					//{
					//touchData.dragHorizontal = NO;
					//touchData.dragVertical = NO;
					//[self startPlayingNote:touch];
					//NSLog(@"to slow to drag :  %f", touchData.speed);
					//}
				}
			}
			//      else
			//      {
			//        NSLog(@"didn't drag touchData.speed = %.02fms", touchData.speed);
			//        return;
			//      }
		}
	}

	if (numPointersHorizontal > 0)
	{
		dxAvg /= numPointersHorizontal;
		_originX += round(dxAvg*self.contentScaleFactor) / self.contentScaleFactor;

		double dx = -(_originX - _prevOriginX);

		if (dx > 0)
		{
			[self setNeedsDisplayInRect : CGRectMake(self.bounds.size.width - (_backBufferLeft + dx), 0, dx, self.bounds.size.height)];

			if (_backBufferLeft + dx > self.bounds.size.width)
			{
				[self setNeedsDisplayInRect : CGRectMake(self.bounds.size.width - (_backBufferLeft + dx - self.bounds.size.width), 0, dx, self.bounds.size.height)];

			}

			[self.overlayView setNeedsDisplay];
		}
		else if (dx < 0)
		{
			[self setNeedsDisplayInRect : CGRectMake(self.bounds.size.width - _backBufferLeft, 0, -dx, self.bounds.size.height)];

			if (_backBufferLeft + dx < 0)
			{
				[self setNeedsDisplayInRect : CGRectMake(self.bounds.size.width - (_backBufferLeft + dx + self.bounds.size.width), 0, dx, self.bounds.size.height)];
			}

			[self.overlayView setNeedsDisplay];
		}
	}

	if (numPointersVertical  > 0)
	{
		dyAvg /= numPointersVertical;
		_originY += round(dyAvg*self.contentScaleFactor) / self.contentScaleFactor;

		double dy = -(_originY - _prevOriginY);
		if (dy > 0)
		{
			[self setNeedsDisplayInRect : CGRectMake(0, self.bounds.size.height - (_backBufferTop + dy), self.bounds.size.width, dy)];

			if (_backBufferTop + dy > self.bounds.size.height)
			{
				[self setNeedsDisplayInRect : CGRectMake(0, self.bounds.size.height - (_backBufferTop + dy - self.bounds.size.height), self.bounds.size.width, dy)];
			}

			[self.overlayView setNeedsDisplay];
		}
		else if (dy < 0)
		{
			[self setNeedsDisplayInRect : CGRectMake(0, self.bounds.size.height - _backBufferTop, self.bounds.size.width, -dy)];

			if (_backBufferTop + dy < 0)
				[self setNeedsDisplayInRect : CGRectMake(0, self.bounds.size.height - (_backBufferTop + dy + self.bounds.size.height), self.bounds.size.width, dy)];

			[self.overlayView setNeedsDisplay];
		}
	}


}
*/


// returns 1 on sucess, 0 on failer
int LatticeView::loadKeyboard(StorageFile^ file)
{
	critical_section::scoped_lock lock(this->criticalSection);

	IVector<Platform::String^>^ lines = PerformSynchronously(FileIO::ReadLinesAsync(file));

	this->keyboardName = convertFileNameToString(file->Name->Data());
	trimFileExtension(this->keyboardName);

	Vector2d g1(600, 0);
	Vector2d g2(0, 600);

	lattice.textCustomLoc.clear();
	lattice.cellVerticies1.clear();
	lattice.cellVerticies2.clear();
	lattice.cellVerticies3.clear();

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
		else if (!wcscmp(token, _T("origin")))
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
				wstring fileName = token;
				StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
				Concurrency::event synchronizer;  // we need to wait for temperaments to load before we update lattice.
				create_task(localFolder->GetFolderAsync("Temperaments")).then([&synchronizer,fileName, this](StorageFolder^ folder)
				{

					create_task(folder->GetFileAsync(ref new Platform::String(fileName.c_str()))).then([&synchronizer, this](StorageFile^ file)
					{
						temperament.loadTemperament(file);
					}).then([&](task<void> t)
					{
						try
						{
							t.get();
						}
						catch (Platform::Exception^ e)
						{
							// Handle error
							MessageDialog^ dialog = ref new MessageDialog(ref new Platform::String((L"cannot open temperament file: " + fileName).c_str()), "");
							create_task(dialog->ShowAsync());
						}
				    	synchronizer.set();
					}, Concurrency::task_continuation_context::use_arbitrary());
				}, Concurrency::task_continuation_context::use_arbitrary());
				
				synchronizer.wait();
			}
		}
		
	}

	if (lattice.textCustomLoc.size() == 0)
	{
		lattice.textCustomLoc.push_back(Vector2d(0,0));
	}

	if (lattice.cellVerticies1.size() == 0)
	{
		lattice.cellVerticies1.push_back(Vector2d(0, 1));
	}

	if (lattice.cellVerticies2.size() == 0)
	{
		lattice.cellVerticies2.push_back(Vector2d(0, 1));
	}

	maybeUpdateLattice(g1(0), g1(1), g2(0), g2(1));

	return 1;
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

int LatticeView::saveKeyboard()
{
	wstring fileNameBase = convertStringToFileName(keyboardName.c_str());
	wstring fileName = fileNameBase + _T(".kb");

	wstringstream content;

	content << "name " << keyboardName.c_str() << endl;
	content << "origin " << originX << L" " << originY << endl;
	content << "basis1 " << lattice.largeGen1(0) << " " << lattice.largeGen1(1) << endl;
	content << "basis2 " << lattice.largeGen2(0) << " " << lattice.largeGen2(1) << endl;
	if(lattice.keyMode == VORONOI)
		content << "keyShape VORONOI" << endl;
	else if (lattice.keyMode == RECTANGLE)
		content << "keyShape RECTANGLE" << endl;
	else if (lattice.keyMode == CUSTOM)
		content << "keyShape CUSTOM" << endl;

	content << "period " << periodVec(0) << " " << periodVec(1) << endl;
	content << "generator " << generatorVec(0) << " " << generatorVec(1) << endl;

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

	content << "temperament " << convertStringToFileName(temperament.name.c_str()) << L".tpmt" << endl;

	wstring str = content.str();

	StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
	create_task(localFolder->GetFolderAsync("Keyboards")).then([=](StorageFolder^ folder)
	{
		create_task(folder->CreateFileAsync(ref new Platform::String(fileName.c_str()), CreationCollisionOption::ReplaceExisting)).then([=](StorageFile^ file)
		{
			create_task(FileIO::WriteTextAsync(file, ref new Platform::String(str.c_str())));
		});
	});

	return 1;
}
