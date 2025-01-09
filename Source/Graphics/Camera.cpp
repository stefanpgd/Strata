#include "Graphics/Camera.h"
#include "Framework/Input.h"
#include "Graphics/DXAccess.h"
#include "Graphics/Window.h"
#include "Framework/Mathematics.h"

Camera::Camera(glm::vec3 position) : Position(position)
{
	UpdateViewMatrix();
	ResizeProjectionMatrix(DXAccess::GetWindow()->GetWindowWidth(), DXAccess::GetWindow()->GetWindowHeight());
}

void Camera::Update(float deltaTime)
{
	scrollVelocity += Input::GetScollVelocity();
	scrollVelocity = Lerp(scrollVelocity, 0.0f, 0.003f);

	Position.z += zoomSpeed * scrollVelocity * deltaTime;

	glm::vec3 target = glm::vec3(0.0f, 0.5f, 0.0f);
	float distance = glm::length(target - Position);

	if(distance < minimumZoomSpacing)
	{
		Position.z = minimumZoomSpacing;
		scrollVelocity = 0.0f;
	}

	view = glm::lookAt(Position, target, glm::vec3(0, 1.0, 0.0));
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	//view = glm::lookAt(Position, Position + front, up);
	viewProjection = projection * view;
}

void Camera::ResizeProjectionMatrix(int windowWidth, int windowHeight)
{
	aspectRatio = float(windowWidth) / float(windowHeight);
	projection = glm::perspective(glm::radians(FOV), aspectRatio, nearClip, farClip);

	viewProjection = projection * view;
}

const glm::vec3& Camera::GetForwardVector()
{
	return front;
}

const glm::vec3& Camera::GetUpwardVector()
{
	return up;
}

const glm::mat4& Camera::GetViewProjectionMatrix()
{
	return viewProjection;
}

const glm::mat4& Camera::GetViewMatrix()
{
	return view;
}

const glm::mat4& Camera::GetProjectionMatrix()
{
	return projection;
}