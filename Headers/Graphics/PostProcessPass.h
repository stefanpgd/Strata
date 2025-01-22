#pragma once
#include "Graphics/DXCommon.h"
#include "Graphics/DXRootSignature.h"
#include "Graphics/DXPipeline.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderTarget.h"

class PostProcessPass
{
public:
	void SetComponents(RenderTarget* sceneOutput, RenderTarget* postProcessTarget, Mesh* screenQuad);

	virtual void Update(float deltaTime) = 0;
	virtual void RecordPass(ComPtr<ID3D12GraphicsCommandList4> commandList) = 0;

public:
	bool IsEnabled = true;

protected:
	DXRootSignature* rootSignature;
	DXPipeline* pipeline;

	RenderTarget* sceneOutput;
	RenderTarget* postProcessTarget;
	Mesh* screenQuad;
};