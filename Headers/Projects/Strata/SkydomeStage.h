#pragma once

#include "Graphics/RenderStage.h"
#include <string>

class Model;
class Texture;
class RenderTarget;
class Scene;

class SkydomeStage : public RenderStage
{
public:
	SkydomeStage(Scene* activeScene, RenderTarget* modelRenderTarget);

	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	void LoadSkydome(std::string path);
	void InitializePipeline();

private:
	RenderTarget* modelRenderTarget;
	Scene* activeScene;

	Model* skydome;
	Texture* skydomeTexture;

	DXRootSignature* rootSignature;
	DXPipeline* pipeline;
};