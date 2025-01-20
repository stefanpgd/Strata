#pragma once

#include <imgui.h>
#include "Graphics/Transform.h"
#include "Graphics/Texture.h"
#include "Graphics/RenderTarget.h"


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

inline void EditorTextureHighlight(Texture* texture, std::string name, glm::vec2 size)
{
	ImGui::Separator();
	ImGui::Columns(2);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = texture->GetSRV();
	ImGui::Image((ImTextureID)gpuHandle.ptr, ImVec2(size.x, size.y));
	ImGui::NextColumn();
	ImGui::Text(name.c_str());

	ImGui::Columns(1);
	ImGui::Separator();
}

inline void EditorRenderTargetHighlight(RenderTarget* renderTarget, std::string name, float size, float aspectRatio)
{
	ImGui::SeparatorText(name.c_str());
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = renderTarget->GetSRV();
	ImGui::Image((ImTextureID)gpuHandle.ptr, ImVec2(size * aspectRatio, size));
	ImGui::Separator();
}