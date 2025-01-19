#pragma once
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Graphics/DXCommon.h"

class DXDescriptorHeap;
class Texture;

/// <summary>
/// This class is responsible for setting up and running a Window we can draw to.
/// Part of this is creating 'ScreenBuffers', which are our back buffers for our swap chain.
/// 
/// How this relates to our renderer is that, no matter what. The screen's buffer will always be cleared
/// and presented during a given frame. So, the screen buffer is always the final representation of our render.
/// So make sure this ends up being used as the output.  
/// </summary>
class Window
{
public:
	Window(const std::wstring& applicationName, unsigned int windowWidth, unsigned int windowHeight);

	void Present();
	void Resize();

	unsigned int GetCurrentScreenBufferIndex();

	ComPtr<ID3D12Resource> GetCurrentScreenBuffer();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentScreenRTV();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthDSV();

	HWND GetHWND();
	unsigned int GetWindowWidth();
	unsigned int GetWindowHeight();
	const D3D12_VIEWPORT& GetViewport();
	const D3D12_RECT& GetScissorRect();

private:
	void SetupWindow();
	void CreateSwapChain();

	void UpdateScreenBuffers();
	void UpdateDepthBuffer();

public:
	static const unsigned int BackBufferCount = 3;

private:
	// Window Settings //
	std::wstring windowName;
	unsigned int windowWidth;
	unsigned int windowHeight;
	HWND windowHandle;
	RECT windowRect;

	bool vSync = false;
	bool tearingSupported = true;
	bool fullscreen = false;

	// Screen Buffers //
	ComPtr<IDXGISwapChain4> swapChain;
	ComPtr<ID3D12Resource> screenBuffers[BackBufferCount];
	int screenBufferRTVs[BackBufferCount];

	// Depth Buffer //
	ComPtr<ID3D12Resource> depthBuffer;
	int depthDSVIndex;

	// Rasterizer Objects //
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
};