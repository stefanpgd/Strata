#pragma once

#include <array>

/// <summary>
/// Takes care of some of the default UI present in the project, such as the menu bar. 
/// </summary>
class Editor
{
public:
	Editor();

	void Update(float deltaTime);

private:
	void ImGuiStyleSettings();

private:
	float deltaTime;
	std::array<float, 300> deltaTimeLog;
	unsigned int frameCount = 0;

	struct ImFont* baseFont;
	struct ImFont* boldFont;
};