// shadowdepth.hlsl - Depth-only vertex and pixel shaders for shadow mapping
// Renders scene to depth texture using light-space transforms

cbuffer TransformBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float Depth : TEXCOORD0;
};

// Vertex Shader - Transform to light space
PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    
    // Transform position to world space
    float4 worldPos = mul(float4(input.Position, 1.0), World);
    
    // Transform to light view space
    float4 viewPos = mul(worldPos, View);
    
    // Transform to light projection space
    output.Position = mul(viewPos, Projection);
    
    // Pass depth for pixel shader (linear depth in [0, 1])
    output.Depth = output.Position.z / output.Position.w;
    
    return output;
}

// Pixel Shader - Output depth
float4 PSMain(PS_INPUT input) : SV_TARGET {
    // Return depth as grayscale for visualization/debugging
    // In actual shadow mapping, this would be stored directly in the depth buffer
    // via the depth stencil view in the render target, not through the pixel shader
    return float4(input.Depth, input.Depth, input.Depth, 1.0);
}
