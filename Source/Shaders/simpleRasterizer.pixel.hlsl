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
    float4 albedo = Diffuse.Sample(LinearSampler, IN.TexCoord0);
    
    float3 lightDir = normalize(float3(0.1, -1.0f, 0.5f));
    float diff = saturate(dot(IN.Normal, -lightDir));
    diff = max(diff, 0.25f);
    
    if(albedo.a < 0.5)
    {
        discard;
    }
    
    return float4(albedo.rgb * diff, 1.0f);
}