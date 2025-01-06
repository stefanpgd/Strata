#pragma once

#include <imgui.h>
#include "Graphics/Transform.h"

// A collection of common editor elements that are nice to have to quickly iterate
// For example: Display Transform information
// If any UI element/window is usually re-occuring in project(s), then it can probably be moved in here
inline void EditorTransformInfo(Transform& transform)
{
	ImGui::DragFloat3("Position", &transform.Position[0], 0.1f);
	ImGui::DragFloat3("Rotation", &transform.Rotation[0], 1.0f);

	float scale = transform.Scale[0];
	if(ImGui::DragFloat("Scale", &scale, 0.1f))
	{
		transform.Scale = glm::vec3(scale);
	}
}