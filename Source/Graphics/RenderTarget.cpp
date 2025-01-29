#include "Graphics/RenderTarget.h"
#include "Graphics/DXAccess.h"
#include "Graphics/DXDescriptorHeap.h"
#include "Graphics/DXUtilities.h"
#include "Graphics/DepthBuffer.h"

RenderTarget::RenderTarget(unsigned int width, unsigned int height, DXGI_FORMAT format, bool useDepthBuffer) 
	: width(width), height(height), format(format), useDepthBuffer(useDepthBuffer)
{
	// Default/initial state
	resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	AllocateResource();
	CreateDescriptors();

	if(this->useDepthBuffer)
	{
		depthBuffer = new DepthBuffer(this->width, this->height);
	}

	scissorRect = CD3DX12_RECT(0, 0, width, height);
	viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
}

void RenderTarget::Clear()
{
	if(resourceState != D3D12_RESOURCE_STATE_RENDER_TARGET)
	{
		LOG(Log::MessageType::Error, "You are trying to clear target without it being in the right state. Bind() it first.");
		return;
	}

	const float renderTargetColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	
	DXCommands* directCommands = DXAccess::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ComPtr<ID3D12GraphicsCommandList4> commandList = directCommands->GetGraphicsCommandList();
	commandList->ClearRenderTargetView(GetRTV(), renderTargetColor, 0, nullptr);

	if(useDepthBuffer)
	{
		commandList->ClearDepthStencilView(depthBuffer->GetDSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}

void RenderTarget::Bind()
{
	// TODO: Slowly noticing more that I need a ResourceState tracker.
	// slowly just continue to use this, then once enough examples exist make a system
	if(resourceState != D3D12_RESOURCE_STATE_RENDER_TARGET)
	{
		TransitionResource(renderTarget.Get(), resourceState, D3D12_RESOURCE_STATE_RENDER_TARGET);
		resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	DXCommands* directCommands = DXAccess::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ComPtr<ID3D12GraphicsCommandList4> commandList = directCommands->GetGraphicsCommandList();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRTV();

	if(useDepthBuffer)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle = depthBuffer->GetDSV();
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	}
	else
	{
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	}

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

void RenderTarget::PrepareAsShaderResource()
{
	if(resourceState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
	{
		TransitionResource(renderTarget.Get(), resourceState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		resourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}
}

/// <summary>
/// Copies the current render on this render target directly on the 'Active' screen buffer.
/// It's assumed that screen buffer is always in 'RENDER_TARGET' mode, and not yet present.
/// Since this copying happens before the UI/ImGui pass.
/// </summary>
void RenderTarget::CopyToScreenBuffer()
{
	DXCommands* directCommands = DXAccess::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ComPtr<ID3D12GraphicsCommandList4> commandList = directCommands->GetGraphicsCommandList();
	ComPtr<ID3D12Resource> screenBuffer = DXAccess::GetWindow()->GetCurrentScreenBuffer();

	TransitionResource(renderTarget.Get(), resourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);
	TransitionResource(screenBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

	commandList->CopyResource(screenBuffer.Get(), renderTarget.Get());

	TransitionResource(renderTarget.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, resourceState);
	TransitionResource(screenBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

/// <summary>
/// Copies the current screen buffer into this render target. 
/// Relevant whenever you might want the output for the scene for something like a post processing technique.
/// </summary>
void RenderTarget::CopyFromScreenBuffer()
{
	DXCommands* directCommands = DXAccess::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ComPtr<ID3D12GraphicsCommandList4> commandList = directCommands->GetGraphicsCommandList();
	ComPtr<ID3D12Resource> screenBuffer = DXAccess::GetWindow()->GetCurrentScreenBuffer();

	TransitionResource(renderTarget.Get(), resourceState, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(screenBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

	commandList->CopyResource(renderTarget.Get(), screenBuffer.Get());

	TransitionResource(renderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceState);
	TransitionResource(screenBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void RenderTarget::CopyFromRenderTarget(RenderTarget* target)
{
	DXCommands* directCommands = DXAccess::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ComPtr<ID3D12GraphicsCommandList4> commandList = directCommands->GetGraphicsCommandList();
	ComPtr<ID3D12Resource> targetResource = target->GetResource();
	D3D12_RESOURCE_STATES targetOriginalState = target->GetResourceState();

	TransitionResource(renderTarget.Get(), resourceState, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(targetResource.Get(), targetOriginalState, D3D12_RESOURCE_STATE_COPY_SOURCE);

	commandList->CopyResource(renderTarget.Get(), targetResource.Get());

	TransitionResource(renderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceState);
	TransitionResource(targetResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, targetOriginalState);
}

void RenderTarget::AllocateResource()
{
	D3D12_RESOURCE_DESC renderTargetDescription = {};
	renderTargetDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	renderTargetDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	renderTargetDescription.Format = format;
	renderTargetDescription.Width = this->width;
	renderTargetDescription.Height = this->height;

	renderTargetDescription.DepthOrArraySize = 1;
	renderTargetDescription.MipLevels = 1;
	renderTargetDescription.SampleDesc.Count = 1;
	renderTargetDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	DXAccess::GetDevice()->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &renderTargetDescription,
		resourceState, nullptr, IID_PPV_ARGS(&renderTarget));
}

void RenderTarget::CreateDescriptors()
{
	DXDescriptorHeap* RTVHeap = DXAccess::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DXDescriptorHeap* SRVHeap = DXAccess::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ComPtr<ID3D12Device5> device = DXAccess::GetDevice();

	// SRV //
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	srvIndex = SRVHeap->GetNextAvailableIndex();
	device->CreateShaderResourceView(renderTarget.Get(), &srvDesc, SRVHeap->GetCPUHandleAt(srvIndex));

	// UAV //
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = format;

	uavIndex = SRVHeap->GetNextAvailableIndex();
	device->CreateUnorderedAccessView(renderTarget.Get(), nullptr, &uavDesc, SRVHeap->GetCPUHandleAt(uavIndex));

	// RTV //
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	rtvIndex = RTVHeap->GetNextAvailableIndex();
	device->CreateRenderTargetView(renderTarget.Get(), &rtvDesc, RTVHeap->GetCPUHandleAt(rtvIndex));
}

#pragma region
DXGI_FORMAT RenderTarget::GetFormat()
{
	return format;
}

ComPtr<ID3D12Resource> RenderTarget::GetResource()
{
	return renderTarget;
}

D3D12_RESOURCE_STATES RenderTarget::GetResourceState()
{
	return resourceState;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderTarget::GetRTV()
{
	return DXAccess::GetCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvIndex);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderTarget::GetSRV()
{
	return DXAccess::GetGPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srvIndex);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderTarget::GetUAV()
{
	return DXAccess::GetGPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, uavIndex);
}
#pragma endregion Getters