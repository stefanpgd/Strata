#include "Projects/Strata/SkydomeStage.h"
#include "Graphics/DXObject.h"
#include "Graphics/DXComponents.h"
#include "Graphics/DXUtilities.h"
#include "Graphics/RenderTarget.h"
#include "Framework/Scene.h"

#include "Utilities/Logger.h"
#include <tinyexr.h>

SkydomeStage::SkydomeStage(Scene* activeScene, RenderTarget* modelRenderTarget) 
	: activeScene(activeScene), modelRenderTarget(modelRenderTarget)
{
	skydome = new Model("Assets/Models/Sphere/sphere.gltf");
	skydome->transform.Scale = glm::vec3(-1000.0f);

	LoadSkydome("Assets/Skydomes/sea.exr");
	InitializePipeline();
}

void SkydomeStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	DXDescriptorHeap* SRVHeap = DXAccess::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	commandList->SetGraphicsRootSignature(rootSignature->GetAddress());
	commandList->SetPipelineState(pipeline->GetAddress());

	glm::mat4 modelMatrix = skydome->transform.GetModelMatrix();
	glm::mat4 MVP = activeScene->Camera->GetViewProjectionMatrix() * modelMatrix;

	commandList->SetGraphicsRoot32BitConstants(0, 16, &MVP, 0);
	commandList->SetGraphicsRoot32BitConstants(0, 16, &modelMatrix, 16);

	Mesh* sky = skydome->GetMesh(0);

	commandList->SetGraphicsRootDescriptorTable(1, sky->Textures.Albedo->GetSRV());
	commandList->IASetVertexBuffers(0, 1, &sky->GetVertexBufferView());
	commandList->IASetIndexBuffer(&sky->GetIndexBufferView());

	commandList->DrawIndexedInstanced(sky->GetIndicesCount(), 1, 0, 0, 0);

	modelRenderTarget->CopyToScreenBuffer();
}

void SkydomeStage::LoadSkydome(std::string path)
{
	const char* err = nullptr;
	float* image;

	int width;
	int height;

	int result = LoadEXR(&image, &width, &height, path.c_str(), &err);
	if(result != TINYEXR_SUCCESS)
	{
		std::string error(err);
		LOG(Log::MessageType::Error, "EXR Failed to load:");
		LOG(Log::MessageType::Error, error.c_str());
		assert(false);
	}

	skydomeTexture = new Texture(image, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(float) * 4);
	skydome->GetMesh(0)->Textures.Albedo = skydomeTexture;
}

void SkydomeStage::InitializePipeline()
{
	CD3DX12_DESCRIPTOR_RANGE1 exrTexture[1];
	exrTexture[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); 
	rootParameters[1].InitAsDescriptorTable(1, &exrTexture[0], D3D12_SHADER_VISIBILITY_PIXEL); 

	rootSignature = new DXRootSignature(rootParameters, _countof(rootParameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXPipelineDescription pipelineDescription;
	pipelineDescription.RootSignature = rootSignature;
	pipelineDescription.VertexPath = "Source/Shaders/simpleRasterizer.vertex.hlsl";
	pipelineDescription.PixelPath = "Source/Shaders/skydome.pixel.hlsl";

	pipeline = new DXPipeline(pipelineDescription);
}