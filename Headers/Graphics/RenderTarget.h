#pragma once

#include "Graphics/DXCommon.h"

class DepthBuffer;

class RenderTarget
{
public:
	RenderTarget(unsigned int width, unsigned int height, 
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool useDepthBuffer = true);

	void Clear();
	void Bind();
	void PrepareAsShaderResource();

	void CopyToScreenBuffer();
	void CopyFromScreenBuffer();
	void CopyFromRenderTarget(RenderTarget* target);

	ComPtr<ID3D12Resource> GetResource();
	D3D12_RESOURCE_STATES GetResourceState();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTV();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSRV();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetUAV();

private:
	void AllocateResource();
	void CreateDescriptors();

private:
	ComPtr<ID3D12Resource> renderTarget;
	D3D12_RESOURCE_STATES resourceState;

	D3D12_RECT scissorRect;
	D3D12_VIEWPORT viewport;

	DepthBuffer* depthBuffer;
	bool useDepthBuffer;

	DXGI_FORMAT format;
	unsigned int width;
	unsigned int height;

	unsigned int rtvIndex;
	unsigned int srvIndex;
	unsigned int uavIndex;
};