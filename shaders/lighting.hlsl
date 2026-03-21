// lighting.hlsl - Multi-light Phong shading with shadow mapping (Phase 5+)
// Supports up to 8 lights (spot and point) with per-light colors
// Phase 5: Added shadow map with 2x2 PCF filtering

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    float3 CameraPos;
    float Padding0;
};

cbuffer LightingConstants : register(b1) {
    struct Light {
        float3 Position;
        float Intensity;
        float3 Direction;
        float SpotAngle;
        float3 Color;
        int Type;  // 0=Point, 1=Spot
    };
    Light Lights[8];
    int NumLights;
    float3 AmbientColor;
};

cbuffer ShadowConstants : register(b2) {
    matrix LightView;
    matrix LightProj;
    int UseShadows;
    float ShadowBias;
    float2 ShadowMap_TexelSize;
};

// Shadow depth texture and sampler
Texture2D<float> ShadowMap : register(t0);
SamplerComparisonState ShadowSampler : register(s0);

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL;
    float3 WorldPos : TEXCOORD0;
    float4 LightSpacePos : TEXCOORD1;  // Phase 5: Position in light space
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
    
    // Phase 5: Transform to light space for shadow mapping
    output.LightSpacePos = mul(worldPos, LightView);
    output.LightSpacePos = mul(output.LightSpacePos, LightProj);
    
    // Transform normal to world space
    output.WorldNormal = mul(input.Normal, (float3x3)World);
    
    return output;
}

// Phase 5: 2x2 PCF shadow filtering
float SampleShadowMap_PCF(float4 lightSpacePos) {
    // Perspective divide
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    // Convert to texture coordinates [0, 1]
    float2 shadowTexCoord = float2(
        0.5 * projCoords.x + 0.5,
        -0.5 * projCoords.y + 0.5
    );
    
    // Clamp to shadow map bounds
    shadowTexCoord = clamp(shadowTexCoord, float2(0.0, 0.0), float2(1.0, 1.0));
    
    // Depth from light's perspective
    float depth = projCoords.z - ShadowBias;
    
    // 2x2 PCF: Sample 4 neighbors
    float shadowFactor = 0.0;
    float2 texelSize = ShadowMap_TexelSize;
    
    // Sample corners of 2x2 neighborhood
    shadowFactor += ShadowMap.SampleCmp(ShadowSampler, shadowTexCoord + float2(-texelSize.x, -texelSize.y), depth);
    shadowFactor += ShadowMap.SampleCmp(ShadowSampler, shadowTexCoord + float2( texelSize.x, -texelSize.y), depth);
    shadowFactor += ShadowMap.SampleCmp(ShadowSampler, shadowTexCoord + float2(-texelSize.x,  texelSize.y), depth);
    shadowFactor += ShadowMap.SampleCmp(ShadowSampler, shadowTexCoord + float2( texelSize.x,  texelSize.y), depth);
    
    // Average the 4 samples
    return shadowFactor * 0.25;
}

// Pixel Shader - Multi-light Phong shading with shadow mapping
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Normalize vectors
    float3 normal = normalize(input.WorldNormal);
    float3 viewDir = normalize(CameraPos - input.WorldPos);
    
    // Start with ambient lighting
    float3 finalColor = AmbientColor;
    
    // Phase 5: Sample shadow factor
    float shadowFactor = 1.0;
    if (UseShadows) {
        shadowFactor = SampleShadowMap_PCF(input.LightSpacePos);
    }
    
    // Accumulate lighting from all active lights
    for (int i = 0; i < NumLights && i < 8; ++i) {
        float3 lightDir = normalize(Lights[i].Position - input.WorldPos);
        float distance  = length(Lights[i].Position - input.WorldPos);
        
        // Spot light cone check
        if (Lights[i].Type == 1) {  // Spot light
            float spotFactor = dot(-lightDir, normalize(Lights[i].Direction));
            float spotCutoff = cos(radians(Lights[i].SpotAngle));
            if (spotFactor < spotCutoff) {
                continue;  // Outside spot cone
            }
        }
        
        // Distance attenuation (inverse square)
        float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);
        
        // Diffuse lighting (Lambertian)
        float ndotl = max(dot(normal, lightDir), 0.0);
        
        // Specular lighting (Blinn-Phong)
        float3 halfVec = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfVec), 0.0), 32.0);
        
        // Accumulate diffuse and specular
        // Phase 5: Apply shadow factor to lighting
        float3 diffuse = Lights[i].Color * Lights[i].Intensity * ndotl * shadowFactor;
        float3 specular = Lights[i].Color * Lights[i].Intensity * spec * 0.5 * shadowFactor;
        
        finalColor += (diffuse + specular) * attenuation;
    }
    
    return float4(finalColor, 1.0);
}
