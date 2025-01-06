#include "Common.hlsl"

struct ShadowInfo
{
    float3 cameraPosition;
    float3 sunDirection;
};
ConstantBuffer<ShadowInfo> shadowInfo : register(b0);

[shader("miss")]
void Miss(inout HitInfo payload : SV_RayPayload)
{
    payload.color = float3(1.0, 1.0, 1.0);
}