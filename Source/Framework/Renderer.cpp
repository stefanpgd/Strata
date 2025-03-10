#include "Framework/Renderer.h"
#include "Framework/Project.h"

// DirectX Components //
#include "Graphics/DXAccess.h"
#include "Graphics/DXDevice.h"
#include "Graphics/DXCommands.h"
#include "Graphics/DXUtilities.h"

// Renderer Components //
#include "Graphics/Window.h"
#include "Graphics/Texture.h"

#include <cassert>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

namespace RendererInternal
{
	Window* window = nullptr;
	DXDevice* device = nullptr;

	DXCommands* directCommands = nullptr;
	DXCommands* copyCommands = nullptr;

	DXDescriptorHeap* CBVHeap = nullptr;
	DXDescriptorHeap* DSVHeap = nullptr;
	DXDescriptorHeap* RTVHeap = nullptr;

	Texture* defaultTexture = nullptr;
}
using namespace RendererInternal;

Renderer::Renderer(const std::wstring& applicationName, unsigned int windowWidth,
	unsigned int windowHeight)
{
	// Initialization for all vital components for rendering //
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	device = new DXDevice();
	CBVHeap = new DXDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5000, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	RTVHeap = new DXDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 300);
	DSVHeap = new DXDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 10);

	directCommands = new DXCommands(D3D12_COMMAND_LIST_TYPE_DIRECT, Window::BackBufferCount);
	copyCommands = new DXCommands(D3D12_COMMAND_LIST_TYPE_DIRECT, 1);

	window = new Window(applicationName, windowWidth, windowHeight);

	InitializeImGui();
}

// What should Render be?
// Technically it should just prepare a render call, likely by just clearing out the render target, which is the screen buffer
// Then afterwards, ASSUME that the project has written to it. That in mind, Window needs a refactor.
void Renderer::Render(Project* project)
{
	unsigned int backBufferIndex = window->GetCurrentScreenBufferIndex();
	ComPtr<ID3D12GraphicsCommandList4> commandList = directCommands->GetGraphicsCommandList();
	ID3D12DescriptorHeap* heaps[] = { CBVHeap->GetAddress() };

	ComPtr<ID3D12Resource> renderTargetBuffer = window->GetCurrentScreenBuffer();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = window->GetCurrentScreenRTV();
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle = window->GetDepthDSV();

	directCommands->ResetCommandList(backBufferIndex);
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	TransitionResource(renderTargetBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	BindAndClearRenderTarget(window, &rtvHandle, &dsvHandle);

	project->Render(commandList);

	BindRenderTarget(window, &rtvHandle);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
	TransitionResource(renderTargetBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	directCommands->ExecuteCommandList(backBufferIndex);
	window->Present();
	directCommands->WaitForFenceValue(window->GetCurrentScreenBufferIndex());
}

void Renderer::Resize()
{
	directCommands->Flush();
	window->Resize();
}

void Renderer::InitializeImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window->GetHWND());

	const unsigned int cbvIndex = CBVHeap->GetNextAvailableIndex();
	ImGui_ImplDX12_Init(device->GetAddress(), Window::BackBufferCount, DXGI_FORMAT_R8G8B8A8_UNORM,
		CBVHeap->GetAddress(), CBVHeap->GetCPUHandleAt(cbvIndex), CBVHeap->GetGPUHandleAt(cbvIndex));
}

#pragma region DXAccess Implementations
ComPtr<ID3D12Device5> DXAccess::GetDevice()
{
	if(!device)
	{
		assert(false && "DXDevice hasn't been initialized yet, call will return nullptr");
	}

	return device->Get();
}

DXCommands* DXAccess::GetCommands(D3D12_COMMAND_LIST_TYPE type)
{
	if(!directCommands || !copyCommands)
	{
		assert(false && "Commands haven't been initialized yet, call will return nullptr");
	}

	switch(type)
	{
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return copyCommands;
		break;

	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return directCommands;
		break;
	}

	assert(false && "This command type hasn't been added yet!");
	return nullptr;
}

unsigned int DXAccess::GetCurrentScreenBufferIndex()
{
	if(!window)
	{
		assert(false && "Window hasn't been initialized yet, can't retrieve back buffer index");
	}

	return window->GetCurrentScreenBufferIndex();
}

Texture* DXAccess::GetDefaultTexture()
{
	// TODO: Move to TextureManager

	// Incase an texture isn't present, the 'default' texture gets loaded in
	// Similar to Valve's ERROR 3D model
	return defaultTexture;
}

DXDescriptorHeap* DXAccess::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	switch(type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return CBVHeap;
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return DSVHeap;
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return RTVHeap;
		break;
	}

	// Invalid type passed
	assert(false && "Descriptor type passed isn't a valid or created type!");
	return nullptr;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DXAccess::GetCPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int index)
{
	switch(type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return CBVHeap->GetCPUHandleAt(index);
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return DSVHeap->GetCPUHandleAt(index);
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return RTVHeap->GetCPUHandleAt(index);
		break;
	}

	// Invalid type passed
	assert(false && "Descriptor type passed isn't a valid or created type!");
	return CD3DX12_CPU_DESCRIPTOR_HANDLE();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DXAccess::GetGPUDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int index)
{
	switch(type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return CBVHeap->GetGPUHandleAt(index);
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return DSVHeap->GetGPUHandleAt(index);
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return RTVHeap->GetGPUHandleAt(index);
		break;
	}

	// Invalid type passed
	assert(false && "Descriptor type passed isn't a valid or created type!");
	return CD3DX12_GPU_DESCRIPTOR_HANDLE();
}

Window* DXAccess::GetWindow()
{
	return window;
}

unsigned int DXAccess::GetWindowWidth()
{
	return window->GetWindowWidth();
}
unsigned int DXAccess::GetWindowHeight()
{
	return window->GetWindowHeight();
}
#pragma endregion