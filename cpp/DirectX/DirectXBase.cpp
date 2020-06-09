//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "DirectXBase.h"
#include "DirectXHelper.h"
#include <windows.ui.xaml.media.dxinterop.h>

using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;
using namespace SDKTemplate;

// Constants used to calculate screen rotations.
namespace ScreenRotation
{
	// 0-degree Z-rotation
	static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// 90-degree Z-rotation
	static const XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// 180-degree Z-rotation
	static const XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// 270-degree Z-rotation
	static const XMFLOAT4X4 Rotation270(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);
};

// Constructor.
DirectXBase::DirectXBase() :
//    m_windowSizeChangeInProgress(false),
    m_screenViewport(),
    m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1),
    m_d3dRenderTargetSize(),
    m_outputSize(),
    m_logicalSize(),
    m_nativeOrientation(DisplayOrientations::None),
    m_currentOrientation(DisplayOrientations::None),
    m_dpi(-1.0f),
    m_compositionScaleX(1.0f),
    m_compositionScaleY(1.0f)
{
	//CreateDeviceIndependentResources();
	//CreateDeviceDependentResources();
}

//// Initialize the DirectX resources required to run.
//void DirectXBase::Initialize(CoreWindow^ window, float dpi)
//{
//    m_window = window;
//
//    CreateDeviceIndependentResources();
//	CreateDeviceDependentResources();
//    SetDpi(dpi);
//}

//// Recreate all device resources and set them back to the current state.
//void DirectXBase::HandleDeviceLost()
//{
//    // Reset these member variables to ensure that SetDpi recreates all resources.
//    float dpi = m_dpi;
//    m_dpi = -1.0f;
//    m_windowBounds.Width = 0;
//    m_windowBounds.Height = 0;
//    m_swapChain = nullptr;
//
//	CreateDeviceDependentResources();
//    SetDpi(dpi);
//}



// These are the resources required independent of the device.
void DirectXBase::CreateDeviceIndependentResources()
{
    // Initialize Direct2D resources.
    D2D1_FACTORY_OPTIONS options;
    ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
    // If the project is in a debug build, enable Direct2D debugging via SDK Layers.
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    // Initialize the Direct2D Factory.
    DX::ThrowIfFailed(
        D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(ID2D1Factory2),
            &options,
            &m_d2dFactory
            ),
        L"Error Initializing the Direct2D Factory");

    // Initialize the DirectWrite Factory.
    DX::ThrowIfFailed(
        DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory2),
            &m_dwriteFactory
            ),
      L"Error Initializing the DirectWrite Factory.");

    // Initialize the Windows Imaging Component (WIC) Factory.
    DX::ThrowIfFailed(
        CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_wicFactory)
            ),
      L"Error Initializing the Windows Imaging Component.");
}

/*
// These are the resources that depend on the device.
void DirectXBase::CreateDeviceDependentResources()
{
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default. It is required for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    ComPtr<IDXGIDevice> dxgiDevice;

#if defined(_DEBUG)
    if (DX::SdkLayersAvailable())
    {
        // If the project is in a debug build, enable debugging via SDK Layers with this flag.
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
#endif

    // This array defines the set of DirectX hardware feature levels this app will support.
    // Note the ordering should be preserved.
    // Don't forget to declare your application's minimum required feature level in its
    // description.  All applications are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    // Create the Direct3D 11 API device object and a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(
        D3D11CreateDevice(
            nullptr,                    // Specify nullptr to use the default adapter.
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            creationFlags,              // Set debug and Direct2D compatibility flags.
            featureLevels,              // List of feature levels this app can support.
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
            &device,                    // Returns the Direct3D device created.
            &m_featureLevel,            // Returns feature level of device created.
            &context                    // Returns the device immediate context.
            )
        );

    // Get the Direct3D 11.1 API device and context interfaces.
    DX::ThrowIfFailed(
        device.As(&m_d3dDevice)
        );

    DX::ThrowIfFailed(
        context.As(&m_d3dContext)
        );

    // Get the underlying DXGI device of the Direct3D device.
    DX::ThrowIfFailed(
        m_d3dDevice.As(&dxgiDevice)
        );

    // Create the Direct2D device object and a corresponding context.
    DX::ThrowIfFailed(
        m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
        );

    DX::ThrowIfFailed(
        m_d2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_d2dContext
            )
        );
}
*/


