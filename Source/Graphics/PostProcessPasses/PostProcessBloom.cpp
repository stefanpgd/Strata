#include "Graphics/PostProcessPasses/PostProcessBloom.h"
#include "Graphics/DXAccess.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXRootSignature.h"
#include "Graphics/DXPipeline.h"
#include <imgui.h>
#include "Utilities/EditorElements.h"

PostProcessBloom::PostProcessBloom()
{
	InitializePipelines();

	unsigned int width = DXAccess::GetWindowWidth();
	unsigned int height = DXAccess::GetWindowHeight();

	tresholdTarget = new RenderTarget(width, height);
	blurTargetFront = new RenderTarget(width, height);
	blurTargetBack = new RenderTarget(width, height);
}

void PostProcessBloom::Update(float deltaTime)
{
	ImGui::Begin("Post Processing");
	ImGui::SeparatorText("Bloom");
	ImGui::Checkbox("Bloom Enabled", &IsEnabled);
	ImGui::DragFloat("Bloom Treshold", &bloomTreshold, 0.01f, 0.0f, 1.0f);
	ImGui::DragInt("Blur Passes", &blurPasses, 0.1, 2, 12);
	ImGui::Separator();
	ImGui::End();
}

void PostProcessBloom::RecordPass(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	// 1) Threshold Pass
	tresholdTarget->Bind();
	tresholdTarget->Clear();

	commandList->SetGraphicsRootSignature(tresholdRootsignature->GetAddress());
	commandList->SetPipelineState(tresholdPipeline->GetAddress());

	commandList->SetGraphicsRootDescriptorTable(0, sceneOutput->GetSRV());
	commandList->SetGraphicsRoot32BitConstants(1, 1, &bloomTreshold, 0);

	commandList->DrawIndexedInstanced(screenQuad->GetIndicesCount(), 1, 0, 0, 0);

	// 2) Blur pass 
	commandList->SetGraphicsRootSignature(blurRootsignature->GetAddress());
	commandList->SetPipelineState(blurPipeline->GetAddress());

	blurTargetBack->Bind();
	blurTargetBack->Clear();

	blurTargetFront->Bind();
	blurTargetFront->Clear();

	RenderTarget* lastBlurTarget = nullptr;
	for(int i = 0; i < blurPasses; i++)
	{
		if(i == 0)
		{
			blurTargetBack->Bind();

			tresholdTarget->PrepareAsShaderResource();
			commandList->SetGraphicsRootDescriptorTable(0, tresholdTarget->GetSRV());
		}
		else
		{
			RenderTarget* input = i % 2 ? blurTargetBack : blurTargetFront;
			RenderTarget* target = i % 2 ? blurTargetFront : blurTargetBack;

			target->Bind();
			target->Clear();

			input->PrepareAsShaderResource();
			commandList->SetGraphicsRootDescriptorTable(0, input->GetSRV());

			lastBlurTarget = target;
		}

		int isHorizontal = i % 2;
		commandList->SetGraphicsRoot32BitConstants(1, 1, &isHorizontal, 0);
		commandList->DrawIndexedInstanced(screenQuad->GetIndicesCount(), 1, 0, 0, 0);
	}

	postProcessTarget->CopyFromRenderTarget(blurTargetBack);

	// 3) Blend pass 
	postProcessTarget->Bind();
	lastBlurTarget->PrepareAsShaderResource();

	commandList->SetGraphicsRootSignature(blendRootsignature->GetAddress());
	commandList->SetPipelineState(blendPipeline->GetAddress());

	commandList->SetGraphicsRootDescriptorTable(0, sceneOutput->GetSRV());
	commandList->SetGraphicsRootDescriptorTable(1, lastBlurTarget->GetSRV());
	commandList->DrawIndexedInstanced(screenQuad->GetIndicesCount(), 1, 0, 0, 0);
}

void PostProcessBloom::InitializePipelines()
{
	CD3DX12_DESCRIPTOR_RANGE1 sceneTexture[1];
	sceneTexture[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Threshold //
	CD3DX12_ROOT_PARAMETER1 tresholdParameters[2];
	tresholdParameters[0].InitAsDescriptorTable(1, &sceneTexture[0], D3D12_SHADER_VISIBILITY_PIXEL); // Scene Render
	tresholdParameters[1].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	tresholdRootsignature = new DXRootSignature(tresholdParameters, _countof(tresholdParameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXPipelineDescription tresholdDescription;
	tresholdDescription.RootSignature = tresholdRootsignature;
	tresholdDescription.VertexPath = "Source/Shaders/postprocess_screen.vertex.hlsl";
	tresholdDescription.PixelPath = "Source/Shaders/postprocess_bloomtreshold.pixel.hlsl";

	tresholdPipeline = new DXPipeline(tresholdDescription);

	// Blur //
	CD3DX12_DESCRIPTOR_RANGE1 tresholdTexture[1];
	tresholdTexture[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	CD3DX12_ROOT_PARAMETER1 blurParameters[2];
	blurParameters[0].InitAsDescriptorTable(1, &sceneTexture[0], D3D12_SHADER_VISIBILITY_PIXEL);
	blurParameters[1].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	blurRootsignature = new DXRootSignature(blurParameters, _countof(blurParameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXPipelineDescription blurDescription;
	blurDescription.RootSignature = blurRootsignature;
	blurDescription.VertexPath = "Source/Shaders/postprocess_screen.vertex.hlsl";
	blurDescription.PixelPath = "Source/Shaders/postprocess_bloomblur.pixel.hlsl";

	blurPipeline = new DXPipeline(blurDescription);

	// Blend //
	CD3DX12_ROOT_PARAMETER1 blendParameters[2];
	blendParameters[0].InitAsDescriptorTable(1, &sceneTexture[0], D3D12_SHADER_VISIBILITY_PIXEL);
	blendParameters[1].InitAsDescriptorTable(1, &tresholdTexture[0], D3D12_SHADER_VISIBILITY_PIXEL);

	blendRootsignature = new DXRootSignature(blendParameters, _countof(blendParameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXPipelineDescription blendDescription;
	blendDescription.RootSignature = blendRootsignature;
	blendDescription.VertexPath = "Source/Shaders/postprocess_screen.vertex.hlsl";
	blendDescription.PixelPath = "Source/Shaders/postprocess_bloomblend.pixel.hlsl";

	blendPipeline = new DXPipeline(blendDescription);
}