Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_Target
{
    return gTexture.Sample(gSampler, input.tex);
}