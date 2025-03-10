#include "Graphics/Mesh.h"
#include "Graphics/DXAccess.h"
#include "Graphics/DXUtilities.h"
#include "Graphics/DXR/DXRayTracingUtilities.h"
#include "Graphics/Texture.h"
#include "Graphics/DXBuffer.h"
#include "Graphics/DXCommands.h"
#include "Framework/Mathematics.h"
#include <cassert>

#include "Graphics/Extensions/Mesh_TinyglTF.h"

// TODO: Consider loading in not interleaved, due to being able to load the model data in bulk. 

Mesh::Mesh(tinygltf::Model& model, tinygltf::Primitive& primitive, glm::mat4& transform, bool isRayTracingGeometry) 
	: isRayTracingGeometry(isRayTracingGeometry)
{
	// Geometry Data //
	glTFLoadVertexAttribute(vertices, "POSITION", model, primitive);
	glTFLoadVertexAttribute(vertices, "NORMAL", model, primitive);
	glTFLoadVertexAttribute(vertices, "TANGENT", model, primitive);
	glTFLoadVertexAttribute(vertices, "TEXCOORD_0", model, primitive);
	glTFLoadIndices(indices, model, primitive);

	GenerateTangents();
	glTFApplyNodeTransform(vertices, transform);

	UploadGeometryBuffers();

	if(isRayTracingGeometry)
	{
		SetupGeometryDescription();
		BuildBLAS();
	}

	// Material & Texture Data //
	DXBufferProperties bufferProperties;
	bufferProperties.name = "Material Buffer";
	bufferProperties.isConstantBuffer = false;
	bufferProperties.isCPUAccessible = true;
	bufferProperties.isStructuredBuffer = true;

	materialBuffer = new DXBuffer(bufferProperties, &Material, 1, sizeof(Material));
	LoadTextures(model, primitive);
}

Mesh::Mesh(Vertex* verts, unsigned int vertexCount, unsigned int* indi,
	unsigned int indexCount, bool isRayTracingGeometry) : isRayTracingGeometry(isRayTracingGeometry)
{
	for(int i = 0; i < vertexCount; i++)
	{
		vertices.push_back(verts[i]);
	}

	for(int i = 0; i < indexCount; i++)
	{
		indices.push_back(indi[i]);
	}

	UploadGeometryBuffers();

	if(isRayTracingGeometry)
	{
		SetupGeometryDescription();
		BuildBLAS();
	}
}

void Mesh::UpdateMaterial()
{
	materialBuffer->UpdateData(&Material);
}

void Mesh::UploadGeometryBuffers()
{
	unsigned int vertexBufferSize = sizeof(Vertex) * vertices.size();
	unsigned int indicesBufferSize = sizeof(unsigned int) * indices.size();

	AllocateAndMapResource(vertexBuffer, vertices.data(), vertexBufferSize);
	AllocateAndMapResource(indexBuffer, indices.data(), indicesBufferSize );

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = vertexBufferSize;
	vertexBufferView.StrideInBytes = sizeof(Vertex);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = indicesBufferSize;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	verticesCount = vertices.size();
	indicesCount = indices.size();

	vertices.clear();
	indices.clear();
}

void Mesh::SetupGeometryDescription()
{
	ComPtr<ID3D12Device5> device = DXAccess::GetDevice();
	geometryDescription.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geometryDescription.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDescription.Triangles.Transform3x4 = 0;

	// Vertex Buffer //
	geometryDescription.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
	geometryDescription.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
	geometryDescription.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDescription.Triangles.VertexCount = verticesCount;

	// Index Buffer //
	geometryDescription.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
	geometryDescription.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	geometryDescription.Triangles.IndexCount = indicesCount;
}

void Mesh::LoadTextures(tinygltf::Model& model, tinygltf::Primitive& primitive)
{
	glTFLoadTextureByType(&Textures.Albedo, glTFTextureType::BaseColor, model, primitive);
	// Load others if necessary //
}

void Mesh::BuildBLAS()
{
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

	inputs.pGeometryDescs = &geometryDescription;
	inputs.NumDescs = 1;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE; // there are also other options like 'Fast Build'

	AllocateAccelerationStructureMemory(inputs, blasScratch.GetAddressOf(), blasResult.GetAddressOf());
	BuildAccelerationStructure(inputs, blasScratch, blasResult);
}

void Mesh::GenerateTangents()
{
	Vertex& vertex = vertices[0];

	// Incase the vertex doesn't have the default value of a zero-vector
	// it means that the Tangent attribute was present for the model
	// if not, we need to generate them.
	if(vertex.Tangent != glm::vec3(0.0f))
	{
		return;
	}

	// Grab the average tangent of all triangles in the model //
	for(unsigned int i = 0; i < indices.size(); i += 3)
	{
		Vertex& v0 = vertices[indices[i]];
		Vertex& v1 = vertices[indices[i + 1]];
		Vertex& v2 = vertices[indices[i + 2]];

		glm::vec3 tangent;

		// Edges of triangles //
		glm::vec3 edge1 = v1.Position - v0.Position;
		glm::vec3 edge2 = v2.Position - v0.Position;

		// UV deltas //
		glm::vec2 deltaUV1 = v1.TextureCoord0 - v0.TextureCoord0;
		glm::vec2 deltaUV2 = v2.TextureCoord0 - v0.TextureCoord0;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f;

		v0.Tangent += tangent;
		v1.Tangent += tangent;
		v2.Tangent += tangent;

		v0.Tangent = glm::normalize(v0.Tangent);
		v1.Tangent = glm::normalize(v1.Tangent);
		v2.Tangent = glm::normalize(v2.Tangent);
	}
}
#pragma endregion

#pragma region Getters
const D3D12_VERTEX_BUFFER_VIEW& Mesh::GetVertexBufferView()
{
	return vertexBufferView;
}

const D3D12_INDEX_BUFFER_VIEW& Mesh::GetIndexBufferView()
{
	return indexBufferView;
}

const unsigned int Mesh::GetIndicesCount()
{
	return indicesCount;
}

ID3D12Resource* Mesh::GetVertexBuffer()
{
	return vertexBuffer.Get();
}

ID3D12Resource* Mesh::GetIndexBuffer()
{
	return indexBuffer.Get();
}

D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetMaterialGPUAddress()
{
	return materialBuffer->GetGPUVirtualAddress();
}

D3D12_RAYTRACING_GEOMETRY_DESC Mesh::GetGeometryDescription()
{
	if(isRayTracingGeometry)
	{
		return geometryDescription;
	}

	LOG(Log::MessageType::Error, "Object was NOT marked as Ray Tracing Geometry")
	assert(false);
	return D3D12_RAYTRACING_GEOMETRY_DESC();
}

ID3D12Resource* Mesh::GetBLAS()
{
	return blasResult.Get();
}
#pragma endregion