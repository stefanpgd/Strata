struct PixelIN
{
    float3 WorldPosition : WorldPosition;
    float3 Normal : Normal;
    float2 TexCoord0 : TexCoord0;
};

struct SceneInfo
{
    float3 sunDirection;
};
ConstantBuffer<SceneInfo> sceneInfo : register(b0);

Texture2D Diffuse : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PixelIN IN) : SV_TARGET
{
    float3 albedo = Diffuse.Sample(LinearSampler, IN.TexCoord0).rgb;
    float diffuse = saturate(dot(IN.Normal, sceneInfo.sunDirection));
    
    // Ambient + Diffuse 
    float3 output = (albedo * 0.04) + albedo * diffuse;
    return float4(output, 1.0f);
}