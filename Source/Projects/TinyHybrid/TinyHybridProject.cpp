#include "Projects/TinyHybrid/TinyHybridProject.h"
#include "Projects/TinyHybrid/SimpleRenderStage.h"
#include "Projects/TinyHybrid/RTShadowsStage.h"
#include "Projects/TinyHybrid/BlendResultStage.h"

#include "Graphics/Model.h"
#include "Graphics/Camera.h"
#include "Utilities/EditorElements.h"
#include "Framework/Scene.h"

#include <imgui.h>
#include <vector>

TinyHybridProject::TinyHybridProject()
{
	scene = new Scene(true, true);
	scene->Camera->Position = glm::vec3(0.0f, 0.5f, 1.5f);

	simpleRenderStage = new SimpleRenderStage(scene);
	shadowStage = new RTShadowStage(scene);
	blendStage = new BlendResultStage(shadowStage->outputBuffer);
}

void TinyHybridProject::Update(float deltaTime)
{
	scene->Update(deltaTime);
	shadowStage->Update(deltaTime);

	// Editor //
	ImGui::Begin("Scene");
	ImGui::DragFloat3("Sun Direction", &scene->SunDirection[0], 0.001f, -1.0f, 1.0f);
	scene->SunDirection = glm::normalize(scene->SunDirection);
	ImGui::End();
}

void TinyHybridProject::Render(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	simpleRenderStage->RecordStage(commandList);
	shadowStage->RecordStage(commandList);
	blendStage->RecordStage(commandList);
}