#pragma once
#include "Graphics/PostProcessPass.h"

class PostProcessBloom : public PostProcessPass
{
public: 
	PostProcessBloom();

	void Update(float deltaTime) override;
	void RecordPass(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	void InitializePipelines();

private:
	DXRootSignature* tresholdRootsignature;
	DXPipeline* tresholdPipeline;
	RenderTarget* tresholdTarget;
	float bloomTreshold = 0.995;

	DXRootSignature* blurRootsignature;
	DXPipeline* blurPipeline;
	RenderTarget* blurTargetFront;
	RenderTarget* blurTargetBack;
	int blurPasses = 6;

	DXRootSignature* blendRootsignature;
	DXPipeline* blendPipeline;
};