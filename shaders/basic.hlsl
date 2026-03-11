// basic.hlsl - Basic vertex and pixel shader for Phase 18
// Implements simple Phong lighting with WVP transformation

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    float3 LightDir;
    float Padding;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL;
    float3 WorldPos : TEXCOORD0;
};

// Vertex Shader
PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    
    // Transform position to world space
    float4 worldPos = mul(float4(input.Position, 1.0), World);
    output.WorldPos = worldPos.xyz;
    
    // Transform to view space
    float4 viewPos = mul(worldPos, View);
    
    // Transform to projection space
    output.Position = mul(viewPos, Projection);
    
    // Transform normal to world space
    output.WorldNormal = mul(input.Normal, (float3x3)World);
    
    return output;
}

// Pixel Shader - Simple Phong lighting
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Normalize vectors
    float3 normal = normalize(input.WorldNormal);
    float3 lightDir = normalize(LightDir);
    
    // Diffuse lighting (Lambertian)
    float ndotl = max(dot(normal, lightDir), 0.0);
    
    // Ambient + diffuse
    float3 ambient = float3(0.2, 0.2, 0.2);
    float3 diffuse = float3(0.8, 0.8, 0.8) * ndotl;
    
    float3 finalColor = ambient + diffuse;
    
    return float4(finalColor, 1.0);
}
