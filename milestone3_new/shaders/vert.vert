#version 450

// Inputs from vertex buffer (must match Vertex::getAttributeDescriptions())
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// Output to fragment shader
layout(location = 0) out vec3 fragColor;

// UBO
layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main(){
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos, 1.0);
    fragColor = inNormal * 0.5 + 0.5;
}