#include "Graphics/PostProcessPasses/PostProcessVignette.h"
#include "Graphics/DXRootSignature.h"
#include "Graphics/DXPipeline.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXAccess.h"

#include <imgui.h>

PostProcessVignette::PostProcessVignette()
{
	InitializePipeline();
}

void PostProcessVignette::Update(float deltaTime)
{
	ImGui::Begin("Post Processing");
	ImGui::SeparatorText("Vignette");
	ImGui::Checkbox("Is Enabled", &IsEnabled);
	ImGui::DragFloat("Strength", &strength, 0.01f, 0.0f, 3.0f);
	ImGui::Separator();
	ImGui::End();
}

void PostProcessVignette::RecordPass(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	commandList->SetGraphicsRootSignature(rootSignature->GetAddress());
	commandList->SetPipelineState(pipeline->GetAddress());
	
	commandList->SetGraphicsRootDescriptorTable(0, sceneOutput->GetSRV());
	commandList->SetGraphicsRoot32BitConstants(1, 1, &strength, 0);
}

void PostProcessVignette::InitializePipeline()
{
	CD3DX12_DESCRIPTOR_RANGE1 diffuseTexture[1];
	diffuseTexture[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(1, &diffuseTexture[0], D3D12_SHADER_VISIBILITY_PIXEL); // Scene Render
	rootParameters[1].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	rootSignature = new DXRootSignature(rootParameters, _countof(rootParameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXPipelineDescription pipelineDescription;
	pipelineDescription.RootSignature = rootSignature;
	pipelineDescription.VertexPath = "Source/Shaders/postprocess_screen.vertex.hlsl";
	pipelineDescription.PixelPath = "Source/Shaders/postprocess_vignette.pixel.hlsl";

	pipeline = new DXPipeline(pipelineDescription);
}
