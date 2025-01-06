#pragma once

#include <string>
#include "Graphics/DXCommon.h"

/// <summary>
/// A 'project' is an "rendering/game application" build out in the framework.
/// For example, a Game of Life simulation would be contained within a project.
/// 
/// Project will be a base class, the engine will own projects and run one at a time.
/// Through the editor we can switch between existing projects.
/// </summary>
class Project
{
public:
	virtual void Update(float deltaTime) = 0;
	virtual void Render(ComPtr<ID3D12GraphicsCommandList4> commandList) = 0;

public:
	std::string ProjectName = "New Project";
};