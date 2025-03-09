#pragma once

#include <unordered_map>
#include <string>

class Texture;

// TODO: Add a way to remove a specific texture
// TODO: Add a function to clear all texture data (useful when switching between scenes)

class TextureManager
{
public:
	static void AddTexture(const std::string& path, Texture* texture);
	static Texture* LoadTexture(const std::string& path);
	static Texture* GetTexture(const std::string& path);
	static bool IsStored(const std::string& path);

private:
	static std::unordered_map<std::string, Texture*> textureAssets;
};