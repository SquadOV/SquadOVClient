Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer PsConstants : register(b0)
{
    float inWidth;
    float inHeight;
    uint mode;
    uint hasTexture;
}

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 color : COLOR0;
};

float4 bilinearInterpolation(float2 parentPixel, float2 diff) {
    const float2 offsets = sign(diff) * float2(1.f, 1.f);
    const float2 dims = float2(inWidth, inHeight);

    const float2 tlUv = parentPixel / dims;
    const float2 trUv = (parentPixel + float2(offsets.x, 0.f)) / dims;
    const float2 blUv = (parentPixel + float2(0.f, offsets.y)) / dims;
    const float2 brUv = (parentPixel + offsets) / dims;

    const float4 tlPixel = gTexture.Sample(gSampler, tlUv);
    const float4 trPixel = gTexture.Sample(gSampler, trUv);
    const float4 blPixel = gTexture.Sample(gSampler, blUv);
    const float4 brPixel = gTexture.Sample(gSampler, brUv);

    const float2 absDiff = abs(diff);
    const float4 tPixel = absDiff.x * tlPixel + (1.f - absDiff.x) * trPixel;
    const float4 bPixel = absDiff.x * blPixel + (1.f - absDiff.x) * brPixel;
    return absDiff.y * tPixel + (1.f - absDiff.y) * bPixel;
}

float4 cubicInterpolation(float4 p0, float4 p1, float4 p2, float4 p3, float x) {
    return p1 + 0.5f * x*(p2 - p0 + x*(2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3 + x*(3.0f*(p1 - p2) + p3 - p0)));
}

float4 bicubicInterpolation(float2 parentPixel, float2 diff) {
    const int2 diffSigns = sign(diff);
    if (diffSigns.x < 0) {
        parentPixel.x -= 1.f;
        diff.x += 1.f;
    }

    if (diffSigns.y < 0) {
        parentPixel.y -= 1.f;
        diff.y += 1.f;
    }

    const float2 dims = float2(inWidth, inHeight);
    // Bicubic interpolation relies on having 16 pixels.
    // We assume the pixel the sample is on is p(1, 1) and we want
    // all p(x, y) where 0 <= x <= 3 and 0 <= y <= 3.
    float4 p00 = gTexture.Sample(gSampler, (parentPixel + float2(-1.f, -1.f)) / dims);
    float4 p01 = gTexture.Sample(gSampler, (parentPixel + float2(-1.f, 0.f)) / dims);
    float4 p02 = gTexture.Sample(gSampler, (parentPixel + float2(-1.f, 1.f)) / dims);
    float4 p03 = gTexture.Sample(gSampler, (parentPixel + float2(-1.f, 2.f)) / dims);
    float4 p10 = gTexture.Sample(gSampler, (parentPixel + float2(0.f, -1.f)) / dims);
    float4 p11 = gTexture.Sample(gSampler, (parentPixel + float2(0.f, 0.f)) / dims);
    float4 p12 = gTexture.Sample(gSampler, (parentPixel + float2(0.f, 1.f)) / dims);
    float4 p13 = gTexture.Sample(gSampler, (parentPixel + float2(0.f, 2.f)) / dims);
    float4 p20 = gTexture.Sample(gSampler, (parentPixel + float2(1.f, -1.f)) / dims);
    float4 p21 = gTexture.Sample(gSampler, (parentPixel + float2(1.f, 0.f)) / dims);
    float4 p22 = gTexture.Sample(gSampler, (parentPixel + float2(1.f, 1.f)) / dims);
    float4 p23 = gTexture.Sample(gSampler, (parentPixel + float2(1.f, 2.f)) / dims);
    float4 p30 = gTexture.Sample(gSampler, (parentPixel + float2(2.f, -1.f)) / dims);
    float4 p31 = gTexture.Sample(gSampler, (parentPixel + float2(2.f, 0.f)) / dims);
    float4 p32 = gTexture.Sample(gSampler, (parentPixel + float2(2.f, 1.f)) / dims);
    float4 p33 = gTexture.Sample(gSampler, (parentPixel + float2(2.f, 2.f)) / dims);

    // Recompute the diff so that it's the correct xy on this new grid.
    // For any given dimension p0 -> -1, p1 -> 0, p2 -> 1, p3 -> 2
    float4 p0 = cubicInterpolation(p01, p01, p02, p03, diff.y);
    float4 p1 = cubicInterpolation(p11, p11, p12, p13, diff.y);
    float4 p2 = cubicInterpolation(p21, p21, p22, p23, diff.y);
    float4 p3 = cubicInterpolation(p31, p31, p32, p33, diff.y);
    return cubicInterpolation(p0, p1, p2, p3, diff.x);
}

static const float PI = 3.14159265f;

float lanczosKernel1d(float x) {
    if (x == 0.f) {
        return 1.f;
    } else if (x >= -2.f && x < 2.f) {
        return 2.f * sin(PI * x) * sin(PI * x / 2.f) / PI / PI / x / x;
    } else {
        return 0.f;
    }
}

