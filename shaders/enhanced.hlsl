// enhanced.hlsl - Enhanced vertex and pixel shader with music-reactive effects
// Beat-reactive lighting, rim lighting, and dynamic coloring

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    float3 LightDir;
    float BeatIntensity;      // 0-1, pulses with music beats
};

cbuffer MaterialBuffer : register(b1) {
    float3 BaseColor;
    float Metallic;
    float3 EmissiveColor;
    float Roughness;
    float3 CameraPosition;
    float BPM;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL;
    float3 WorldPos : TEXCOORD0;
    float3 ViewDir : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
};

Texture2D SceneTexture : register(t1);
SamplerState SceneSampler : register(s1);

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
    
    // Calculate view direction for specular
    output.ViewDir = normalize(CameraPosition - worldPos.xyz);

    output.TexCoord = input.TexCoord;
    
    return output;
}

// PBR-inspired lighting model
float3 CalculateLighting(float3 normal, float3 lightDir, float3 viewDir, 
                         float3 baseColor, float metallic, float roughness) {
    // Diffuse component
    float ndotl = max(dot(normal, lightDir), 0.0);
    float3 diffuse = baseColor * ndotl;
    
    // Specular component (Blinn-Phong approximation)
    float3 halfVec = normalize(lightDir + viewDir);
    float ndoth = max(dot(normal, halfVec), 0.0);
    float shininess = lerp(256.0, 8.0, roughness);
    float specular = pow(ndoth, shininess);
    
    // Metallic workflow: metals have colored specular
    float3 specularColor = lerp(float3(1, 1, 1), baseColor, metallic);
    
    // Combine lighting
    return diffuse + specularColor * specular * 0.5;
}

// Rim lighting for edge glow
float CalculateRimLight(float3 normal, float3 viewDir, float rimPower) {
    float rim = 1.0 - saturate(dot(normal, viewDir));
    return pow(rim, rimPower);
}

// Music-reactive color pulsing
float3 ApplyBeatEffect(float3 color, float beatIntensity, float bpm) {
    // Pulse intensity based on BPM
    float beatFreq = bpm / 60.0;
    float pulse = beatIntensity * 0.5 + 0.5;
    
    // Add energy to color
    float3 energyColor = float3(1.0, 0.5, 0.2) * beatIntensity;
    return lerp(color, color + energyColor, pulse * 0.3);
}

// Pixel Shader - Enhanced lighting with music reactivity
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Normalize vectors
    float3 normal = normalize(input.WorldNormal);
    float3 lightDir = normalize(LightDir);
    float3 viewDir = normalize(input.ViewDir);
    
    float2 tiledUV = frac(input.TexCoord * 3.0);
    float3 albedo = SceneTexture.Sample(SceneSampler, tiledUV).rgb;

    // Calculate PBR-inspired lighting
    float3 litColor = CalculateLighting(normal, lightDir, viewDir,
                                        BaseColor * albedo, Metallic, Roughness);
    
    // Add rim lighting (edge glow)
    float rim = CalculateRimLight(normal, viewDir, 3.0);
    float3 rimColor = float3(0.3, 0.6, 1.0) * rim * BeatIntensity;
    
    // Apply beat-reactive effects
    float3 finalColor = ApplyBeatEffect(litColor, BeatIntensity, BPM);
    
    // Add emissive contribution
    float emissivePulse = 0.3 + (0.7 * BeatIntensity);
    finalColor += EmissiveColor * emissivePulse;
    
    // Add rim lighting
    finalColor += rimColor;
    
    // Ambient contribution
    float3 ambient = (BaseColor * albedo) * 0.22;
    finalColor += ambient;
    
    // HDR tone mapping (simple Reinhard)
    finalColor = finalColor / (finalColor + float3(1, 1, 1));
    
    // Gamma correction
    finalColor = pow(finalColor, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));
    
    return float4(finalColor, 1.0);
}
