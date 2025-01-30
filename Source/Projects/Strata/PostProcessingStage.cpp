#include "Projects/Strata/PostProcessingStage.h"
#include "Graphics/PostProcessor.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXUtilities.h"

// Post Processing Passes
#include "Graphics/PostProcessPasses/PostProcessVignette.h"
#include "Graphics/PostProcessPasses/PostProcessBloom.h"
#include "Graphics/PostProcessPasses/PostProcessHDR.h"

PostProcessingStage::PostProcessingStage(RenderTarget* sceneOutput)
{
	postProcessor = new PostProcessor(sceneOutput);

	PostProcessBloom* bloomPass = new PostProcessBloom();
	PostProcessHDR* hdrPass = new PostProcessHDR();
	postProcessor->AddPass(bloomPass);
	postProcessor->AddPass(hdrPass);

	//PostProcessVignette* vignettePass = new PostProcessVignette();
	//
	//postProcessor->AddPass(vignettePass);
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