// Configures the Direct3D device, and stores handles to it and the device context.
void DirectXBase::CreateDeviceDependentResources()
{
	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	if (DX::SdkLayersAvailable())
	{
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the Direct3D 11 API device object and a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	HRESULT hr = D3D11CreateDevice(
		nullptr,                    // Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
		0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		creationFlags,              // Set debug and Direct2D compatibility flags.
		featureLevels,              // List of feature levels this app can support.
		ARRAYSIZE(featureLevels),   // Size of the list above.
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
		&device,                    // Returns the Direct3D device created.
		&m_d3dFeatureLevel,         // Returns feature level of device created.
		&context                    // Returns the device immediate context.
		);

	if (FAILED(hr))
	{
		// If the initialization fails, fall back to the WARP device.
		// For more information on WARP, see: 
		// http://go.microsoft.com/fwlink/?LinkId=286690
		DX::ThrowIfFailed(
			D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
				0,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&device,
				&m_d3dFeatureLevel,
				&context
				),
      L"Error creating Create a WARP device.");
	}

	// Store pointers to the Direct3D 11.1 API device and immediate context.
	DX::ThrowIfFailed(
		device.As(&m_d3dDevice)
    , L"Error: device.As(&m_d3dDevice)");

	DX::ThrowIfFailed(
		context.As(&m_d3dContext),
    L"Error: context.As(&m_d3dContext)");

	// Create the Direct2D device object and a corresponding context.
	ComPtr<IDXGIDevice3> dxgiDevice;
	DX::ThrowIfFailed(
		m_d3dDevice.As(&dxgiDevice),
    L"Error: m_d3dDevice.As(&dxgiDevice)"		);

	DX::ThrowIfFailed(
		m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice),
    L"Error: m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)"
		);

	DX::ThrowIfFailed(
		m_d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m_d2dContext
			),
    L"Error: m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dContext)"
		);
}
// Helps track the DPI in the helper class.
// This is called in the dpiChanged event handler in the view class.
//void DirectXBase::SetDpi(float dpi)
//{
//    if (dpi != m_dpi)
//    {
//        // Save the DPI of this display in our class.
//        m_dpi = dpi;
//
//        // Update Direct2D's stored DPI.
//        m_d2dContext->SetDpi(m_dpi, m_dpi);
//
//        // Often a DPI change implies a window size change. In some cases Windows will issue
//        // both a size changed event and a DPI changed event. In this case, the resulting bounds
//        // will not change, and the window resize code will only be executed once.
//        UpdateForWindowSizeChange();
//    }
//}



//// This routine is called in the event handler for the view SizeChanged event.
//void DirectXBase::UpdateForWindowSizeChange()
//{
//    // Only handle window size changed if there is no pending DPI change.
//    if (m_dpi != DisplayInformation::GetForCurrentView()->LogicalDpi)
//    {
//        return;
//    }
//
//    if (m_window->Bounds.Width  != m_windowBounds.Width ||
//        m_window->Bounds.Height != m_windowBounds.Height)
//    {
//        m_d2dContext->SetTarget(nullptr);
//        m_d2dTargetBitmap = nullptr;
//        m_d3dRenderTargetView = nullptr;
//        m_d3dDepthStencilView = nullptr;
//        m_windowSizeChangeInProgress = true;
//        CreateWindowSizeDependentResources();
//    }
//}

