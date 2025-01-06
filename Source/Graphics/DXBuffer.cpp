#include "Graphics/DXBuffer.h"
#include "Graphics/DXUtilities.h"
#include "Graphics/DXDescriptorHeap.h"
#include <string>

DXBuffer::DXBuffer(const void* data, unsigned int numberOfElements, unsigned int elementSize)
	: numberOfElements(numberOfElements), elementSize(elementSize)
{
	if(BufferTypeValidation())
	{
		AllocateResource();
		UploadData(data);
		CreateDescriptors();
	}
	else
	{
		LOG(Log::MessageType::Debug, "Buffer couldn't be created due to above mentioned issues.");
	}
}

DXBuffer::DXBuffer(DXBufferProperties properties, const void* data, unsigned int numberOfElements, unsigned int elementSize)
	: bufferProperties(properties), numberOfElements(numberOfElements), elementSize(elementSize)
{
	if(BufferTypeValidation())
	{
		AllocateResource();
		UploadData(data);
		CreateDescriptors();
	}
	else
	{
		LOG(Log::MessageType::Debug, "Buffer couldn't be created due to above mentioned issues.");
	}
}

void DXBuffer::UpdateData(const void* data)
{
	if(bufferProperties.isCPUAccessible)
	{
		UINT8* pData;
		buffer->Map(0, nullptr, (void**)&pData);
		memcpy(pData, data, inputDataSize);
		buffer->Unmap(0, nullptr);
	}
	else
	{
		LOG(Log::MessageType::Error, "Trying to update data to a non-CPU accessible buffer");
	}
}

bool DXBuffer::BufferTypeValidation()
{
	if(bufferProperties.isCPUAccessible && bufferProperties.isUnorderedAccess)
	{
		LOG(Log::MessageType::Error, "Buffers with Unordered Access cannot reside on either the UPLOAD or READBACK gpu heap");
		return false;
	}

	return true;
}

