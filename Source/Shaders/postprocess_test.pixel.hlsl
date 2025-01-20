struct PixelIN
{
    float2 TexCoord0 : TexCoord0;
};
Texture2D<float4> SceneOutput : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PixelIN IN) : SV_TARGET
{
    float3 sceneOutput = SceneOutput.Sample(LinearSampler, IN.TexCoord0).rgb;
    float2 center = float2(0.5, 0.5);
    
    float dist = saturate(length(IN.TexCoord0 - center) * 1.65);
    float visiblity = 1.0 - dist;
    
    return float4(sceneOutput * visiblity, 1.0f);
}