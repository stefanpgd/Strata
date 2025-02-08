#include "Graphics/TextureManager.h"
#include "Graphics/Texture.h"

std::unordered_map<std::string, Texture*> TextureManager::textureAssets; 
Texture* TextureManager::errorTexture;

Texture* TextureManager::LoadTexture(const std::string& path)
{
	if(IsStored(path))
	{
		return textureAssets[path];
	}

	Texture* newTexture = new Texture(path);
	textureAssets.insert(std::make_pair(path, newTexture));

	return newTexture;
}

void TextureManager::Intialize()
{
	errorTexture = new Texture("Assets/Textures/missing.png");
}

void TextureManager::AddTexture(const std::string& path, Texture* texture)
{
	textureAssets.insert(std::make_pair(path, texture));
}

Texture* TextureManager::GetTexture(const std::string& path)
{
	return textureAssets[path];
}

bool TextureManager::IsStored(const std::string& path)
{
	return textureAssets.find(path) != textureAssets.end();
}

Texture* TextureManager::GetErrorTexture()
{
	return errorTexture;
}
