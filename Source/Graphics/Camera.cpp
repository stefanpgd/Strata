#include "Graphics/Camera.h"
#include "Framework/Input.h"
#include "Graphics/DXAccess.h"
#include "Graphics/Window.h"

Camera::Camera(glm::vec3 position) : Position(position)
{
	UpdateViewMatrix();
	ResizeProjectionMatrix(DXAccess::GetWindow()->GetWindowWidth(), DXAccess::GetWindow()->GetWindowHeight());
}

void Camera::Update()
{
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	view = glm::lookAt(Position, Position + front, up);
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