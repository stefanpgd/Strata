#pragma once
#include "Graphics/RenderStage.h"
#include "Graphics/DXCommon.h"

class Texture;

class BlendResultStage : public RenderStage
{
public:
	BlendResultStage(Texture* shadowOutput);

	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:
	Texture* shadowOutput;
	Texture* sceneRender;
	Texture* blendedOutput;
};