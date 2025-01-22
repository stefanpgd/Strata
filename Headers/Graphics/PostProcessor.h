#pragma once

#include "Graphics/DXCommon.h"
#include <vector>

class Mesh;
class RenderTarget;
class PostProcessPass;

class PostProcessor
{
public:
	PostProcessor();

	void Update(float deltaTime);
	void Execute(ComPtr<ID3D12GraphicsCommandList4> commandList);

	void AddPass(PostProcessPass* pass);

private:
	void InitializeScreenSquad();

private:
	std::vector<PostProcessPass*> passes;
	Mesh* screenQuad;

	RenderTarget* sceneOutput;
	RenderTarget* postProcessingTarget; 
};