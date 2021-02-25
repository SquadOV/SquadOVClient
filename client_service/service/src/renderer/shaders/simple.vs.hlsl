cbuffer VsTransforms : register(b0)
{
    float4x4 worldXform; 
}

struct VS_INPUT
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position.w = 1.0;
    output.position.xyz = input.position;
    output.position = mul(output.position, worldXform);
    output.tex = input.tex;
    return output;
}