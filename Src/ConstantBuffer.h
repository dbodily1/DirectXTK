//--------------------------------------------------------------------------------------
// File: ConstantBuffer.h
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#pragma once

#include "DirectXHelpers.h"
#include "GraphicsMemory.h"
#include "PlatformHelpers.h"


namespace DirectX
{
	// Constant buffer used to send the view-projection matrices to the shader pipeline.
	__declspec(align(16)) struct ViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 viewProjection[2];
	};

	// Assert that the constant buffer remains 16-byte aligned (best practice).
	static_assert((sizeof(ViewProjectionConstantBuffer) % (sizeof(float) * 4)) == 0, "ViewProjection constant buffer size must be 16-byte aligned (16 bytes is the length of four floats).");


    // Strongly typed wrapper around a D3D constant buffer.
    template<typename T>
    class ConstantBuffer
    {
    public:
        // Constructor.
        ConstantBuffer() = default;
        explicit ConstantBuffer(_In_ ID3D11Device* device)
        {
            Create( device );
        }

        ConstantBuffer(ConstantBuffer const&) = delete;
        ConstantBuffer& operator= (ConstantBuffer const&) = delete;

        #if defined(_XBOX_ONE) && defined(_TITLE)
        void Create(_In_ ID3D11Device* device)
        {
            D3D11_BUFFER_DESC desc = {};

            desc.ByteWidth = sizeof(T);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            Microsoft::WRL::ComPtr<ID3D11DeviceX> deviceX;
            ThrowIfFailed(device->QueryInterface(IID_GRAPHICS_PPV_ARGS(deviceX.GetAddressOf())));

            ThrowIfFailed(deviceX->CreatePlacementBuffer(&desc, nullptr, mConstantBuffer.ReleaseAndGetAddressOf()));

            SetDebugObjectName(mConstantBuffer.Get(), L"DirectXTK");
        }


        // Writes new data into the constant buffer.
        void SetData(_In_ ID3D11DeviceContext* deviceContext, T const& value, void** grfxMemory)
        {
            assert( grfxMemory != 0 );

            void* ptr = GraphicsMemory::Get().Allocate( deviceContext, sizeof(T), 64 );
            assert( ptr != 0 );

            *(T*)ptr = value;

            *grfxMemory = ptr;
        }
        #else
        void Create(_In_ ID3D11Device* device)
        {
            D3D11_BUFFER_DESC desc = {};

            desc.ByteWidth = sizeof(T);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            ThrowIfFailed(
                device->CreateBuffer(&desc, nullptr, mConstantBuffer.ReleaseAndGetAddressOf() )
            );

            SetDebugObjectName(mConstantBuffer.Get(), "DirectXTK");
        }


        // Writes new data into the constant buffer.
        void SetData(_In_ ID3D11DeviceContext* deviceContext, T const& value)
        {
            assert( mConstantBuffer.Get() != 0 );

            D3D11_MAPPED_SUBRESOURCE mappedResource;
            
            ThrowIfFailed(
                deviceContext->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)
            );

            *(T*)mappedResource.pData = value;

            deviceContext->Unmap(mConstantBuffer.Get(), 0);
        }
        #endif

        // Looks up the underlying D3D constant buffer.
        ID3D11Buffer* GetBuffer()
        {
            return mConstantBuffer.Get();
        }


    private:
        // The underlying D3D object.
        Microsoft::WRL::ComPtr<ID3D11Buffer> mConstantBuffer;
    };
}
