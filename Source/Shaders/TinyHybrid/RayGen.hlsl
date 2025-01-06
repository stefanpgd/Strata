#include "Common.hlsl"

RWTexture2D<float4> gOutput : register(u0); // Raytracing output texture, accessed as a UAV
RaytracingAccelerationStructure SceneBVH : register(t0); // Raytracing acceleration structure, accessed as a SRV

struct ShadowInfo
{
    float3 cameraPosition;
    float3 sunDirection;
};
ConstantBuffer<ShadowInfo> shadowInfo : register(b0);

float3 GetRayDirection(uint2 launchIndex, float3 cameraPosition)
{
    float2 dimensions = float2(DispatchRaysDimensions().xy);
    float2 uv = launchIndex.xy / dimensions.xy;
    
    // 2) Setup virtual screen plane //
    float aspectRatio = dimensions.x / dimensions.y;
    float xOffset = (aspectRatio - 1.0f) * 0.5f;
    
    float3 direction = float3(0.0f, 0.0f, -1.0f);
    float3 planeOffset = 0.87f;
    
    float3 screenCenter = cameraPosition + (direction * planeOffset);
    
    float3 screenP0 = screenCenter + float3(-0.5 - xOffset, 0.5, 0.0f);
    float3 screenP1 = screenCenter + float3(0.5 + xOffset, 0.5, 0.0f);
    float3 screenP2 = screenCenter + float3(-0.5 - xOffset, -0.5, 0.0f);
    
    float3 screenV = (screenP2 - screenP0);
    float3 screenU = (screenP1 - screenP0);
    
    // 3) Use the plane to find our given ray direction //
    float3 screenPoint = screenP0 + (screenU * uv.x) + (screenV * uv.y);
    float3 rayDirection = normalize(screenPoint - cameraPosition);
    
    return rayDirection;
}

[shader("raygeneration")]
void RayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    gOutput[launchIndex] = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 rayDir = GetRayDirection(launchIndex, shadowInfo.cameraPosition);
    
    RayDesc ray;
    ray.Origin = shadowInfo.cameraPosition;
    ray.Direction = rayDir;
    ray.TMin = 0.001f;
    ray.TMax = 100000;
    
    HitInfo payload;
    payload.depth = 0;
    
    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    gOutput[launchIndex] = float4(payload.color.rrr, 1.0f);
}