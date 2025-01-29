#pragma once
#include "Graphics/RenderStage.h"
#include "Graphics/DXCommon.h"

class Scene;
class DXRootSignatre;
class DXPipeline;
class RenderTarget;

class SceneRenderStage : public RenderStage
{
public:
	SceneRenderStage(Scene* scene);

	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	void InitializePipeline();
	
public:
	RenderTarget* renderTarget;

private:
	Scene* activeScene;

	DXRootSignature* rootSignature;
	DXPipeline* renderPipeline;
};