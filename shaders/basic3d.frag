#version 430 core

// Inputs from vertex shader
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;

// Uniforms
uniform sampler2D uTexture;
uniform vec3 uLightDir;
uniform vec3 uCameraPos;

// Output
out vec4 FragColor;

void main() {
    // Normalize the interpolated normal
    vec3 normal = normalize(vNormal);
    
    // Simple directional light (default: from top-right-back)
    vec3 lightDir = normalize(uLightDir);
    
    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.2);  // Min 0.2 for ambient
    
    // Sample texture
    vec4 texColor = texture(uTexture, vTexCoord);
    
    // Apply lighting to texture
    vec3 finalColor = texColor.rgb * diffuse;
    
    // Output final color
    FragColor = vec4(finalColor, texColor.a);
}