float lanczosKernel2d(float2 xy) {
    return lanczosKernel1d(xy.x) * lanczosKernel1d(xy.y);
}

float4 lanczosInterpolation(float2 parentPixel, float2 diff) {
    // We're going to use a Lanczos kernel of size 2 whic gives us a set of pixels that is more or less equivalent to the ones used for bicubic interpolation.cubicInterpolation
    const float2 dims = float2(inWidth, inHeight);
    float4 p00 = gTexture.Sample(gSampler, (parentPixel + float2(-1.f, -1.f)) / dims);
    float4 p01 = gTexture.Sample(gSampler, (parentPixel + float2(-1.f, 0.f)) / dims);
    float4 p02 = gTexture.Sample(gSampler, (parentPixel + float2(-1.f, 1.f)) / dims);
    float4 p03 = gTexture.Sample(gSampler, (parentPixel + float2(-1.f, 2.f)) / dims);
    float4 p10 = gTexture.Sample(gSampler, (parentPixel + float2(0.f, -1.f)) / dims);
    float4 p11 = gTexture.Sample(gSampler, (parentPixel + float2(0.f, 0.f)) / dims);
    float4 p12 = gTexture.Sample(gSampler, (parentPixel + float2(0.f, 1.f)) / dims);
    float4 p13 = gTexture.Sample(gSampler, (parentPixel + float2(0.f, 2.f)) / dims);
    float4 p20 = gTexture.Sample(gSampler, (parentPixel + float2(1.f, -1.f)) / dims);
    float4 p21 = gTexture.Sample(gSampler, (parentPixel + float2(1.f, 0.f)) / dims);
    float4 p22 = gTexture.Sample(gSampler, (parentPixel + float2(1.f, 1.f)) / dims);
    float4 p23 = gTexture.Sample(gSampler, (parentPixel + float2(1.f, 2.f)) / dims);
    float4 p30 = gTexture.Sample(gSampler, (parentPixel + float2(2.f, -1.f)) / dims);
    float4 p31 = gTexture.Sample(gSampler, (parentPixel + float2(2.f, 0.f)) / dims);
    float4 p32 = gTexture.Sample(gSampler, (parentPixel + float2(2.f, 1.f)) / dims);
    float4 p33 = gTexture.Sample(gSampler, (parentPixel + float2(2.f, 2.f)) / dims);

    float l00 = lanczosKernel2d(diff - float2(-1.f, -1.f));
    float l01 = lanczosKernel2d(diff - float2(-1.f, 0.f));
    float l02 = lanczosKernel2d(diff - float2(-1.f, 1.f));
    float l03 = lanczosKernel2d(diff - float2(-1.f, 2.f));
    float l10 = lanczosKernel2d(diff - float2(0.f, -1.f));
    float l11 = lanczosKernel2d(diff - float2(0.f, 0.f));
    float l12 = lanczosKernel2d(diff - float2(0.f, 1.f));
    float l13 = lanczosKernel2d(diff - float2(0.f, 2.f));
    float l20 = lanczosKernel2d(diff - float2(1.f, -1.f));
    float l21 = lanczosKernel2d(diff - float2(1.f, 0.f));
    float l22 = lanczosKernel2d(diff - float2(1.f, 1.f));
    float l23 = lanczosKernel2d(diff - float2(1.f, 2.f));
    float l30 = lanczosKernel2d(diff - float2(2.f, -1.f));
    float l31 = lanczosKernel2d(diff - float2(2.f, 0.f));
    float l32 = lanczosKernel2d(diff - float2(2.f, 1.f));
    float l33 = lanczosKernel2d(diff - float2(2.f, 2.f));

    float normalization = l00 + l01 + l02 + l03 +
        l10 + l11 + l12 + l13 +
        l20 + l21 + l22 + l23 +
        l30 + l31 + l32 + l33;

    return (l00 * p00 + l01 * p01 + l02 * p02 + l03 * p03 +
        l10 * p10 + l11 * p11 + l12 * p12 + l13 * p13 +
        l20 * p20 + l21 * p21 + l22 * p22 + l23 * p23 +
        l30 * p30 + l31 * p31 + l32 * p32 + l33 * p33) / normalization;
}

float4 main(PS_INPUT input) : SV_Target
{
    if (hasTexture) {
        float2 inPixel = input.tex * float2(inWidth - 1, inHeight - 1);
        float2 diff = fmod(inPixel, float2(1.f, 1.f)) - float2(0.5f, 0.5f);
        inPixel -= diff;

        if (mode == 1) {
            // Bilinear
            return bilinearInterpolation(inPixel, diff);
        } else if (mode == 2) {
            // Bicubic
            return bicubicInterpolation(inPixel, diff);
        } else if (mode == 3) {
            // Lanczos
            return lanczosInterpolation(inPixel, diff);
        } else {
            // Whatever's in the sampler (naive)
            return gTexture.Sample(gSampler, input.tex);
        }
    } else {
        return input.color;
    }
}