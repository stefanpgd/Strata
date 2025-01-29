#include "Graphics/PostProcessor.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXUtilities.h"
#include "Graphics/PostProcessPass.h"

PostProcessor::PostProcessor(RenderTarget* seneOutput) : sceneOutput(seneOutput)
{
	InitializeScreenSquad();

	// Scene Output - A copy of the "main" render target before post processing
	// Post Processing Target - the render target which gets used as target for reach post process pass
	unsigned int width = DXAccess::GetWindowWidth();
	unsigned int height = DXAccess::GetWindowHeight();
	
	postProcessingTarget = new RenderTarget(width, height);
}

void PostProcessor::Update(float deltaTime)
{
	for(PostProcessPass* pass : passes)
	{
		pass->Update(deltaTime);
	}
}

void PostProcessor::Execute(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	if(!IsEnabled)
	{
		return;
	}

	// 1) Clear & Bind out target for the post processor
	postProcessingTarget->Clear();
	postProcessingTarget->Bind();

	// 2) Bind Screen Quad which we can use for our rasterizer and all passes
	commandList->IASetVertexBuffers(0, 1, &screenQuad->GetVertexBufferView());
	commandList->IASetIndexBuffer(&screenQuad->GetIndexBufferView());

	// 3) Iterate over all passes and record their commands
	for(int i = 0; i < passes.size(); i++)
	{
		if(passes[i]->IsEnabled)
		{
			passes[i]->RecordPass(commandList);
		}
	}

	// 4) Copy end result over to the screen buffer (of this frame)
	postProcessingTarget->CopyToScreenBuffer();
}

void PostProcessor::AddPass(PostProcessPass* pass)
{
	pass->SetComponents(sceneOutput, postProcessingTarget, screenQuad);
	passes.push_back(pass);
}

void PostProcessor::InitializeScreenSquad()
{
	Vertex v0, v1, v2, v3;
	v0.Position = glm::vec3(-1.0f, -1.0f, 0.0f);	// bottom left
	v1.Position = glm::vec3(1.0f, -1.0f, 0.0f);		// bottom right
	v2.Position = glm::vec3(-1.0f, 1.0f, 0.0f);		// top left
	v3.Position = glm::vec3(1.0f, 1.0f, 0.0f);		// top right

	v0.TextureCoord0 = glm::vec2(0.0f, 1.0f);
	v1.TextureCoord0 = glm::vec2(1.0f, 1.0f);
	v2.TextureCoord0 = glm::vec2(0.0f, 0.0f);
	v3.TextureCoord0 = glm::vec2(1.0f, 0.0f);

	std::vector<Vertex> vertices;
	vertices.push_back(v0);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);

	// CCW
	unsigned int indices[6] = 
	{
		0, 1, 2, 2, 1, 3
	};

	screenQuad = new Mesh(vertices.data(), 4, indices, 6);
}