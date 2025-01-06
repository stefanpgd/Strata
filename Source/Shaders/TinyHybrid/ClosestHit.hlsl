#include "Common.hlsl"

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 texCoord0;
};
StructuredBuffer<Vertex> VertexData : register(t0);
StructuredBuffer<int> indices : register(t1);
RaytracingAccelerationStructure SceneBVH : register(t2);

struct ShadowInfo
{
    float3 cameraPosition;
    float3 sunDirection;
};
ConstantBuffer<ShadowInfo> shadowInfo : register(b0);

[shader("closesthit")]
void ClosestHit(inout HitInfo payload, Attributes attrib)
{
    if(payload.depth != 0)
    {
        // Means we've already hit geometry, so we want to return black marking it as shadow
        payload.color = float3(0.0, 0.0, 0.0);
        return;
    }
    
    // Trace again, this time towards the "sun"
    payload.depth = 1;
    float3 intersection = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    
    RayDesc shadowRay;
    shadowRay.Origin = intersection;
    shadowRay.Direction = shadowInfo.sunDirection;
    shadowRay.TMin = 0.001f;
    shadowRay.TMax = 100.0f; // trace a hunderd meters 
    
    TraceRay(SceneBVH, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, shadowRay, payload);
    
    return;
}