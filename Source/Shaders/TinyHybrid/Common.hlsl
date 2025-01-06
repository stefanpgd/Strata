// REGION - Buffers & Materials //
struct HitInfo
{
    float3 color;
    float depth;
};

// Attributes output by the raytracing when hitting a surface,
// in this instance the barycentric coordinates
struct Attributes
{
    float2 bary;
};
static float PI = 3.14159265;