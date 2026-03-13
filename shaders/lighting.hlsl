// lighting.hlsl - Multi-light Phong shading for Phase 19
// Supports up to 8 lights (spot and point) with per-light colors

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

// Pixel Shader - Multi-light Phong shading
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Normalize vectors
    float3 normal = normalize(input.WorldNormal);
    float3 viewDir = normalize(CameraPos - input.WorldPos);
    
    // Start with ambient lighting
    float3 finalColor = AmbientColor;
    
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
        float3 diffuse = Lights[i].Color * Lights[i].Intensity * ndotl;
        float3 specular = Lights[i].Color * Lights[i].Intensity * spec * 0.5;
        
        finalColor += (diffuse + specular) * attenuation;
    }
    
    return float4(finalColor, 1.0);
}
