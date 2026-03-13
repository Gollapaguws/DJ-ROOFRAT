// crowd.hlsl - Instanced crowd rendering shader for Phase 20
// Unpacks per-instance data, applies BPM-synced animation, basic lighting

cbuffer ConstantBuffer : register(b0) {
    matrix View;
    matrix Projection;
    float3 LightDir;
    float Time;
};

// Per-vertex data from CrowdMesh
struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;

    // Per-instance data from CrowdAnimator::InstanceData
    float3 InstPosition     : INST_POSITION;
    float  InstPadding1     : INST_PAD1;
    float3 InstColor        : INST_COLOR;
    int    InstAnimationType : INST_ANIMTYPE;
    float  InstAnimState    : INST_ANIMSTATE;
    float  InstLODLevel     : INST_LOD;
    float2 InstPadding2     : INST_PAD2;
};

struct PS_INPUT {
    float4 Position    : SV_POSITION;
    float3 WorldNormal : NORMAL;
    float3 Color       : COLOR;
};

// Apply animation displacement based on type and state
float3 applyAnimation(float3 localPos, int animType, float animState) {
    float3 offset = float3(0, 0, 0);

    if (animType == 1) {
        // Jump: vertical bounce using sine curve
        offset.y = sin(animState * 3.14159) * 0.5;
    } else if (animType == 2) {
        // Wave: lateral arm sway (applied to upper vertices)
        if (localPos.y > 0.5) {
            offset.x = sin(animState * 6.28318) * 0.3;
        }
    } else if (animType == 3) {
        // Dance: body sway side-to-side
        offset.x = sin(animState * 6.28318) * 0.15;
        offset.y = abs(sin(animState * 6.28318 * 2.0)) * 0.1;
    }
    // animType 0 (Idle): no displacement

    return localPos + offset;
}

// Vertex Shader
PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;

    // Apply animation to local position
    float3 animated = applyAnimation(input.Position, input.InstAnimationType, input.InstAnimState);

    // World position = instance position + animated local offset
    float4 worldPos = float4(animated + input.InstPosition, 1.0);

    // View and projection transforms
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);

    // Normal stays in local space (no instance rotation)
    output.WorldNormal = input.Normal;

    // Pass instance color
    output.Color = input.InstColor;

    return output;
}

// Pixel Shader - Simple Phong with instance color
float4 PSMain(PS_INPUT input) : SV_TARGET {
    float3 normal = normalize(input.WorldNormal);
    float3 lightDir = normalize(LightDir);

    float ndotl = max(dot(normal, lightDir), 0.0);

    float3 ambient = input.Color * 0.25;
    float3 diffuse = input.Color * ndotl * 0.75;

    return float4(ambient + diffuse, 1.0);
}