void DXBuffer::AllocateResource()
{
	// 1) Determine how much data we will be needing
	inputDataSize = numberOfElements * elementSize;
	gpuBufferSize = inputDataSize;

	if(bufferProperties.isConstantBuffer)
	{
		// Constant buffers need to be 256-byte aligned. This ensures that we meet that condition.
		gpuBufferSize = ALIGN(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, inputDataSize);
	}

	D3D12_RESOURCE_DESC bufferDescription = CD3DX12_RESOURCE_DESC::Buffer(gpuBufferSize);
	if(bufferProperties.isUnorderedAccess)
	{
		// Unordered access needs to be marked in the resource, not in the descriptor
		bufferDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	// 2) Create commited resources 
	ComPtr<ID3D12Device5> device = DXAccess::GetDevice();
	CD3DX12_HEAP_PROPERTIES gpuHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if(bufferProperties.isCPUAccessible)
	{
		gpuHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	}

	resourceState = D3D12_RESOURCE_STATE_COMMON;
	if(bufferProperties.isCPUAccessible)
	{
		// Resources that get created on the UPLOAD heap must use the state: GENERIC_READ.
		resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	ThrowIfFailed(device->CreateCommittedResource(&gpuHeap, D3D12_HEAP_FLAG_NONE, &bufferDescription,
		resourceState, nullptr, IID_PPV_ARGS(&buffer)));

	// 3) Name resource (for debugging purposes)
	if(!bufferProperties.name.empty())
	{
		std::wstring resourceName = std::wstring(bufferProperties.name.begin(), bufferProperties.name.end());
		buffer->SetName(resourceName.c_str());
	}
}

void DXBuffer::UploadData(const void* data)
{
	if(bufferProperties.isCPUAccessible)
	{
		UINT8* resourcePtr;

		buffer->Map(0, nullptr, (void**)&resourcePtr);
		memcpy(resourcePtr, data, inputDataSize);
		buffer->Unmap(0, nullptr);
	}
	else
	{
		ComPtr<ID3D12Device5> device = DXAccess::GetDevice();
		DXCommands* directCommands = DXAccess::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT);
		ComPtr<ID3D12GraphicsCommandList4> commandList = directCommands->GetGraphicsCommandList();

		D3D12_HEAP_PROPERTIES intermediateGPUHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_HEAP_PROPERTIES targetGPUHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC bufferDescription = CD3DX12_RESOURCE_DESC::Buffer(gpuBufferSize);

		// First allocate an intermediate location for our buffer.
		// For the subresource amount, we enter 1, since we don't need extra subresources (e.g. mips) since it's a buffer
		ComPtr<ID3D12Resource> intermediateBuffer;
		unsigned int intermediateSize = GetRequiredIntermediateSize(buffer.Get(), 0, 1);
		ThrowIfFailed(device->CreateCommittedResource(&intermediateGPUHeap, D3D12_HEAP_FLAG_NONE,
			&bufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&intermediateBuffer)));

		D3D12_SUBRESOURCE_DATA subresourceDescription;
		subresourceDescription.pData = data;
		subresourceDescription.RowPitch = gpuBufferSize;

		directCommands->Flush();
		directCommands->ResetCommandList();

		TransitionResource(buffer.Get(), resourceState, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources(commandList.Get(), buffer.Get(), intermediateBuffer.Get(), 0, 0, 1, &subresourceDescription);
		TransitionResource(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceState);

		directCommands->ExecuteCommandList();
		directCommands->Signal();
		directCommands->WaitForFenceValue();
	}
}

void DXBuffer::CreateDescriptors()
{
	ComPtr<ID3D12Device5> device = DXAccess::GetDevice();
	DXDescriptorHeap* descriptorHeap = DXAccess::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Constant Buffer View //
	if(bufferProperties.isConstantBuffer)
	{
		cbvIndex = descriptorHeap->GetNextAvailableIndex();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDescription = {};
		cbvDescription.BufferLocation = buffer->GetGPUVirtualAddress();
		cbvDescription.SizeInBytes = gpuBufferSize;

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUHandleAt(cbvIndex);
		device->CreateConstantBufferView(&cbvDescription, handle);
	}

	// Shader Resources View //
	if(bufferProperties.isStructuredBuffer)
	{
		srvIndex = descriptorHeap->GetNextAvailableIndex();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
		srvDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDescription.Format = DXGI_FORMAT_UNKNOWN;
		srvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		srvDescription.Buffer.NumElements = numberOfElements;
		srvDescription.Buffer.StructureByteStride = elementSize;
		srvDescription.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUHandleAt(srvIndex);
		device->CreateShaderResourceView(buffer.Get(), &srvDescription, handle);
	}

	// Unordered Access View //
	if(bufferProperties.isUnorderedAccess)
	{
		uavIndex = descriptorHeap->GetNextAvailableIndex();

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescription = {};
		uavDescription.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDescription.Format = DXGI_FORMAT_UNKNOWN;
		uavDescription.Buffer.NumElements = numberOfElements;
		uavDescription.Buffer.StructureByteStride = elementSize;
		uavDescription.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUHandleAt(uavIndex);
		device->CreateUnorderedAccessView(buffer.Get(), nullptr, &uavDescription, handle);
	}
}

#pragma region Getters
unsigned int DXBuffer::GetBufferSize()
{
	return gpuBufferSize;
}

ID3D12Resource* DXBuffer::Get()
{
	return buffer.Get();
}

ComPtr<ID3D12Resource> DXBuffer::GetResource()
{
	return buffer;
}

D3D12_GPU_VIRTUAL_ADDRESS DXBuffer::GetGPUVirtualAddress()
{
	return buffer->GetGPUVirtualAddress();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DXBuffer::GetCBV()
{
	if(bufferProperties.isConstantBuffer)
	{
		return DXAccess::GetGPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvIndex);
	}

	LOG(Log::MessageType::Debug, "Trying to fetch for a CBV without having one setup. Check the properties of the buffer");
	return CD3DX12_GPU_DESCRIPTOR_HANDLE();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DXBuffer::GetSRV()
{
	if(bufferProperties.isStructuredBuffer)
	{
		return DXAccess::GetGPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srvIndex);
	}

	LOG(Log::MessageType::Debug, "Trying to fetch for a SRV without having one setup. Check the properties of the buffer");
	return CD3DX12_GPU_DESCRIPTOR_HANDLE();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DXBuffer::GetUAV()
{
	if(bufferProperties.isUnorderedAccess)
	{
		return DXAccess::GetGPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, uavIndex);
	}

	LOG(Log::MessageType::Debug, "Trying to fetch for a UAV without having one setup. Check the properties of the buffer");
	return CD3DX12_GPU_DESCRIPTOR_HANDLE();
}
#pragma endregion