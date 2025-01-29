#pragma once
#include "Framework/Project.h"

class Scene;
class SceneRenderStage;
class PostProcessingStage;

class StrataProject : public Project
{
public:
	StrataProject();

	void Update(float deltaTime) override;
	void Render(ComPtr<ID3D12GraphicsCommandList4> commandList);

private:
	Scene* scene;

	SceneRenderStage* sceneRenderStage;
	PostProcessingStage* postProcessingStage;
};