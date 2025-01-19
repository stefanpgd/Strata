#include "Graphics/RenderTarget.h"
#include "Graphics/DXAccess.h"
#include "Graphics/DXDescriptorHeap.h"

RenderTarget::RenderTarget(unsigned int width, unsigned int height) : width(width), height(height)
{
	// todo, add parameter to get depth buffer from render target to?
	// todo, add bind function
	
	// todo replace with constructor parameter
	format = DXGI_FORMAT_R8G8B8A8_UNORM;

	AllocateResource();
	CreateDescriptors();
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
		D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&renderTarget));
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

ComPtr<ID3D12Resource> RenderTarget::GetResource()
{
	return renderTarget;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderTarget::GetRTV()
{
	return DXAccess::GetCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvIndex);
}