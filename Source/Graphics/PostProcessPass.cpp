#include "Graphics/PostProcessPass.h"

void PostProcessPass::SetRenderTargets(RenderTarget* sceneOutput, RenderTarget* postProcessTarget)
{
	this->sceneOutput = sceneOutput;
	this->postProcessTarget = postProcessTarget;
}