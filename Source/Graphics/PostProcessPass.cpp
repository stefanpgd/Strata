#include "Graphics/PostProcessPass.h"

void PostProcessPass::SetComponents(RenderTarget* sceneOutput, RenderTarget* postProcessTarget, Mesh* screenQuad)
{
	this->sceneOutput = sceneOutput;
	this->postProcessTarget = postProcessTarget;
	this->screenQuad = screenQuad; 
}