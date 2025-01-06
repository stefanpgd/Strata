#include "Projects/HelloCompute/HelloComputeProject.h"
#include "Projects/HelloCompute/ComputeUVStage.h"

HelloComputeProject::HelloComputeProject()
{
	ProjectName = "Hello Compute";
	computeUVStage = new ComputeUVStage();
}

void HelloComputeProject::Update(float deltaTime)
{
}

void HelloComputeProject::Render(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	computeUVStage->RecordStage(commandList);
}