#include "Framework/Scene.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"
#include "Utilities/Logger.h"
#include <tinyexr.h>

Scene::Scene(bool enableRayTracingGeometry, bool spawnDefaultObjects) 
	: enableRayTracingGeometry(enableRayTracingGeometry)
{
	if(spawnDefaultObjects)
	{
		AddModel("Assets/Models/Default/GroundPlane/plane.gltf");
	}

	skydome = AddModel("Assets/Models/Sphere/sphere.gltf");
	skydome->transform.Scale = glm::vec3(-8000.0f);
	skydome->transform.Rotation = glm::vec3(0.0f, 0.0f, 180.0f);
	LoadSkydomeTexture("Assets/Skydomes/sky.exr");

	SunDirection = glm::normalize(glm::vec3(-0.325, 0.785f, 0.52f));
	Camera = new ::Camera();
}

void Scene::Update(float deltaTime)
{
	Camera->Update(deltaTime);
}

Model* Scene::AddModel(const std::string& path)
{
	// TODO: Add a model loading manager similar to the TextureManager
	Model* newModel = new Model(path, enableRayTracingGeometry);
	models.push_back(newModel);

	return newModel;
}

const std::vector<Model*>& Scene::GetModels()
{
	return models;
}

void Scene::LoadSkydomeTexture(std::string exrPath)
{
	const char* err = nullptr;
	float* image;

	int width;
	int height;

	int result = LoadEXR(&image, &width, &height, exrPath.c_str(), &err);
	if(result != TINYEXR_SUCCESS)
	{
		std::string error(err);
		LOG(Log::MessageType::Error, "EXR Failed to load:");
		LOG(Log::MessageType::Error, error.c_str());
		assert(false);
	}

	skydomeTexture = new Texture(image, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(float) * 4);
	skydome->GetMesh(0)->Textures.Albedo = skydomeTexture;
}