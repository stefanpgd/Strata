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
    float3 lightPosition = float3(0.0, 0.5, 0.0);
    float3 lightColor = float3(1.0, 1.0, 1.0);
    
    float3 dir = lightPosition - IN.WorldPosition;
    float diff = dot(IN.Normal, normalize(dir));
    
    float intensity = 0.8 / pow(length(dir), 1.25);
    
    float3 albedo = Diffuse.Sample(LinearSampler, IN.TexCoord0).rgb;
    float3 outputColor = albedo * lightColor * intensity;
    return float4(outputColor, 1.0f);
}