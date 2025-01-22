#pragma once
#include "Graphics/PostProcessPass.h"

class PostProcessVignette : public PostProcessPass
{
public:
	PostProcessVignette();

	void Update(float deltaTime) override;
	void RecordPass(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	void InitializePipeline();

private:
	float strength = 1.0f;
	float aspectRatio = 1.0f;
};