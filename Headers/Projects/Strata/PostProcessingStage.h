#pragma once
#include "Graphics/RenderStage.h"

class PostProcessingStage : public RenderStage
{
public:
	PostProcessingStage();

	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

private:

};