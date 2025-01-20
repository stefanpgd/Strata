#include "Graphics/PostProcessor.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXUtilities.h"

// TEMP //
#include "Graphics/DXRootSignature.h"
#include "Graphics/DXPipeline.h"

DXRootSignature* root;
DXPipeline* pipeline;

PostProcessor::PostProcessor()
{
	InitializeScreenSquad();
	sceneOutput = new RenderTarget(DXAccess::GetWindowWidth(), DXAccess::GetWindowHeight());

	CD3DX12_DESCRIPTOR_RANGE1 diffuseTexture[1];
	diffuseTexture[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, &diffuseTexture[0], D3D12_SHADER_VISIBILITY_PIXEL); // Lighting data

	root = new DXRootSignature(rootParameters, _countof(rootParameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXPipelineDescription pipelineDescription;
	pipelineDescription.RootSignature = root;
	pipelineDescription.VertexPath = "Source/Shaders/postprocess_screen.vertex.hlsl";
	pipelineDescription.PixelPath = "Source/Shaders/postprocess_test.pixel.hlsl";
	
	pipeline = new DXPipeline(pipelineDescription);
}

void PostProcessor::Execute(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	// First copy the current output of the scene, so that it can be used for our post process passes. 
	sceneOutput->CopyFromScreenBuffer();
	sceneOutput->PrepareAsShaderResource();

	commandList->SetGraphicsRootSignature(root->GetAddress());
	commandList->SetPipelineState(pipeline->GetAddress());

	commandList->IASetVertexBuffers(0, 1, &screenQuad->GetVertexBufferView());
	commandList->IASetIndexBuffer(&screenQuad->GetIndexBufferView());

	commandList->SetGraphicsRootDescriptorTable(0, sceneOutput->GetSRV());

	// For every pass.. draw
	// Vignette as test case
	commandList->DrawIndexedInstanced(screenQuad->GetIndicesCount(), 1, 0, 0, 0);
}

void PostProcessor::InitializeScreenSquad()
{
	Vertex v0, v1, v2, v3;
	v0.Position = glm::vec3(-1.0f, -1.0f, 0.0f);	// bottom left
	v1.Position = glm::vec3(1.0f, -1.0f, 0.0f);		// bottom right
	v2.Position = glm::vec3(-1.0f, 1.0f, 0.0f);		// top left
	v3.Position = glm::vec3(1.0f, 1.0f, 0.0f);		// top right

	v0.TextureCoord0 = glm::vec2(0.0f, 1.0f);
	v1.TextureCoord0 = glm::vec2(1.0f, 1.0f);
	v2.TextureCoord0 = glm::vec2(0.0f, 0.0f);
	v3.TextureCoord0 = glm::vec2(1.0f, 0.0f);

	std::vector<Vertex> vertices;
	vertices.push_back(v0);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);

	// CCW
	unsigned int indices[6] = 
	{
		0, 1, 2, 2, 1, 3
	};

	screenQuad = new Mesh(vertices.data(), 4, indices, 6);
}