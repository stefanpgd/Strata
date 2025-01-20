#pragma once

#include "Graphics/DXCommon.h"

class Mesh;
class RenderTarget;

class PostProcessor
{
public:
	PostProcessor();

	void Execute(ComPtr<ID3D12GraphicsCommandList4> commandList);

private:
	void InitializeScreenSquad();

private:
	RenderTarget* sceneOutput;
	Mesh* screenQuad;
};