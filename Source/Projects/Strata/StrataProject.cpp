#include "Projects/Strata/StrataProject.h"
#include "Projects/Strata/ModelRenderStage.h"

#include "Framework/Scene.h"
#include "Graphics/Camera.h" // TODO: consider if this should be part of Framework or Graphics
#include "Graphics/Model.h"

/// <summary>
/// Goal 1: A plane, a cube on it, and a focused camera that can pan around with dragging the mouse
/// Goal 2: Textured Cube, with mipmapping
/// Goal 3: Post Processing - Vignette & Bloom(?)
/// .. to be determined ..
/// </summary>

StrataProject::StrataProject()
{
	scene = new Scene(false, false);
	scene->AddModel("Assets/Models/GroundPlane/plane.gltf");

	Model* cube = scene->AddModel("Assets/Models/Prototype/proto_cube_01.gltf");
	cube->transform.Position = glm::vec3(0.0f, 0.5f, 0.0f);

	scene->Camera->Position = glm::vec3(0.0f, 0.5f, 1.0f);
	modelRenderStage = new ModelRenderStage(scene);
}

void StrataProject::Update(float deltaTime)
{

}

void StrataProject::Render(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	modelRenderStage->RecordStage(commandList);
}