// colorgrade.hlsl - Mood-based color grading
// Applies color LUT based on crowd mood

Texture2D SourceTexture : register(t0);
SamplerState LinearSampler : register(s0);

cbuffer ColorGradeConstants : register(b0) {
    int Mood;  // 0=Unimpressed, 1=Calm, 2=Grooving, 3=Hyped
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

// Pixel Shader - Apply color grade LUT
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Sample source texture
    float3 color = SourceTexture.Sample(LinearSampler, input.TexCoord).rgb;
    
    // Mood-based color LUTs
    // 0=Unimpressed (neutral), 1=Calm (blue), 2=Grooving (green), 3=Hyped (red)
    static const float3 moodLUTs[4] = {
        float3(1.0, 1.0, 1.0),   // Neutral
        float3(0.8, 0.9, 1.2),   // Blue tint
        float3(0.9, 1.1, 0.9),   // Green tint
        float3(1.2, 0.8, 0.8)    // Red tint
    };
    
    // Clamp mood to valid range
    int moodIndex = clamp(Mood, 0, 3);
    
    // Apply color grade
    float3 gradedColor = color * moodLUTs[moodIndex];
    
    return float4(gradedColor, 1.0);
}
