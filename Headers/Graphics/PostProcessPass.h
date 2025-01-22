#pragma once
#include "Graphics/DXCommon.h"

class DXRootSignature;
class DXPipeline;
class RenderTarget;

class PostProcessPass
{
public:
	void SetRenderTargets(RenderTarget* sceneOutput, RenderTarget* postProcessTarget);

	virtual void Update(float deltaTime) = 0;
	virtual void RecordPass(ComPtr<ID3D12GraphicsCommandList4> commandList) = 0;

public:
	bool IsEnabled = true;

protected:
	DXRootSignature* rootSignature;
	DXPipeline* pipeline;

	RenderTarget* sceneOutput;
	RenderTarget* postProcessTarget;
};