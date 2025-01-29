#pragma once

#include <string>
#include <vector>
#include "Framework/Mathematics.h"

class Model;
class Camera;
class Texture;

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
	Model* AddModel(const std::string& path);

	const std::vector<Model*>& GetModels();

private:
	void LoadSkydomeTexture(std::string exrPath);

public:
	Camera* Camera;
	glm::vec3 SunDirection;

private:
	std::vector<Model*> models;
	bool enableRayTracingGeometry;

	Model* skydome;
	Texture* skydomeTexture;

	friend class Editor;
};