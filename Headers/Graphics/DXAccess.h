#pragma once

class DXDevice;
class DXCommands;
class DXDescriptorHeap;
class Texture;
class Window;

#include "Graphics/DXCommon.h"

namespace DXAccess
{
	DXCommands* GetCommands(D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12Device5> GetDevice();
	
	DXDescriptorHeap* GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int index);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int index);
	
	Window* GetWindow();

	unsigned int GetCurrentScreenBufferIndex();
	Texture* GetDefaultTexture();
}