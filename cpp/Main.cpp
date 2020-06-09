//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "Main.h"
#include "DirectX\DirectXHelper.h"
//#include "D2DRenderer\RectRenderer.h"
#include "Enharmonic\LatticeView.h"

using namespace SDKTemplate;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
Main::Main(LatticeView^ latticeView)
{
	//Instrument test;

	this->latticeView = latticeView;
}

Main::~Main()
{
}

void Main::StartRenderLoop()
{
    // If the animation render loop is already running then do not start another thread.
    if (renderLoopWorker != nullptr && renderLoopWorker->Status == AsyncStatus::Started)
    {
        return;
    }

    // Create a task that will be run on a background thread.
    auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
    {
//        unsigned int fps = 60;
//        unsigned long long  renderTimeDuration = 1000/fps;
 //       unsigned long long nextFrameRenderTime = GetTickCount64() + renderTimeDuration;

        // Calculate the updated frame and render once per vertical blanking interval.
        while (action->Status == AsyncStatus::Started)
        {
            unsigned long long currentTime = GetTickCount64();
            //if(currentTime > nextFrameRenderTime)
            {
//              nextFrameRenderTime = currentTime + renderTimeDuration;

              // Block this thread until the swap chain is finished presenting. Note that it is
              // important to call this before the first Present in order to minimize the latency
              // of the swap chain
               latticeView->WaitOnSwapChain();

              {
                critical_section::scoped_lock lock(latticeView->criticalSection);
                Render();
                latticeView->Present();
               }
            }
        }
    });

    // Run task on a dedicated high priority background thread.
    renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void Main::StopRenderLoop()
{
    renderLoopWorker->Cancel();
}

// Renders the current frame according to the current application state.
void Main::Render()
{
	//try
	//{
		auto context = latticeView->GetD3DDeviceContext();

		// Reset the viewport to target the whole screen.
		auto viewport = latticeView->GetScreenViewport();
		context->RSSetViewports(1, &viewport);

		// Reset render targets to the screen.
		ID3D11RenderTargetView *const targets[1] = { latticeView->GetBackBufferRenderTargetView() };
		context->OMSetRenderTargets(1, targets, latticeView->GetDepthStencilView());

		// Clear the back buffer and depth stencil view.
		context->ClearRenderTargetView(latticeView->GetBackBufferRenderTargetView(), DirectX::Colors::Black);
		context->ClearDepthStencilView(latticeView->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Render the scene objects.
		latticeView->Render();
	//}
	//catch (...)
	//{

	//}
}

