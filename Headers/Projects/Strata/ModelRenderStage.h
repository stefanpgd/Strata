#pragma once
#include "Graphics/RenderStage.h"
#include "Graphics/DXCommon.h"

class Scene;
class DXRootSignatre;
class DXPipeline;
class RenderTarget;

class ModelRenderStage : public RenderStage
{
public:
	ModelRenderStage(Scene* scene);

	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	void InitializePipeline();
	
private:
	Scene* activeScene;
	RenderTarget* renderTarget;

	DXRootSignature* rootSignature;
	DXPipeline* renderPipeline;
};