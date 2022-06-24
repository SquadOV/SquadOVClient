Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer PsConstants : register(b0)
{
    // dims of the OUTPUT texture.
    float2 dims;
    float2 invDims;
    uint mode;
    uint hasTexture;
    float opacity;
    uint useOpacity;
}

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 color : COLOR0;
};

float4 multisampleTextureFilter(float2 inUv) {
    // So based on https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
    // Let's use the same standard 8 sample pattern that DirectX does.
    //
    // I *think* what ddx and ddy will give me here is effectively the difference in the UV from one output pixel to the next in either
    // the X or Y direction. If I assume that I'm covering a single pixel, then ddx(uv.x) is the difference of the UV from the left to the right.
    // So going 0.5 the partial derivative in either direction will get me to the next pixel.
    //
    // Therefore it makes to perform the sampling within this region.

    float4 sum = 0.f;
    float2 bounds = float2(ddx(inUv.x), ddy(inUv.y));

    float2 s1 = 0.0625f * bounds;
    float2 s3 = 0.1875f * bounds;
    float2 s5 = 0.3125f * bounds;
    float2 s7 = 0.4375f * bounds;

    sum += gTexture.Sample(gSampler, inUv + float2(s1.x, -s3.y));
    sum += gTexture.Sample(gSampler, inUv + float2(-s1.x, s3.y));
    sum += gTexture.Sample(gSampler, inUv + float2(s5.x, s1.y));
    sum += gTexture.Sample(gSampler, inUv + float2(-s3.x, -s5.y));
    sum += gTexture.Sample(gSampler, inUv + float2(-s5.x, s5.y));
    sum += gTexture.Sample(gSampler, inUv + float2(-s7.x, -s1.y));
    sum += gTexture.Sample(gSampler, inUv + float2(s3.x, s7.y));
    sum += gTexture.Sample(gSampler, inUv + float2(s7.x, -s7.y));

    return sum / 8.f;
}


float4 cubicWeight(float t, float tau) {
    return float4(
        -1.f * tau * t + 2 * tau * pow(t, 2) - tau * pow(t, 3),
        1.f + (tau - 3.f) * pow(t, 2) + (2.f - tau) * pow(t, 3),
        tau * t + (3.f - 2.f * tau) * pow(t, 2) + (tau - 2.f) * pow(t, 3),
        -1.f * tau * pow(t, 2) + tau * pow(t, 3)
    );
}

float4 cubicInterpolation(float4 p0, float4 p1, float4 p2, float4 p3, float t) {
    // Note that we'll be using the Catmull Rom interpolation from here: https://www.cs.cmu.edu/~fp/courses/graphics/asst5/catmullRom.pdf
    // In our case
    //  p_{i-2} = p0
    //  p_{i-1} = p1
    //  p_{i} = p2
    //  p_{i+1} = p3
    float4 w = cubicWeight(t, 0.75);
    return w.r * p0 + w.g * p1 + w.b * p2 + w.a * p3;
}

float4 bicubicInterpolation(float2 inUv) {
    // So bicubicInterpolation assumes we're interpolating from the four corners of a unit square.
    // So the question becomes, which unit square are we in - size and location. Let's assume that the
    // center of any particular pixel is at (0.5, 0.5). If the input UV is greater than (0.5, 0.5) in that pixel,
    // then the current pixel is the top left corner we'll use for interpolation. If x < 0.5, then we'll need
    // to shift the interpolation grid one pixel to the left. If y < 0.5, then we'll need to shift the interpolation
    // grid one pixel to the top.
    float2 xy = inUv * dims;
    float2 xy0 = floor(xy - 0.5f) + 0.5f;
    float2 t = xy - xy0;

    // Bicubic interpolation relies on having 16 pixels.
    // From the above, we have xy0 which is the coordinate f(0,0).
    // From there, we need the f(-1, -1) to f(2, 2) values.
    // For variable naming, we'll say that f(-1, 1) = f00 and f(2, 2) = f33.
    float4 p00 = gTexture.Sample(gSampler, (xy0 + float2(-1.f, -1.f)) * invDims);
    float4 p01 = gTexture.Sample(gSampler, (xy0 + float2(-1.f, 0.f)) * invDims);
    float4 p02 = gTexture.Sample(gSampler, (xy0 + float2(-1.f, 1.f)) * invDims);
    float4 p03 = gTexture.Sample(gSampler, (xy0 + float2(-1.f, 2.f)) * invDims);
    float4 p10 = gTexture.Sample(gSampler, (xy0 + float2(0.f, -1.f)) * invDims);
    float4 p11 = gTexture.Sample(gSampler, (xy0 + float2(0.f, 0.f)) * invDims);
    float4 p12 = gTexture.Sample(gSampler, (xy0 + float2(0.f, 1.f)) * invDims);
    float4 p13 = gTexture.Sample(gSampler, (xy0 + float2(0.f, 2.f)) * invDims);
    float4 p20 = gTexture.Sample(gSampler, (xy0 + float2(1.f, -1.f)) * invDims);
    float4 p21 = gTexture.Sample(gSampler, (xy0 + float2(1.f, 0.f)) * invDims);
    float4 p22 = gTexture.Sample(gSampler, (xy0 + float2(1.f, 1.f)) * invDims);
    float4 p23 = gTexture.Sample(gSampler, (xy0 + float2(1.f, 2.f)) * invDims);
    float4 p30 = gTexture.Sample(gSampler, (xy0 + float2(2.f, -1.f)) * invDims);
    float4 p31 = gTexture.Sample(gSampler, (xy0 + float2(2.f, 0.f)) * invDims);
    float4 p32 = gTexture.Sample(gSampler, (xy0 + float2(2.f, 1.f)) * invDims);
    float4 p33 = gTexture.Sample(gSampler, (xy0 + float2(2.f, 2.f)) * invDims);

    float4 p0 = cubicInterpolation(p01, p01, p02, p03, t.y);
    float4 p1 = cubicInterpolation(p11, p11, p12, p13, t.y);
    float4 p2 = cubicInterpolation(p21, p21, p22, p23, t.y);
    float4 p3 = cubicInterpolation(p31, p31, p32, p33, t.y);
    return cubicInterpolation(p0, p1, p2, p3, t.x);

}

float4 computeColor(PS_INPUT input) {
    if (hasTexture) {
        if (mode == 1) {
            return bicubicInterpolation(input.tex);
        } else if (mode == 2) {
            return multisampleTextureFilter(input.tex);
        } else {
            // Whatever's in the sampler (naive)
            return gTexture.Sample(gSampler, input.tex);
        }
    } else {
        return input.color;
    }
}

float4 main(PS_INPUT input) : SV_Target
{
    float4 color = computeColor(input);
    return float4(color.rgb, useOpacity ? color.a * opacity :  1.f);
}