#pragma once

#include "Graphics/DXCommon.h"
#include "Graphics/Mesh.h"

// This correspond with the Input Assembler for DXPipeline.
// When the VertexBuffer is Interleaved, the POSITION id will be used
enum VertexBufferLayoutIDs
{
	POSITION, 
	NORMAL,
	TANGENT,
	TEXCOORD0,
	ALL_ID_COUNT 
};

class DXVertexBuffer
{
public:
	DXVertexBuffer(unsigned int verticesCount, bool isPacked);

	void MapToBuffer(void* data, unsigned int stride, VertexBufferLayoutIDs id = VertexBufferLayoutIDs::POSITION);

	ID3D12Resource* GetResource(VertexBufferLayoutIDs id = VertexBufferLayoutIDs::POSITION);
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(VertexBufferLayoutIDs id = VertexBufferLayoutIDs::POSITION);

private:
	void AllocateVertexBuffer(ComPtr<ID3D12Resource>& resource, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, unsigned int stride);

private:
	ComPtr<ID3D12Resource> vertexBuffers[VertexBufferLayoutIDs::ALL_ID_COUNT];
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[VertexBufferLayoutIDs::ALL_ID_COUNT];

	bool isPacked;
	unsigned int verticesCount;
};