// These resources need to be recreated every time the window size is changed.
void DirectXBase::CreateWindowSizeDependentResources()
{
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_d3dRenderTargetView = nullptr;
	m_d2dContext->SetTarget(nullptr);
	m_d2dTargetBitmap = nullptr;
	m_d3dDepthStencilView = nullptr;
	m_d3dContext->Flush();

	// Calculate the necessary swap chain and render target size in pixels.
	m_outputSize.Width = m_logicalSize.Width * m_compositionScaleX;
	m_outputSize.Height = m_logicalSize.Height * m_compositionScaleY;

	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	if (m_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(
			2, // Single-buffered swap chain.
			lround(m_d3dRenderTargetSize.Width),
			lround(m_d3dRenderTargetSize.Height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT //Enables GetFrameLatencyWaitableObject()
			);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			HandleDeviceLost();

			// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
			// and correctly set up the new device.
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr, L"Error: hr != DXGI_ERROR_DEVICE_REMOVED && hr != DXGI_ERROR_DEVICE_RESET");
		}
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

		swapChainDesc.Width = lround(m_d3dRenderTargetSize.Width); // Match the size of the window.
		swapChainDesc.Height = lround(m_d3dRenderTargetSize.Height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
																	 //swapChainDesc.Flags = 0;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT; // Enable GetFrameLatencyWaitableObject()
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
		ComPtr<IDXGIDevice3> dxgiDevice;
		DX::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice),
      L"Error: m_d3dDevice.As(&dxgiDevice)"
			);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter),
      L"Error: dxgiDevice->GetAdapter(&dxgiAdapter)"
			);

		ComPtr<IDXGIFactory2> dxgiFactory;
		DX::ThrowIfFailed(
			dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)),
      L"Error: dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))"
			);

		// When using XAML interop, the swap chain must be created for composition.
		DX::ThrowIfFailed(
			dxgiFactory->CreateSwapChainForComposition(
				m_d3dDevice.Get(),
				&swapChainDesc,
				nullptr,
				&m_swapChain
				),
      L"Error: dxgiFactory->CreateSwapChainForComposition(...)"
			);

		// Associate swap chain with SwapChainPanel
		// UI changes will need to be dispatched back to the UI thread
		m_swapChainPanel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]()
		{
			// Get backing native interface for SwapChainPanel
			ComPtr<ISwapChainPanelNative> panelNative;
			DX::ThrowIfFailed(
				reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative)),
        L"Error: reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative))"
				);

			DX::ThrowIfFailed(
				panelNative->SetSwapChain(m_swapChain.Get()),
        L"Error: panelNative->SetSwapChain(m_swapChain.Get()))");
		}, CallbackContext::Any));

		ComPtr<IDXGISwapChain2> swapChain2;
		DX::ThrowIfFailed(m_swapChain.As(&swapChain2),
    L"Error: X::ThrowIfFailed(m_swapChain.As(&swapChain2)");

		// Get the frame latency waitable object, which is used by the WaitOnSwapChain method. This
		// requires that swap chain be created with the DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
		// flag.
		m_frameLatencyWaitableObject = swapChain2->GetFrameLatencyWaitableObject();
	}

	// Set the proper orientation for the swap chain, and generate 2D and
	// 3D matrix transformations for rendering to the rotated swap chain.
	// Note the rotation angle for the 2D and 3D transforms are different.
	// This is due to the difference in coordinate spaces.  Additionally,
	// the 3D matrix is specified explicitly to avoid rounding errors.

	switch (displayRotation)
	{
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform2D = Matrix3x2F::Identity();
		m_orientationTransform3D = ScreenRotation::Rotation0;
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform2D =
			Matrix3x2F::Rotation(90.0f) *
			Matrix3x2F::Translation(m_logicalSize.Height, 0.0f);
		m_orientationTransform3D = ScreenRotation::Rotation270;
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform2D =
			Matrix3x2F::Rotation(180.0f) *
			Matrix3x2F::Translation(m_logicalSize.Width, m_logicalSize.Height);
		m_orientationTransform3D = ScreenRotation::Rotation180;
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
               m_orientationTransform2D = 
			Matrix3x2F::Rotation(270.0f) *
			Matrix3x2F::Translation(0.0f, m_logicalSize.Width);
		m_orientationTransform3D = ScreenRotation::Rotation90;
		break;

	default:
		throw ref new FailureException();
	}

	DX::ThrowIfFailed(
		m_swapChain->SetRotation(displayRotation),
   L"Error: m_swapChain->SetRotation(displayRotation)"
		);

	//Setup inverse scale on the swap chain
	DXGI_MATRIX_3X2_F inverseScale = { 0 };
	inverseScale._11 = 1.0f / m_compositionScaleX;
	inverseScale._22 = 1.0f / m_compositionScaleY;
	ComPtr<IDXGISwapChain2> spSwapChain2;
	DX::ThrowIfFailed(
		m_swapChain.As<IDXGISwapChain2>(&spSwapChain2),
    L"Error: m_swapChain.As<IDXGISwapChain2>(&spSwapChain2)"
		);

	DX::ThrowIfFailed(
		spSwapChain2->SetMatrixTransform(&inverseScale),
    L"Error: spSwapChain2->SetMatrixTransform(&inverseScale)"
		);

	// Create a render target view of the swap chain back buffer.
	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)),
    L"Error: m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))"
		);

	DX::ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&m_d3dRenderTargetView
			),
      L"m_d3dDevice->CreateRenderTargetView(...)"
		);

	// Create a depth stencil view for use with 3D rendering if needed.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		lround(m_d3dRenderTargetSize.Width),
		lround(m_d3dRenderTargetSize.Height),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
		);

	ComPtr<ID3D11Texture2D> depthStencil;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			&depthStencil
			),
      L"m_d3dDevice->CreateTexture2D()"
		);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
			depthStencil.Get(),
			&depthStencilViewDesc,
			&m_d3dDepthStencilView
			)
    ,
    L"m_d3dDevice->CreateDepthStencilView(...)"
		);

	// Set the 3D rendering viewport to target the entire window.
	m_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		m_d3dRenderTargetSize.Width,
		m_d3dRenderTargetSize.Height
		);

	m_d3dContext->RSSetViewports(1, &m_screenViewport);

	// Create a Direct2D target bitmap associated with the
	// swap chain back buffer and set it as the current target.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			m_dpi,
			m_dpi
			);

	ComPtr<IDXGISurface2> dxgiBackBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)),
    L"m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))"
		);

	DX::ThrowIfFailed(
		m_d2dContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			&m_d2dTargetBitmap
			),
    L"m_d2dContext->CreateBitmapFromDxgiSurface(...)"
		);

	m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());

	// Grayscale text anti-aliasing is recommended for all Windows Store apps.
	m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

