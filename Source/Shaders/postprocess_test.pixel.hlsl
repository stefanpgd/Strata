struct PixelIN
{
    float2 TexCoord0 : TexCoord0;
};
Texture2D<float4> SceneOutput : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PixelIN IN) : SV_TARGET
{
    float3 sceneOutput = SceneOutput.Sample(LinearSampler, IN.TexCoord0).rgb;
    return float4(sceneOutput, 1.0f);
}