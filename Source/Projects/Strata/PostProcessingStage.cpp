#include "Projects/Strata/PostProcessingStage.h"
#include "Graphics/PostProcessor.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXUtilities.h"

/// <summary>
/// Lets think architecture -
/// Post processing stage is just our personal implemetation for this project.
/// That means that it likely should just "use" components that are part of the framework.
/// With that in mind, we likely want to make a "PostProcesser".
/// 
/// What would that be? Likely an owner of post processing passes. Each pass has a function
/// like Draw/Run. Then applies it to the RenderTarget that got passed to the PostProcessor.
/// 
/// This stage is responsible for providing the render targets, and the "draw order" of post processing passes.
/// Then we just call the post processor to initiate the calls.
/// 
/// We also want the option to switch it On/Off, which means that this stage will require some booleans for that.
/// </summary>

// Knowing this, let's start with Vignette through a Post Processor, and being able to turn off/on the post processing
PostProcessingStage::PostProcessingStage()
{
	postProcessor = new PostProcessor();
	postProcessingTarget = new RenderTarget(DXAccess::GetWindowWidth(), DXAccess::GetWindowHeight());
}

void PostProcessingStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	postProcessingTarget->Clear();
	postProcessingTarget->Bind();

	postProcessor->Execute(commandList);

	postProcessingTarget->CopyToScreenBuffer();
}