// This method is called when the XAML control is created (or re-created).
void DirectXBase::SetSwapChainPanel(SwapChainPanel^ panel)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_swapChainPanel = panel;
	m_logicalSize = Windows::Foundation::Size(static_cast<float>(panel->ActualWidth), static_cast<float>(panel->ActualHeight));
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_compositionScaleX = panel->CompositionScaleX;
	m_compositionScaleY = panel->CompositionScaleY;
	m_dpi = currentDisplayInformation->LogicalDpi;
	m_d2dContext->SetDpi(m_dpi, m_dpi);

	CreateWindowSizeDependentResources();
}

// This method is called in the event handler for the SizeChanged event.
void DirectXBase::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DpiChanged event.
void DirectXBase::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;
		m_d2dContext->SetDpi(m_dpi, m_dpi);
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the OrientationChanged event.
void DirectXBase::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the CompositionScaleChanged event.
void DirectXBase::SetCompositionScale(float compositionScaleX, float compositionScaleY)
{
	if (m_compositionScaleX != compositionScaleX ||
		m_compositionScaleY != compositionScaleY)
	{
		m_compositionScaleX = compositionScaleX;
		m_compositionScaleY = compositionScaleY;
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DisplayContentsInvalidated event.
void DirectXBase::ValidateDevice()
{
    // The D3D Device is no longer valid if the default adapter changed since the device
    // was created or if the device has been removed.

    // First, get the information for the default adapter from when the device was created.

    ComPtr<IDXGIDevice3> dxgiDevice;
    DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice),
    L"m_d3dDevice.As(&dxgiDevice)");

    ComPtr<IDXGIAdapter> deviceAdapter;
    DX::ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter),
    L"dxgiDevice->GetAdapter(&deviceAdapter)");

    ComPtr<IDXGIFactory2> deviceFactory;
    DX::ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory))
    , L"Error: deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory))");

    ComPtr<IDXGIAdapter1> previousDefaultAdapter;
    DX::ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter),
      L"Error: deviceFactory->EnumAdapters1(0, &previousDefaultAdapter)");

    DXGI_ADAPTER_DESC previousDesc;
    DX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc),
      L"Error: deviceFactory->EnumAdapters1(0, &previousDefaultAdapter)");

    // Next, get the information for the current default adapter.

    ComPtr<IDXGIFactory2> currentFactory;
    DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)),
    L"Error: CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory))");

    ComPtr<IDXGIAdapter1> currentDefaultAdapter;
    DX::ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter),
    L"Error: currentFactory->EnumAdapters1(0, &currentDefaultAdapter)");

    DXGI_ADAPTER_DESC currentDesc;
    DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc),
    L"DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc)");

    // If the adapter LUIDs don't match, or if the device reports that it has been removed,
    // a new D3D device must be created.

    if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
        previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
        FAILED(m_d3dDevice->GetDeviceRemovedReason()))
    {
        // Release references to resources related to the old device.
        dxgiDevice = nullptr;
        deviceAdapter = nullptr;
        deviceFactory = nullptr;
        previousDefaultAdapter = nullptr;

        // Create a new device and swap chain.
        HandleDeviceLost();
    }
}

