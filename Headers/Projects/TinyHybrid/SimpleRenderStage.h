#pragma once
#include "Graphics/RenderStage.h"
#include "Graphics/DXCommon.h"

class Scene;
class DXRootSignature;
class DXPipeline;

class SimpleRenderStage : public RenderStage
{
public:
	SimpleRenderStage(Scene* sceneToRender);

	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;
	
private:
	void InitializePipeline();

private:
	Scene* activeScene;

	DXRootSignature* rootSignature;
	DXPipeline* renderPipeline;
};