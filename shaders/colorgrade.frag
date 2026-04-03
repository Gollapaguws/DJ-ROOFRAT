#version 430 core

// Color grading effect: exposure, gamma, saturation adjustments
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D sceneTex;

uniform float exposure;
uniform float gamma;
uniform float saturation;

out vec4 FragColor;

// Mood-based color grading (0=neutral, 1=calm/blue, 2=grooving/green, 3=hyped/red)
uniform int mood;

void main() {
    vec4 color = texture(sceneTex, vTexCoord);
    
    // Apply exposure adjustment
    color.rgb *= pow(2.0, exposure);
    
    // Apply mood-based color tint
    if (mood == 1) {
        // Calm: blue tint
        color.rgb *= vec3(0.8, 0.9, 1.2);
    } else if (mood == 2) {
        // Grooving: green tint
        color.rgb *= vec3(0.9, 1.1, 0.9);
    } else if (mood == 3) {
        // Hyped: red tint
        color.rgb *= vec3(1.2, 0.8, 0.8);
    }
    
    // Apply saturation adjustment
    vec3 gray = vec3(dot(color.rgb, vec3(0.299, 0.587, 0.114)));
    color.rgb = mix(gray, color.rgb, saturation);
    
    // Apply gamma correction
    color.rgb = pow(color.rgb, vec3(1.0 / gamma));
    
    // Clamp result to [0, 1]
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    
    FragColor = color;
}
