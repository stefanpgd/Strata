#pragma once
#include "Framework/Project.h"

class ComputeUVStage;

class HelloComputeProject : public Project
{
public:
	HelloComputeProject();

	void Update(float deltaTime) override;
	void Render(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	ComputeUVStage* computeUVStage;
};