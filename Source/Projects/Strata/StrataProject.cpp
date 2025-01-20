#include "Projects/Strata/StrataProject.h"
#include "Projects/Strata/ModelRenderStage.h"
#include "Projects/Strata/PostProcessingStage.h"

#include "Framework/Scene.h"
#include "Graphics/Camera.h" // TODO: consider if this should be part of Framework or Graphics
#include "Graphics/Model.h"

/// <summary>
/// Goal 1 [x]: A plane, a cube on it, and a focused camera that can pan around with dragging the mouse
/// Goal 2 [-]: Post Processing - Vignette & Bloom(?)
/// Goal 3 [ ]: Textured Cube, with mipmapping
/// Goal 4: Some sort of scene serialization (?)
/// 
/// Optional goals to work through
/// - Proper implementation of resizing for all base systems?
/// .. to be determined ..
/// </summary>
/// 
/// - [x] To do post processing I really have to properly figure out the render target stuff.
///		  This might also mean rewriting elements of the Window Class
/// 
/// To do - Read into mip-maps, together with things such as the subresource stuff to properly access it in HLSL
/// Likely start with Jeremiah's stuff and decipher it..

// Random note, Jeremiah seems to have made a resource state tracker. Might be cool to investigate

/// For post processing I will first need: 
// To do - Go through the entire code base, find some things that could already use improvements, maybe pair them with some of the goals

StrataProject::StrataProject()
{
	scene = new Scene(false, false);
	scene->Camera->Position = glm::vec3(0.0f, 0.5f, 2.5f);
	
	Model* cube = scene->AddModel("Assets/Models/Prototype/proto_cube_01.gltf");
	cube->transform.Position = glm::vec3(0.0f, 0.5f, 0.0f);
	scene->AddModel("Assets/Models/GroundPlane/plane.gltf");

	modelRenderStage = new ModelRenderStage(scene);
	postProcessingStage = new PostProcessingStage();
}

void StrataProject::Update(float deltaTime)
{
	scene->Update(deltaTime);
}

void StrataProject::Render(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	//modelRenderStage->RecordStage(commandList);
	postProcessingStage->RecordStage(commandList);
}