#include "Framework/Scene.h"
#include "Graphics/Model.h"
#include "Graphics/Camera.h"

Scene::Scene(bool enableRayTracingGeometry, bool spawnDefaultObjects) 
	: enableRayTracingGeometry(enableRayTracingGeometry)
{
	if(spawnDefaultObjects)
	{
		AddModel("Assets/Models/Default/GroundPlane/plane.gltf");
		//AddModel("Assets/Models/Default/Dragon/dragon.gltf");
		AddModel("Assets/Models/Default/FlightHelmet/FlightHelmet.gltf");
	}

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