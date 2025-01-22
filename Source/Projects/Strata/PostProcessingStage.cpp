#include "Projects/Strata/PostProcessingStage.h"
#include "Graphics/PostProcessor.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXUtilities.h"

// Post Processing Passes
#include "Graphics/PostProcessPasses/PostProcessVignette.h"
#include "Graphics/PostProcessPasses/PostProcessBloom.h"

PostProcessingStage::PostProcessingStage()
{
	postProcessor = new PostProcessor();

	PostProcessBloom* bloomPass = new PostProcessBloom();
	PostProcessVignette* vignettePass = new PostProcessVignette();

	postProcessor->AddPass(bloomPass);
	postProcessor->AddPass(vignettePass);
}

void PostProcessingStage::Update(float deltaTime)
{
	postProcessor->Update(deltaTime);
}

void PostProcessingStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	// The 'PostProcessingTarget' is the acting renderTarget which collects each pass performed by the post processor
	// The PostProcessor is just responsible for firing the Passes
	postProcessor->Execute(commandList);
}