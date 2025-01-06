#pragma once
#include <string>
#include <vector>
#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class Project;

/// <summary>
/// Receives a Project to render, but should be able to work independently without one.
/// This takes care of rendering pipeline initialization and initializing components that Project renderstages might need.
/// </summary>
class Renderer
{
public:
	Renderer(const std::wstring& applicationName, unsigned int windowWidth, unsigned int windowHeight);
	
	void Render(Project* project);
	void Resize();

private:
	void InitializeImGui();
};