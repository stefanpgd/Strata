#pragma once

#include "Graphics/PostProcessPass.h"

class RenderTarget;

class PostProcessHDR : public PostProcessPass
{
public:
	PostProcessHDR();

	void Update(float deltaTime) override;
	void RecordPass(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	void InitializePipeline();

private:
	RenderTarget* ldrTarget;

	float exposure = 1.0f;
	float gamma = 2.24f;
};