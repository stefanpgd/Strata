struct PixelIN
{
    float3 WorldPosition : WorldPosition;
    float3 Normal : Normal;
    float2 TexCoord0 : TexCoord0;
};

Texture2D Diffuse : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PixelIN IN) : SV_TARGET
{
    float3 albedo = Diffuse.Sample(LinearSampler, IN.TexCoord0).rgb;
    albedo = pow(max(albedo, 0.0), 2.24); // might have to move to scene fragment shader
    
    return float4(albedo, 1.0f);
}