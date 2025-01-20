struct PixelIN
{
    float2 TexCoord0 : TexCoord0;
};

float4 main(PixelIN IN) : SV_TARGET
{
    return float4(IN.TexCoord0, 0.0, 1.0f);
}