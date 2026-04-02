#version 430 core

// Vertex attributes
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// Uniforms
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// Outputs to fragment shader
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vFragPos;

void main() {
    // Transform position to clip space
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
    
    // Transform normal to world space (for lighting)
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    
    // Pass through texture coordinates
    vTexCoord = aTexCoord;
    
    // Fragment position in world space (for lighting)
    vFragPos = vec3(uModel * vec4(aPosition, 1.0));
}
