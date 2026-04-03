#version 430 core

// Bloom effect: Gaussian blur + threshold
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D sceneTex;

uniform vec2 texelSize;
uniform float threshold;
uniform float intensity;

out vec4 FragColor;

void main() {
    // Simple bloom: apply Gaussian blur to bright areas
    // First, check if this texel is bright (above threshold)
    vec4 center = texture(sceneTex, vTexCoord);
    float brightness = dot(center.rgb, vec3(0.299, 0.587, 0.114));
    
    if (brightness < threshold) {
        FragColor = vec4(0.0);
        return;
    }
    
    // Apply Gaussian blur to bright areas
    vec4 result = vec4(0.0);
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    
    result += texture(sceneTex, vTexCoord) * weights[0];
    for (int i = 1; i < 5; i++) {
        result += texture(sceneTex, vTexCoord + vec2(texelSize.x * i, 0.0)) * weights[i];
        result += texture(sceneTex, vTexCoord - vec2(texelSize.x * i, 0.0)) * weights[i];
    }
    
    FragColor = result * intensity;
}
