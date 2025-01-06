#pragma once
#include "DXCommon.h"

/// <summary>
/// Functionality description of a DXBuffer.
/// based on the settings enabled/disabled, the functionality of the buffer changes.
/// 
/// By default, DXBuffers are treated as Constant Buffers without any extra functionality
/// </summary>
struct DXBufferProperties
{
	std::string name = "";
	bool isConstantBuffer = true;
	bool isUnorderedAccess = false;
	bool isStructuredBuffer = false;
	bool isCPUAccessible = false;
};

/// <summary>
/// A generic buffer class that supports Constant Buffers, Shader Resources and Unordered Access.
/// Should be used whenever large groups of data are required on the GPU, but only buffers that are 
/// technically linear one-dimensional arrays. Such as a material buffer, or a static transform buffer.
/// 
/// So this shouldn't be used for things like 2D textures
/// </summary>
class DXBuffer
{
public:
	DXBuffer(const void* data, unsigned int numberOfElements, unsigned int elementSize);
	DXBuffer(DXBufferProperties properties, const void* data, unsigned int numberOfElements, unsigned int elementSize);

	void UpdateData(const void* data);

	unsigned int GetBufferSize();

	ID3D12Resource* Get();
	ComPtr<ID3D12Resource> GetResource();
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetCBV();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSRV();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetUAV();

private:
	bool BufferTypeValidation();

	void AllocateResource();
	void UploadData(const void* data);
	void CreateDescriptors();

private:
	// Resource info //
	ComPtr<ID3D12Resource> buffer;
	DXBufferProperties bufferProperties;
	D3D12_RESOURCE_STATES resourceState;

	// The data that needs to be allocated sometimes needs to be aligned.
	// For example, with constant buffers (256), this is why the allocation amount
	// and the data that will be copied from the CPU can differ. 
	unsigned int inputDataSize; 
	unsigned int gpuBufferSize;

	// Buffer/element sizes are expressed in bytes
	unsigned int elementSize; 
	unsigned int numberOfElements;

	// Descriptor info //
	unsigned int cbvIndex;
	unsigned int srvIndex;
	unsigned int uavIndex;
};