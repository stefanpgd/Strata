#include "Projects/HelloCompute/ComputeUVStage.h"
#include "Graphics/DXComponents.h"

ComputeUVStage::ComputeUVStage()
{
	// TODO: Read up again how to pass along UAVs/SRVs, there should be a more concise way
	CD3DX12_DESCRIPTOR_RANGE1 backBufferRange[1];
	backBufferRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

	CD3DX12_ROOT_PARAMETER1 computeUVParameters[1];
	computeUVParameters[0].InitAsDescriptorTable(1, &backBufferRange[0]);

	rootSignature = new DXRootSignature(computeUVParameters, _countof(computeUVParameters), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	computePipeline = new DXComputePipeline(rootSignature, "Source/Shaders/computeUV.compute.hlsl");

	int width = window->GetWindowWidth();
	int height = window->GetWindowHeight();
	unsigned int* textureBuffer = new unsigned int[width * height];
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			textureBuffer[x + y * width] = 0;
		}
	}

	renderBuffer = new Texture(textureBuffer, window->GetWindowWidth(), window->GetWindowHeight());
	delete textureBuffer;
}

void ComputeUVStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	// 1) Present UV coords onto render buffer //
	int dispatchX = window->GetWindowWidth() / 8;
	int dispatchY = window->GetWindowHeight() / 8;
	commandList->SetComputeRootSignature(rootSignature->GetAddress());
	commandList->SetPipelineState(computePipeline->GetAddress());

	commandList->SetComputeRootDescriptorTable(0, renderBuffer->GetUAV());
	commandList->Dispatch(dispatchX, dispatchY, 1);
	
	// 2) Copy render buffer to screen //
	ComPtr<ID3D12Resource> screenBuffer = DXAccess::GetWindow()->GetCurrentScreenBuffer();
	TransitionResource(renderBuffer->GetAddress(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
	TransitionResource(screenBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	
	commandList->CopyResource(screenBuffer.Get(), renderBuffer->GetAddress());
	
	TransitionResource(renderBuffer->GetAddress(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	TransitionResource(screenBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
