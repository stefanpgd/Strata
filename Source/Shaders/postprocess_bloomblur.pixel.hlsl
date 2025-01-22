struct PixelIN
{
    float2 TexCoord0 : TexCoord0;
};

struct BloomSettings
{
    bool isHorizontal;
};
ConstantBuffer<BloomSettings> Settings : register(b0);

Texture2D<float4> emissiveTexture: register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PixelIN IN) : SV_TARGET
{
    unsigned int width;
    unsigned int height;
    emissiveTexture.GetDimensions(width, height);
    float2 stepSize = float2(1.0, 1.0) / float2(width, height);
    
    float gauss[5] = 
    { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    
    float3 result = emissiveTexture.Sample(LinearSampler, IN.TexCoord0).rgb * gauss[0];
    
    float2 direction = 0.0f;
    if(Settings.isHorizontal)
    {
        direction = float2(stepSize.x, 0.0f);
    }
    else
    {
        direction = float2(0.0f, stepSize.y);
    }
    
    for (int i = 1; i < 5; i++)
    {
        float3 up = emissiveTexture.Sample(LinearSampler, IN.TexCoord0 + (direction * i)).rgb;
        float3 down = emissiveTexture.Sample(LinearSampler, IN.TexCoord0 - (direction * i)).rgb;
        
        result += up * gauss[i];
        result += down * gauss[i];
    }
    
    return float4(result, 1.0f);
}