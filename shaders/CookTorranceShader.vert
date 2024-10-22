// VERTEX SHADER
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
} ubo;

// values taken from previous pipeline stage
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;

// values passed to fragment shader
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragUV;

void main() {
    // compute clipping coordinates
    gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);

    fragPos = (ubo.mMat * vec4(inPosition, 1.0)).xyz;
    fragNorm = mat3(ubo.nMat) * inNorm;
    fragUV = inUV;
}