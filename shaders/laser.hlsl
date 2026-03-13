// laser.hlsl - Volumetric laser beam with glow and additive blending
// For beat-reactive laser effects in Phase 19

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    float3 LaserColor;
    float Intensity;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float Intensity : TEXCOORD1;
};

// Vertex Shader
PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    
    // Transform position to world space
    float4 worldPos = mul(float4(input.Position, 1.0), World);
    
    // Transform to view space
    float4 viewPos = mul(worldPos, View);
    
    // Transform to projection space
    output.Position = mul(viewPos, Projection);
    
    // Pass through texture coordinates and intensity
    output.TexCoord = input.TexCoord;
    output.Intensity = Intensity;
    
    return output;
}

// Pixel Shader - Volumetric glow with additive blending
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Distance from beam center (TexCoord.x: 0=left edge, 0.5=center, 1=right edge)
    float distFromCenter = abs(input.TexCoord.x - 0.5) * 2.0;  // [0, 1]
    
    // Glow falloff (sharper in center, fades at edges)
    float glowFalloff = 1.0 - saturate(distFromCenter);
    glowFalloff = pow(glowFalloff, 2.5);  // Power curve for sharper beam
    
    // Laser color with intensity modulation
    float3 finalColor = LaserColor * input.Intensity * glowFalloff;
    
    // Alpha for additive blending (higher in center)
    float alpha = glowFalloff * input.Intensity;
    
    return float4(finalColor, alpha);
}
