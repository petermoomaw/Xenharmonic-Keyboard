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

#pragma once

//#include "Enharmonic\LatticeView.h"
#include "Enharmonic\Instrument.h"

ref class LatticeView;

// Renders Direct2D and 3D content on the screen.
namespace SDKTemplate
{
    class Main //: public DX::IDeviceNotify
    {
    public:
	//	Main(const std::shared_ptr<LatticeView>& deviceResources);
		Main(LatticeView^ sceneRenderer);
        ~Main();
        void StartRenderLoop();
        void StopRenderLoop();
       
 

  //  private:
        void Render();

		LatticeView^ latticeView;

        Windows::Foundation::IAsyncAction^ renderLoopWorker;
    };
}