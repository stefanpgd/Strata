#pragma once

#include <string>
#include <vector>
#include "Framework/Mathematics.h"

class Model;
class Camera;

/// <summary>
/// Responsible for owning and managing all the geometry in a Scene
/// If a model needs to be loaded, it happens through the Scene.
/// Later on functionality such as serialization should be connected up via here.
/// </summary>
class Scene
{
public:
	Scene(bool enableRayTracingGeometry = false, bool spawnDefaultObjects = true);

	void Update(float deltaTime);
	void AddModel(const std::string& path);

	const std::vector<Model*>& GetModels();
public:
	Camera* Camera;
	glm::vec3 SunDirection;

private:
	std::vector<Model*> models;
	bool enableRayTracingGeometry;

	friend class Editor;
};