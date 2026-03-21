// tunnel.hlsl - Procedural tunnel geometry shader with scrolling UVs and beat-based distortion
// Music-reactive effect: scrolls texture and distorts geometry based on beat intensity

cbuffer TunnelBuffer : register(b0) {
    float4x4 WorldViewProj;
    float ScrollOffset;     // UV scroll parameter for animated texture
    float BeatIntensity;    // 0-1 range for radial distortion amplitude
    float2 Padding1;        // Padding for 16-byte alignment
};

cbuffer MaterialBuffer : register(b1) {
    float3 BaseColor;
    float Metallic;
    float3 EmissiveColor;
    float Roughness;
    float3 CameraPosition;
    float BPM;
};

// Texture for tunnel surface
Texture2D TunnelTexture : register(t0);
SamplerState TexSampler : register(s0);

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL;
    float3 WorldPos : TEXCOORD0;
    float2 ScrolledTexCoord : TEXCOORD1;
    float3 ViewDir : TEXCOORD2;
};

// Vertex Shader
PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    
    // Apply beat-based radial distortion to position
    float3 distortedPos = input.Position;
    
    // For vertically-aligned tunnel, distort in XY plane
    float2 radial = float2(distortedPos.x, distortedPos.y);
    float radius = length(radial);
    float theta = atan2(radial.y, radial.x);
    
    // Apply sinusoidal distortion based on beat intensity
    float distortionFreq = 2.0f;
    float displacement = BeatIntensity * sin(theta * distortionFreq) * 0.5f;
    float distortedRadius = radius + displacement;
    
    // Recompute XY with distorted radius
    distortedPos.x = distortedRadius * cos(theta);
    distortedPos.y = distortedRadius * sin(theta);
    
    // Transform position to projection space
    output.Position = mul(float4(distortedPos, 1.0f), WorldViewProj);
    
    output.WorldPos = distortedPos;
    output.WorldNormal = input.Normal;
    
    // Apply scrolling to V texture coordinate (vertical scroll for tunnel)
    output.ScrolledTexCoord = input.TexCoord;
    output.ScrolledTexCoord.y += ScrollOffset;  // Scroll moves texture downward
    output.ScrolledTexCoord.y = fmod(output.ScrolledTexCoord.y, 1.0f);  // Wrap around
    
    // Calculate view direction
    output.ViewDir = normalize(CameraPosition - distortedPos);
    
    return output;
}

// Pixel Shader
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Sample texture with scrolled coordinates
    float4 texColor = TunnelTexture.Sample(TexSampler, input.ScrolledTexCoord);
    
    // Normal mapping / lighting
    float3 normal = normalize(input.WorldNormal);
    float3 viewDir = normalize(input.ViewDir);
    
    // Simple Lambertian lighting with beat intensity boost
    float3 lightDir = float3(0.0f, 1.0f, 0.0f);
    float light = max(dot(normal, lightDir), 0.0f);
    light += BeatIntensity * 0.5f;  // Brighten with beat
    
    // Combine texture with lighting
    float3 litColor = texColor.rgb * light;
    
    // Add emissive glow
    litColor += EmissiveColor * BeatIntensity * 0.3f;
    
    return float4(litColor, texColor.a);
}
