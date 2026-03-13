// bloom.hlsl - Gaussian bloom post-processing effect
// Two-pass separable Gaussian blur (horizontal + vertical)

Texture2D SourceTexture : register(t0);
SamplerState LinearSampler : register(s0);

cbuffer BloomConstants : register(b0) {
    float2 TextureSize;  // (width, height)
    float2 BlurDirection;  // (1,0) for horizontal, (0,1) for vertical
    float Intensity;
    float3 Padding;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

// Vertex Shader (fullscreen quad)
PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    output.Position = float4(input.Position, 1.0);
    output.TexCoord = input.TexCoord;
    return output;
}

// Pixel Shader - Gaussian blur
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Gaussian weights (5-tap)
    static const float weights[5] = {
        0.227027,
        0.1945946,
        0.1216216,
        0.054054,
        0.016216
    };
    
    float2 texelSize = 1.0 / TextureSize;
    float2 blurOffset = texelSize * BlurDirection;
    
    // Center sample
    float3 result = SourceTexture.Sample(LinearSampler, input.TexCoord).rgb * weights[0];
    
    // Samples in both directions from center
    for (int i = 1; i < 5; ++i) {
        float2 offset = blurOffset * float(i);
        result += SourceTexture.Sample(LinearSampler, input.TexCoord + offset).rgb * weights[i];
        result += SourceTexture.Sample(LinearSampler, input.TexCoord - offset).rgb * weights[i];
    }
    
    // Apply intensity
    result *= Intensity;
    
    return float4(result, 1.0);
}
