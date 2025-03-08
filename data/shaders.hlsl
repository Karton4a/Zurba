cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 MVP;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    result.position = mul(MVP, float4(input.position, 1.0f));
    result.normal = input.normal;
    result.uv = input.uv;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}
