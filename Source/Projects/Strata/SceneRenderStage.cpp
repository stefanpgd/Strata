#include "Projects/Strata/SceneRenderStage.h"
#include "Graphics/DXComponents.h"
#include "Graphics/DXObject.h"

#include "Framework/Scene.h"
#include "Framework/Mathematics.h"
#include "Utilities/Logger.h"
#include "Graphics/DXUtilities.h"
#include <assert.h>

#include "Graphics/RenderTarget.h"

SceneRenderStage::SceneRenderStage(Scene* scene) : activeScene(scene)
{
	if(!activeScene)
	{
		LOG(Log::MessageType::Error, "Cannot render the scene if 'scene' was never initialized!");
		assert(false);
		return;
	}

	renderTarget = new RenderTarget(DXAccess::GetWindowWidth(), DXAccess::GetWindowHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	InitializePipeline();
}

void SceneRenderStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	DXDescriptorHeap* SRVHeap = DXAccess::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	renderTarget->Bind();
	renderTarget->Clear();

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

void SceneRenderStage::InitializePipeline()
{
	// TODO: Expanding the root signature functionality, similar to the creation of as DXShaderBindingTable
	// Might be very effective at reducing the time of making these pipelines 
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
	pipelineDescription.RenderTargetFormat = renderTarget->GetFormat();

	renderPipeline = new DXPipeline(pipelineDescription);
}