// textured.hlsl - Textured rendering shader for Phase 3
// Implements UV mapping with texture sampling and checkerboard pattern

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
    float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 WorldNormal : NORMAL;
    float3 WorldPos : TEXCOORD1;
};

// Texture resource and sampler
Texture2D mainTexture : register(t0);
SamplerState mainSampler : register(s0);

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
    
    // Pass through texture coordinates
    output.TexCoord = input.TexCoord;
    
    // Transform normal to world space
    output.WorldNormal = mul(input.Normal, (float3x3)World);
    
    return output;
}

// Pixel Shader
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Sample texture using UV coordinates
    float4 texColor = mainTexture.Sample(mainSampler, input.TexCoord);
    
    // Simple lighting: ambient + diffuse
    float3 lightDir = normalize(LightDir);
    float3 normal = normalize(input.WorldNormal);
    
    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.3;
    float lighting = ambient + diffuse * 0.7;
    
    // Apply lighting to texture color
    float4 finalColor = texColor * float4(lighting, lighting, lighting, 1.0);
    
    return finalColor;
}
