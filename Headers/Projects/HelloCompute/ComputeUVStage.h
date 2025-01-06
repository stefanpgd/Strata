#pragma once
#include "Graphics/RenderStage.h"

class Texture;

class ComputeUVStage : public RenderStage
{
public:
	ComputeUVStage();

	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	Texture* renderBuffer;
};