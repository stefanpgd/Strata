struct PixelIN
{
    float2 TexCoord0 : TexCoord0;
};

struct ExposureSettings
{
    float exposure;
    float gamma;
};
ConstantBuffer<ExposureSettings> Settings : register(b0);

Texture2D<float4> hdrSceneTexture : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PixelIN IN) : SV_TARGET
{
    float3 color = hdrSceneTexture.Sample(LinearSampler, IN.TexCoord0).rgb;
    color = pow(max(color, 0.0), 2.24); // might have to move to scene fragment shader
    
    float3 mapped = float3(1.0, 1.0, 1.0) - exp(-color * Settings.exposure);
    
    float invGamma = 1.0 / Settings.gamma;
    mapped = pow(max(mapped, 0.0), invGamma);
    
    return float4(mapped, 1.0f);
}