//// Recreate all device resources and set them back to the current state.
void DirectXBase::HandleDeviceLost()
{
	m_swapChain = nullptr;

	//if (m_deviceNotify != nullptr)
	//{
	//	// Notify the renderers that device resources need to be released.
	//	// This ensures all references to the existing swap chain are released so that a new one can be created.
	//	m_deviceNotify->OnDeviceLost();
	//}

	// Create the new device and swap chain.
	CreateDeviceDependentResources();
	m_d2dContext->SetDpi(m_dpi, m_dpi);
	CreateWindowSizeDependentResources();

	//if (m_deviceNotify != nullptr)
	//{
	//	// Notify the renderers that resources can now be created again.
	//	m_deviceNotify->OnDeviceRestored();
	//}
}

// Call this method when the app suspends to hint to the driver that the app is entering an idle state
// and that its memory can be used temporarily for other apps.
void DirectXBase::Trim()
{
    ComPtr<IDXGIDevice3> dxgiDevice;
    m_d3dDevice.As(&dxgiDevice);

    dxgiDevice->Trim();
}

// Present the contents of the swap chain to the screen.
void DirectXBase::Present()
{
////////////////////////////
// Peter added this in. Got an occurance of INVALID_POINTER_READ_c0000005_Enharmonic.exe!DirectXBase::Present
// Hope that this will deal with that issue.
   if(m_swapChain == nullptr)  
   {
     LOG(L"Recovery: m_swapChain == nullptr in Present");
     HandleDeviceLost();
   }
///////////////////////////////////////

    // The application may optionally specify "dirty" or "scroll" rects to improve efficiency
    // in certain scenarios.  In this sample, however, we do not utilize those features.
    DXGI_PRESENT_PARAMETERS parameters = {0};
    parameters.DirtyRectsCount = 0;
    parameters.pDirtyRects = nullptr;
    parameters.pScrollRect = nullptr;
    parameters.pScrollOffset = nullptr;

    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

    // Discard the contents of the render target.
    // This is a valid operation only when the existing contents will be entirely
    // overwritten. If dirty or scroll rects are used, this call should be removed.
    m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

    // Discard the contents of the depth stencil.
    m_d3dContext->DiscardView(m_d3dDepthStencilView.Get());

    // If the device was removed either by a disconnection or a driver upgrade, we 
    // must recreate all device resources.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        HandleDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr, L"hr != DXGI_ERROR_DEVICE_REMOVED && hr != DXGI_ERROR_DEVICE_RESET");
    }

    //if (m_windowSizeChangeInProgress)
    //{
    //    // A window size change has been initiated and the app has just completed presenting
    //    // the first frame with the new size. Notify the resize manager so we can short
    //    // circuit any resize animation and prevent unnecessary delays.
    //    CoreWindowResizeManager::GetForCurrentView()->NotifyLayoutCompleted();
    //    m_windowSizeChangeInProgress = false;
    //}
}

//Block the current thread until the swap chain has finished presenting.
void DirectXBase::WaitOnSwapChain()
{
	DWORD result = WaitForSingleObjectEx(
		m_frameLatencyWaitableObject,
		INFINITE, // Infinite timeout (shouldn't ever occur)
		true
		);
}

// This method determines the rotation between the display device's native Orientation and the
// current display orientation.
DXGI_MODE_ROTATION DirectXBase::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// Note: NativeOrientation can only be Landscape or Portrait even though
	// the DisplayOrientations enum has other values.
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}