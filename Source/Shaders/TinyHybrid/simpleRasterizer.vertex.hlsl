struct Vertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord0 : TEXCOORD0;
};

struct VertexShaderOutput
{
    float3 WorldPosition : WorldPosition;
    float3 Normal : Normal;
    float2 TexCoord0 : TexCoord0;
    float4 Position : SV_Position;
};

struct TransformData
{
    matrix MVP;
    matrix Model;
};
ConstantBuffer<TransformData> transformData : register(b0);

VertexShaderOutput main(Vertex IN)
{
	VertexShaderOutput OUT;
    
    OUT.Position = mul(transformData.MVP, float4(IN.Position, 1.0f));
    OUT.WorldPosition = mul(transformData.Model, float4(IN.Position, 1.0f)).rgb;
    OUT.Normal = normalize(mul(transformData.Model, float4(IN.Normal, 0.0f)).xyz);
    OUT.TexCoord0 = IN.TexCoord0;
    
    return OUT;
}