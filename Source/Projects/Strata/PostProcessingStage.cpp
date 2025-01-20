#include "Projects/Strata/PostProcessingStage.h"
#include "Graphics/PostProcessor.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXUtilities.h"

PostProcessingStage::PostProcessingStage()
{
	postProcessor = new PostProcessor();
	postProcessingTarget = new RenderTarget(DXAccess::GetWindowWidth(), DXAccess::GetWindowHeight());
}

void PostProcessingStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	// The 'PostProcessingTarget' is the acting renderTarget which collects each pass performed by the post processor
	// The PostProcessor is just responsible for firing the Passes
	postProcessingTarget->Clear();
	postProcessingTarget->Bind();

	postProcessor->Execute(commandList);

	postProcessingTarget->CopyToScreenBuffer();
}