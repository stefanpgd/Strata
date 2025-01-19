#pragma once

#include "Graphics/DXCommon.h"

class RenderTarget
{
public:
	RenderTarget(unsigned int width, unsigned int height);

	ComPtr<ID3D12Resource> GetResource();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTV();

private:
	void AllocateResource();
	void CreateDescriptors();

private:
	ComPtr<ID3D12Resource> renderTarget;
	D3D12_RESOURCE_STATES resourceState;

	DXGI_FORMAT format;
	unsigned int width;
	unsigned int height;

	unsigned int rtvIndex;
	unsigned int srvIndex;
	unsigned int uavIndex;
};