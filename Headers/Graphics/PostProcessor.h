#pragma once

#include "Graphics/DXCommon.h"
#include <vector>

class Mesh;
class RenderTarget;
class PostProcessPass;

class PostProcessor
{
public:
	PostProcessor(RenderTarget* sceneOutput);

	void Update(float deltaTime);
	void Execute(ComPtr<ID3D12GraphicsCommandList4> commandList);

	void AddPass(PostProcessPass* pass);

public:
	bool IsEnabled = true;

private:
	void InitializeScreenSquad();

private:
	std::vector<PostProcessPass*> passes;
	Mesh* screenQuad;

	RenderTarget* sceneOutput;
	RenderTarget* postProcessingTarget; 
};