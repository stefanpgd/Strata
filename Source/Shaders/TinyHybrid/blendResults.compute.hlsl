RWTexture2D<float4> sceneRender : register(u0);
RWTexture2D<float4> shadowMask : register(u1);
RWTexture2D<float4> outputBuffer : register(u2);

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    unsigned int width;
    unsigned int height;
    sceneRender.GetDimensions(width, height);
    
    if(dispatchThreadID.x > width || dispatchThreadID.y > height)
    {
        return;
    }
    
    float3 sceneColor = sceneRender[dispatchThreadID.xy].rgb;
    float shadow = shadowMask[dispatchThreadID.xy].r;
    
    // Shadow - 0 means it has no light
    // Shadow - 1 means there is light
    float ambientStrength = 0.2f;
    shadow = clamp(shadow, ambientStrength, 1.0f);
    
    outputBuffer[dispatchThreadID.xy] = float4(sceneColor * shadow, 1.0f);

}