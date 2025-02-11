#pragma once
#include "Graphics/RenderStage.h"

class PostProcessor;
class RenderTarget;

class PostProcessingStage : public RenderStage
{
public:
	PostProcessingStage(RenderTarget* sceneOutput);

	void Update(float deltaTime);
	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	PostProcessor* postProcessor;
};