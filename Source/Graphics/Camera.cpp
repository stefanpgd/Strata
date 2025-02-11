#include "Graphics/Camera.h"
#include "Framework/Input.h"
#include "Graphics/DXAccess.h"
#include "Graphics/Window.h"
#include "Framework/Mathematics.h"
#include "Utilities/Logger.h"

#include <imgui.h>

Camera::Camera(glm::vec3 position) : Position(position)
{
	UpdateViewMatrix();
	ResizeProjectionMatrix(DXAccess::GetWindowWidth(), DXAccess::GetWindowHeight());
}

void Camera::Update(float deltaTime)
{
	glm::vec3 target = glm::vec3(0.0f, 0.5f, 0.0f);

	if(Input::GetMouseButton(MouseCode::Right))
	{
		yawVelocity += Input::GetMouseVelocityX();
		pitchVelocity += Input::GetMouseVelocityY();
	}

	yawVelocity = Lerp(yawVelocity, 0.0f, deltaTime * 6.0);
	pitchVelocity = Lerp(pitchVelocity, 0.0f, deltaTime * 6.0);

	lookAtYaw += panSpeed * yawVelocity * deltaTime;
	lookAtPitch += panSpeed * pitchVelocity * deltaTime;

	lookAtPitch = glm::clamp(lookAtPitch, -89.0f, 89.0f);

	float yawRadians = glm::radians(lookAtYaw);
	float pitchRadians = glm::radians(lookAtPitch);

	glm::vec3 spherical;
	spherical.x = cos(yawRadians) * cos(pitchRadians);
	spherical.y = sin(pitchRadians);
	spherical.z = sin(yawRadians) * cos(pitchRadians);

	spherical = glm::normalize(spherical) * scrollDistance;
	Position = spherical + target;

	scrollVelocity += Input::GetScollVelocity();
	scrollVelocity = Lerp(scrollVelocity, 0.0f, deltaTime * 6.0);
	
	scrollDistance += zoomSpeed * scrollVelocity * deltaTime;
	
	if(scrollDistance < minimumZoomSpacing)
	{
		scrollDistance = minimumZoomSpacing;
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