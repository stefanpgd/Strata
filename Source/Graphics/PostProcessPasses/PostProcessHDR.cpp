#include "Graphics/PostProcessPasses/PostProcessHDR.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXUtilities.h"
#include <imgui.h>

PostProcessHDR::PostProcessHDR()
{
	InitializePipeline();

	ldrTarget = new RenderTarget(DXAccess::GetWindowWidth(), DXAccess::GetWindowHeight());
}

void PostProcessHDR::Update(float deltaTime)
{
	ImGui::Begin("Post Processing");
	ImGui::SeparatorText("HDR / Tonemapping");
	ImGui::Checkbox("Tonemapping Enabled", &IsEnabled);
	ImGui::DragFloat("Exposure", &exposure, 0.01f, 0.001f, 5.0f);
	ImGui::DragFloat("Gamma", &gamma, 0.01, 1.0f, 4.0f);
	ImGui::Separator();
	ImGui::End();
}

void PostProcessHDR::RecordPass(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	ldrTarget->Bind();
	ldrTarget->Clear();

	sceneOutput->PrepareAsShaderResource();

	commandList->SetGraphicsRootSignature(rootSignature->GetAddress());
	commandList->SetPipelineState(pipeline->GetAddress());

	commandList->SetGraphicsRootDescriptorTable(0, sceneOutput->GetSRV());
	commandList->SetGraphicsRoot32BitConstants(1, 1, &exposure, 0);
	commandList->SetGraphicsRoot32BitConstants(1, 1, &gamma, 1);

	commandList->DrawIndexedInstanced(screenQuad->GetIndicesCount(), 1, 0, 0, 0);

	postProcessTarget->CopyFromRenderTarget(ldrTarget);
}

void PostProcessHDR::InitializePipeline()
{
	CD3DX12_DESCRIPTOR_RANGE1 hdrSceneTexture[1];
	hdrSceneTexture[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(1, &hdrSceneTexture[0], D3D12_SHADER_VISIBILITY_PIXEL); // Scene Render
	rootParameters[1].InitAsConstants(2, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL); // Gamma & Exposure

	rootSignature = new DXRootSignature(rootParameters, _countof(rootParameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXPipelineDescription pipelineDescription;
	pipelineDescription.RootSignature = rootSignature;
	pipelineDescription.VertexPath = "Source/Shaders/postprocess_screen.vertex.hlsl";
	pipelineDescription.PixelPath = "Source/Shaders/postprocess_hdr.pixel.hlsl";

	pipeline = new DXPipeline(pipelineDescription);
}