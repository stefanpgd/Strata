#include "Projects/Strata/ModelRenderStage.h"
#include "Graphics/DXComponents.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Graphics/Camera.h"

#include "Framework/Scene.h"
#include "Framework/Mathematics.h"
#include "Utilities/Logger.h"
#include <assert.h>

ModelRenderStage::ModelRenderStage(Scene* scene) : activeScene(scene)
{
	if(!activeScene)
	{
		LOG(Log::MessageType::Error, "Cannot render the scene if 'scene' was never initialized!");
		assert(false);
		return;
	}

	InitializePipeline();
}

void ModelRenderStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	DXDescriptorHeap* SRVHeap = DXAccess::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	commandList->SetGraphicsRootSignature(rootSignature->GetAddress());
	commandList->SetPipelineState(renderPipeline->GetAddress());

	const std::vector<Model*>& models = activeScene->GetModels();
	for(Model* model : models)
	{
		glm::mat4 modelMatrix = model->transform.GetModelMatrix();
		glm::mat4 MVP = activeScene->Camera->GetViewProjectionMatrix() * modelMatrix;

		commandList->SetGraphicsRoot32BitConstants(0, 16, &MVP, 0);
		commandList->SetGraphicsRoot32BitConstants(0, 16, &modelMatrix, 16);

		const std::vector<Mesh*> meshes = model->GetMeshes();
		for(Mesh* mesh : meshes)
		{
			commandList->SetGraphicsRootDescriptorTable(1, mesh->Textures.Albedo->GetSRV());

			commandList->IASetVertexBuffers(0, 1, &mesh->GetVertexBufferView());
			commandList->IASetIndexBuffer(&mesh->GetIndexBufferView());
			commandList->DrawIndexedInstanced(mesh->GetIndicesCount(), 1, 0, 0, 0);
		}
	}
}

void ModelRenderStage::InitializePipeline()
{
	CD3DX12_DESCRIPTOR_RANGE1 diffuseTexture[1];
	diffuseTexture[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // MVP, Model
	rootParameters[1].InitAsDescriptorTable(1, &diffuseTexture[0], D3D12_SHADER_VISIBILITY_PIXEL); // Lighting data

	rootSignature = new DXRootSignature(rootParameters, _countof(rootParameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXPipelineDescription pipelineDescription;
	pipelineDescription.RootSignature = rootSignature;
	pipelineDescription.VertexPath = "Source/Shaders/simpleRasterizer.vertex.hlsl";
	pipelineDescription.PixelPath = "Source/Shaders/simpleRasterizer.pixel.hlsl";

	renderPipeline = new DXPipeline(pipelineDescription);
}