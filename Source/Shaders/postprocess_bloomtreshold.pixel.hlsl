struct PixelIN
{
    float2 TexCoord0 : TexCoord0;
};

struct TresholdSettings
{
    float treshold;
};
ConstantBuffer<TresholdSettings> Settings : register(b0);

Texture2D<float4> SceneOutput : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PixelIN IN) : SV_TARGET
{
    float3 sceneOutput = SceneOutput.Sample(LinearSampler, IN.TexCoord0).rgb;
    float lum = dot(sceneOutput, float3(0.2126, 0.7152, 0.0722));
    
    float3 output = 0.0f;
    
    if(lum > Settings.treshold)
    {
        output = min(sceneOutput, float3(5.0f, 5.0f, 5.0f));
    }
    
    return float4(output, 1.0f);
}