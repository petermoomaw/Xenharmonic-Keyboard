//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "DirectXHelper.h"

// Helper class that initializes DirectX APIs for both 2D and 3D rendering.
// Some of the code in this class may be omitted if only 2D or only 3D rendering is being used.
ref class DirectXBase abstract
{
internal:
    DirectXBase();
//    virtual void Initialize(Windows::UI::Core::CoreWindow^ window, float dpi);

	void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
	void SetLogicalSize(Windows::Foundation::Size logicalSize);
	void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
	void SetDpi(float dpi);
	void SetCompositionScale(float compositionScaleX, float compositionScaleY);

    void ValidateDevice();
    
    virtual void HandleDeviceLost();
 //   virtual void UpdateForWindowSizeChange();
    virtual void Render() = 0;
    void Trim();
    virtual void Present();

	//Used to achieve lowest output latency
	void WaitOnSwapChain();

	// Device Accessors.
	Windows::Foundation::Size GetOutputSize() const { return m_outputSize; }
	Windows::Foundation::Size GetLogicalSize() const { return m_logicalSize; }

	// D3D Accessors.
	ID3D11Device2*          GetD3DDevice() const { return m_d3dDevice.Get(); }
	ID3D11DeviceContext2*   GetD3DDeviceContext() const { return m_d3dContext.Get(); }
	IDXGISwapChain1*        GetSwapChain() const { return m_swapChain.Get(); }
	D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return m_d3dFeatureLevel; }
	ID3D11RenderTargetView* GetBackBufferRenderTargetView() const { return m_d3dRenderTargetView.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return m_d3dDepthStencilView.Get(); }
	D3D11_VIEWPORT          GetScreenViewport() const { return m_screenViewport; }
	DirectX::XMFLOAT4X4     GetOrientationTransform3D() const { return m_orientationTransform3D; }

	// D2D Accessors.
	ID2D1Factory2*          GetD2DFactory() const { return m_d2dFactory.Get(); }
	ID2D1Device1*           GetD2DDevice() const { return m_d2dDevice.Get(); }
	ID2D1DeviceContext1*    GetD2DDeviceContext() const { return m_d2dContext.Get(); }
	ID2D1Bitmap1*           GetD2DTargetBitmap() const { return m_d2dTargetBitmap.Get(); }
	IDWriteFactory2*        GetDWriteFactory() const { return m_dwriteFactory.Get(); }
	IWICImagingFactory2*    GetWicImagingFactory() const { return m_wicFactory.Get(); }
	D2D1::Matrix3x2F        GetOrientationTransform2D() const { return m_orientationTransform2D; }

    virtual void CreateDeviceIndependentResources();
    virtual void CreateDeviceDependentResources();
    virtual void CreateWindowSizeDependentResources();

protected private:
    DXGI_MODE_ROTATION ComputeDisplayRotation();
//    Platform::Agile<Windows::UI::Core::CoreWindow>  m_window;


internal:
    // DirectX Core Objects. Required for 2D and 3D.
    Microsoft::WRL::ComPtr<ID3D11Device2>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext2>    m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;

    // Direct3D rendering objects. Required for 3D.
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_d3dRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_d3dDepthStencilView;
    D3D11_VIEWPORT                                  m_screenViewport;

    // Direct2D drawing components.
    Microsoft::WRL::ComPtr<ID2D1Factory2>           m_d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device1>            m_d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext1>     m_d2dContext;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1>            m_d2dTargetBitmap;

        // DirectWrite drawing components.
    Microsoft::WRL::ComPtr<IDWriteFactory2>         m_dwriteFactory;
    Microsoft::WRL::ComPtr<IWICImagingFactory2>     m_wicFactory;

    //// Cached renderer properties.
    //D3D_FEATURE_LEVEL                               m_featureLevel;
    //Windows::Foundation::Size                       m_renderTargetSize;
    //Windows::Foundation::Rect                       m_windowBounds;
    //float                                           m_dpi;
    //bool                                            m_windowSizeChangeInProgress;

	// Cached reference to the XAML panel.
	Windows::UI::Xaml::Controls::SwapChainPanel^    m_swapChainPanel;

	// Used to achieve lowest output latency
	HANDLE m_frameLatencyWaitableObject;

	// Cached device properties.
	D3D_FEATURE_LEVEL                               m_d3dFeatureLevel;
	Windows::Foundation::Size                       m_d3dRenderTargetSize;
	Windows::Foundation::Size                       m_outputSize;
	Windows::Foundation::Size                       m_logicalSize;
	Windows::Graphics::Display::DisplayOrientations m_nativeOrientation;
	Windows::Graphics::Display::DisplayOrientations m_currentOrientation;
	float                                           m_dpi;
	float                                           m_compositionScaleX;
	float                                           m_compositionScaleY;

	// Transforms used for display orientation.
	D2D1::Matrix3x2F    m_orientationTransform2D;
	DirectX::XMFLOAT4X4 m_orientationTransform3D;
};
