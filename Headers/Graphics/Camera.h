#pragma once
#include "Framework/Mathematics.h"

class Camera
{
public: 
	Camera(glm::vec3 position = glm::vec3(0.0f, 1.0f, 4.0f));

	void Update();

	void UpdateViewMatrix();
	void ResizeProjectionMatrix(int windowWidth, int windowHeight);

	const glm::vec3& GetForwardVector();
	const glm::vec3& GetUpwardVector();

	const glm::mat4& GetViewMatrix();
	const glm::mat4& GetProjectionMatrix();
	const glm::mat4& GetViewProjectionMatrix();

public:
	glm::vec3 Position;

private:
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 viewProjection; 

	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	float FOV = 60.0f;
	float nearClip = 0.01f;
	float farClip = 1000.0f;
	float aspectRatio;
};