#include "Projects/TinyHybrid/RTShadowsStage.h"
#include "Graphics/Texture.h"
#include "Graphics/DXComponents.h"
#include "Graphics/DXR/DXRayTracingPipeline.h"
#include "Graphics/DXR/DXTLAS.h"
#include "Graphics/DXR/DXShaderBindingTable.h"
#include "Framework/Scene.h"

#include "Graphics/Mesh.h"
#include "Graphics/Model.h"
#include "Graphics/Camera.h"

RTShadowStage::RTShadowStage(Scene* sceneToRender) : activeScene(sceneToRender)
{
	CreateShaderResources();

	TLAS = new DXTopLevelAS(sceneToRender);
	InitializePipeline();
	InitializeShaderBindingTable();
}

void RTShadowStage::Update(float deltaTime)
{
	// TODO: Any handier way? Maybe replace the upload buffer with a structred buffer?
	shadowInfo.cameraPosition[0] = activeScene->Camera->Position.x;
	shadowInfo.cameraPosition[1] = activeScene->Camera->Position.y;
	shadowInfo.cameraPosition[2] = activeScene->Camera->Position.z;

	shadowInfo.sunDirection[0] = activeScene->SunDirection.x;
	shadowInfo.sunDirection[1] = activeScene->SunDirection.y;
	shadowInfo.sunDirection[2] = activeScene->SunDirection.z;

	shadowInfoBuffer->UpdateData(&shadowInfo);
}

void RTShadowStage::RecordStage(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	if(!activeScene)
	{
		LOG(Log::MessageType::Error, "Cannot trace shadows if 'scene' was never initialized!");
		return;
	}

	ComPtr<ID3D12Resource> renderTargetBuffer = DXAccess::GetWindow()->GetCurrentScreenBuffer();
	ID3D12Resource* const output = outputBuffer->GetAddress();

	TransitionResource(output, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->SetPipelineState1(rayTracePipeline->GetPipelineState());
	commandList->DispatchRays(shaderTable->GetDispatchRayDescription());
	TransitionResource(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
}

void RTShadowStage::CreateShaderResources()
{
	int width = DXAccess::GetWindow()->GetWindowWidth();
	int height = DXAccess::GetWindow()->GetWindowHeight();

	outputBuffer = new Texture(width, height, DXGI_FORMAT_R8G8B8A8_UNORM);

	DXBufferProperties bufferProperties;
	bufferProperties.isCPUAccessible = true;
	bufferProperties.isConstantBuffer = true;
	bufferProperties.isStructuredBuffer = false;

	shadowInfoBuffer = new DXBuffer(bufferProperties, &shadowInfo, 1, sizeof(RTShadowInfo));
}

void RTShadowStage::InitializePipeline()
{
	DXRayTracingPipelineSettings settings;
	settings.maxRayRecursionDepth = 4;

	settings.rayGenPath = L"Source/Shaders/TinyHybrid/RayGen.hlsl";
	settings.closestHitPath = L"Source/Shaders/TinyHybrid/ClosestHit.hlsl";
	settings.missPath = L"Source/Shaders/TinyHybrid/Miss.hlsl";

	// RayGen Root //
	CD3DX12_DESCRIPTOR_RANGE rayGenRange[1];
	rayGenRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0); // Output Buffer 

	CD3DX12_ROOT_PARAMETER rayGenParameters[3];
	rayGenParameters[0].InitAsDescriptorTable(_countof(rayGenRange), &rayGenRange[0]);
	rayGenParameters[1].InitAsShaderResourceView(0, 0); // BVH
	rayGenParameters[2].InitAsConstantBufferView(0, 0);

	settings.rayGenParameters = &rayGenParameters[0];
	settings.rayGenParameterCount = _countof(rayGenParameters);

	// Hit Root //
	CD3DX12_ROOT_PARAMETER hitParameters[4];
	hitParameters[0].InitAsShaderResourceView(0, 0); // Vertex buffer
	hitParameters[1].InitAsShaderResourceView(1, 0); // Index buffer
	hitParameters[2].InitAsShaderResourceView(2, 0); // TLAS Scene 
	hitParameters[3].InitAsConstantBufferView(0, 0);

	settings.hitParameters = &hitParameters[0];
	settings.hitParameterCount = _countof(hitParameters);

	// Miss Root //
	settings.missParameters = nullptr;
	settings.missParameterCount = 0;

	settings.payLoadSize = sizeof(float) * 4; // RGB, Depth, Seed
	rayTracePipeline = new DXRayTracingPipeline(settings);
}

void RTShadowStage::InitializeShaderBindingTable()
{
	if(!shaderTable)
	{
		shaderTable = new DXShaderBindingTable(rayTracePipeline->GetPipelineProperties());
	}

	// Overlapping entries //
	auto shadowInfoPtr = reinterpret_cast<UINT64*>(shadowInfoBuffer->GetGPUVirtualAddress());

	// Ray Gen Entry //
	DXDescriptorHeap* heap = DXAccess::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto targetBuffer = reinterpret_cast<UINT64*>(outputBuffer->GetUAV().ptr);
	auto tlasPtr = reinterpret_cast<UINT64*>(TLAS->GetGPUVirtualAddress());
	shaderTable->AddRayGenerationProgram(L"RayGen", { targetBuffer, tlasPtr, shadowInfoPtr });

	// Mis Entry //
	shaderTable->AddMissProgram(L"Miss", { });

	// Hit Entries //
	const std::vector<Model*>& models = activeScene->GetModels();
	for(Model* model : models)
	{
		const std::vector<Mesh*>& meshes = model->GetMeshes();
		for(Mesh* mesh : meshes)
		{
			auto vertex = reinterpret_cast<UINT64*>(mesh->GetVertexBuffer()->GetGPUVirtualAddress());
			auto index = reinterpret_cast<UINT64*>(mesh->GetIndexBuffer()->GetGPUVirtualAddress());
			shaderTable->AddHitProgram(L"HitGroup", { vertex, index, tlasPtr, shadowInfoPtr });
		}
	}

	shaderTable->BuildShaderTable();
}