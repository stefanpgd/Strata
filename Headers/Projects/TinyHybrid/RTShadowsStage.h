#pragma once
#include "Graphics/RenderStage.h"
#include "Graphics/DXCommon.h"
#include "Framework/Mathematics.h"

class Scene;
class Texture;
class DXBuffer;
class DXRayTracingPipeline;
class DXTopLevelAS;
class DXShaderBindingTable;

struct RTShadowInfo
{
	float cameraPosition[3];
	BOOL space;
	float sunDirection[3];
	float stub[57];
};

class RTShadowStage : public RenderStage
{
public:
	RTShadowStage(Scene* sceneToRender);

	void Update(float deltaTime);
	void RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList) override;

public:
	Texture* outputBuffer;

private:
	void CreateShaderResources();
	void InitializePipeline();
	void InitializeShaderBindingTable();

private:
	Scene* activeScene;

	RTShadowInfo shadowInfo;
	DXBuffer* shadowInfoBuffer;

	DXTopLevelAS* TLAS;
	DXRayTracingPipeline* rayTracePipeline;
	DXShaderBindingTable* shaderTable;
};