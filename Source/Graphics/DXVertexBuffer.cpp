#include "Graphics/DXVertexBuffer.h"
#include "Graphics/DXUtilities.h"

DXVertexBuffer::DXVertexBuffer(unsigned int verticesCount, bool isPacked) :
	verticesCount(verticesCount), isPacked(isPacked)
{
	if(isPacked)
	{
		unsigned int float3Stride = sizeof(float) * 3;
		unsigned int float2Stride = sizeof(float) * 2;

		AllocateVertexBuffer(vertexBuffers[VertexBufferLayoutIDs::POSITION], vertexBufferViews[VertexBufferLayoutIDs::POSITION], float3Stride);
		AllocateVertexBuffer(vertexBuffers[VertexBufferLayoutIDs::NORMAL], vertexBufferViews[VertexBufferLayoutIDs::NORMAL], float3Stride);
		AllocateVertexBuffer(vertexBuffers[VertexBufferLayoutIDs::TANGENT], vertexBufferViews[VertexBufferLayoutIDs::TANGENT], float3Stride);
		AllocateVertexBuffer(vertexBuffers[VertexBufferLayoutIDs::TEXCOORD0], vertexBufferViews[VertexBufferLayoutIDs::TEXCOORD0], float2Stride);
	}
	else
	{
		unsigned int vertexStride = sizeof(Vertex);

		AllocateVertexBuffer(vertexBuffers[VertexBufferLayoutIDs::POSITION], vertexBufferViews[VertexBufferLayoutIDs::POSITION], vertexStride);
	}
}

void DXVertexBuffer::MapToBuffer(void* data, unsigned int stride, VertexBufferLayoutIDs id)
{
	ComPtr<ID3D12Resource>& resource = vertexBuffers[id];
	unsigned int bufferSizeInBytes = stride * verticesCount;

	UINT8* pData;
	resource->Map(0, nullptr, (void**)&pData);
	memcpy(pData, data, bufferSizeInBytes);
	resource->Unmap(0, nullptr);
}

ID3D12Resource* DXVertexBuffer::GetResource(VertexBufferLayoutIDs id)
{
	return vertexBuffers[id].Get();
}

const D3D12_VERTEX_BUFFER_VIEW& DXVertexBuffer::GetVertexBufferView(VertexBufferLayoutIDs id)
{
	return vertexBufferViews[id];
}

void DXVertexBuffer::AllocateVertexBuffer(ComPtr<ID3D12Resource>& vertexBuffer, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, unsigned int stride)
{
	unsigned int bufferSizeInBytes = verticesCount * stride;

	ComPtr<ID3D12Device5> device = DXAccess::GetDevice();
	CD3DX12_HEAP_PROPERTIES heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC instanceResource = CD3DX12_RESOURCE_DESC::Buffer(bufferSizeInBytes, D3D12_RESOURCE_FLAG_NONE);

	ThrowIfFailed(device->CreateCommittedResource(&heapDesc, D3D12_HEAP_FLAG_NONE, &instanceResource,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(vertexBuffer.GetAddressOf())));

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = stride;
	vertexBufferView.SizeInBytes = bufferSizeInBytes;
}