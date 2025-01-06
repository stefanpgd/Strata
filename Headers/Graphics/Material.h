#pragma once

// TODO:
// Do some bit-shifting/FLAG stuff and send that over to the GPU with a matching checker in Common.hlsl

/// <summary>
/// Default material struct, requires to be 256 bytes since it's used as an buffer that can be mapped too. 
/// </summary>
struct Material
{
	float color[3] = { 1.0f, 1.0f, 1.0f };
};

class Texture;
struct PBRTextureSet
{
	Texture* Albedo = nullptr;
	Texture* Normal = nullptr;
	Texture* RoughnessMetallic = nullptr;
	Texture* AmbientOcclusion = nullptr;
};