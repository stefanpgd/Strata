#include "Projects/Strata/PostProcessingStage.h"
#include "Graphics/PostProcessor.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/DXUtilities.h"

// TEMP //
#include "Utilities/EditorElements.h"
#include <imgui.h>

PostProcessingStage::PostProcessingStage()
{
	postProcessor = new PostProcessor();
	postProcessingTarget = new RenderTarget(DXAccess::GetWindowWidth(), DXAccess::GetWindowHeight());
}

void PostProcessingStage::Update(float deltaTime)
{
	// TODO: temporarily fine for the gimmick but move out when relevant. 
	ImGui::Begin("Render Targets");
	float aspectRatio = float(DXAccess::GetWindowWidth()) / float(DXAccess::GetWindowHeight());
	EditorRenderTargetHighlight(postProcessingTarget, "Post Processor View", 250, aspectRatio);

	ImGui::End();

	postProcessor->Update(deltaTime);
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