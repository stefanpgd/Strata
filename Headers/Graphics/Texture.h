#pragma once

#include "Graphics/DXCommon.h"
#include <string>

/// <summary>
/// A texture should be treated as any sort of 2D GPU Buffer/Resource that will contain any sort of color information.
/// May that be a render of the scene or a normal map for a 3D model. This is it supports UAV, SRV and RTV formats. 
/// </summary>
class Texture
{
public:
	Texture(int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	Texture(void* data, int width, int height, 
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, unsigned int formatSizeInBytes = 4);
	Texture(const std::string& filePath); 

	~Texture();

	int GetWidth();
	int GetHeight();
	DXGI_FORMAT GetFormat();

	int GetSRVIndex();
	int GetUAVIndex();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSRV();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetUAV();

	ID3D12Resource* GetAddress();
	ComPtr<ID3D12Resource> GetResource();
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress();

private:
	void AllocateTexture();
	void UploadData(void* data);

	void CreateDescriptors();

private:
	ComPtr<ID3D12Resource> textureResource;

	DXGI_FORMAT format;
	unsigned int formatSizeInBytes;
	int width;
	int height;

	int srvIndex = 0;
	int uavIndex = 0;
};