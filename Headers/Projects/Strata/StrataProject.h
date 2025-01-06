#pragma once
#include "Framework/Project.h"

class Scene;
class ModelRenderStage;

class StrataProject : public Project
{
public:
	StrataProject();

	void Update(float deltaTime) override;
	void Render(ComPtr<ID3D12GraphicsCommandList4> commandList);

private:
	Scene* scene;
	ModelRenderStage* modelRenderStage;
};