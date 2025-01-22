struct PixelIN
{
    float2 TexCoord0 : TexCoord0;
};

Texture2D<float4> SceneOutput : register(t0);
Texture2D<float4> BloomOutput : register(t1);

SamplerState LinearSampler : register(s0);

float4 main(PixelIN IN) : SV_TARGET
{
    float2 texcoords = IN.TexCoord0;
    
    float3 scene = SceneOutput.Sample(LinearSampler, IN.TexCoord0).rgb;
    float3 bloom = BloomOutput.Sample(LinearSampler, IN.TexCoord0).rgb;
    
    float3 result = scene + bloom;
    return float4(result, 1.0f);
}