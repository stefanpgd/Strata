#include "Projects/TinyHybrid/BlendResultStage.h"
#include "Graphics/Texture.h"
#include "Graphics/DXRootSignature.h"
#include "Graphics/DXComponents.h"

BlendResultStage::BlendResultStage(Texture* shadowOutput) : shadowOutput(shadowOutput)
{
	int width = DXAccess::GetWindow()->GetWindowWidth();
	int height = DXAccess::GetWindow()->GetWindowHeight();
	blendedOutput = new Texture(width, height);
	sceneRender = new Texture(width, height);

	CD3DX12_DESCRIPTOR_RANGE1 sceneRenderRange[1];
	sceneRenderRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE1 shadowCaptureRange[1];
	shadowCaptureRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE1 blendBufferRange[1];
	blendBufferRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, &sceneRenderRange[0]); 
	rootParameters[1].InitAsDescriptorTable(1, &shadowCaptureRange[0]);
	rootParameters[2].InitAsDescriptorTable(1, &blendBufferRange[0]);

	rootSignature = new DXRootSignature(rootParameters, _countof(rootParameters));
	computePipeline = new DXComputePipeline(rootSignature, "Source/Shaders/TinyHybrid/blendResults.compute.hlsl");
}

void BlendResultStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	ComPtr<ID3D12Resource> renderTargetBuffer = DXAccess::GetWindow()->GetCurrentScreenBuffer();

	// 1) Copy scene render into usable UAV textures 
	TransitionResource(sceneRender->GetAddress(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(renderTargetBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

	commandList->CopyResource(sceneRender->GetAddress(), renderTargetBuffer.Get());

	TransitionResource(sceneRender->GetAddress(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	TransitionResource(renderTargetBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 2) Run blend pipeline

	int dispatchX = window->GetWindowWidth() / 8;
	int dispatchY = window->GetWindowHeight() / 8;
	commandList->SetComputeRootSignature(rootSignature->GetAddress());
	commandList->SetPipelineState(computePipeline->GetAddress());

	commandList->SetComputeRootDescriptorTable(0, sceneRender->GetUAV());
	commandList->SetComputeRootDescriptorTable(1, shadowOutput->GetUAV());
	commandList->SetComputeRootDescriptorTable(2, blendedOutput->GetUAV());
	commandList->Dispatch(dispatchX, dispatchY, 1);

	// 3) Copy end result to screen & make sure everything is in the right state 
	TransitionResource(sceneRender->GetAddress(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	TransitionResource(renderTargetBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(renderTargetBuffer.Get(), blendedOutput->GetAddress());
	TransitionResource(renderTargetBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
}