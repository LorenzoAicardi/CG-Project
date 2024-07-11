#version 450
#extension GL_ARB_separate_shader_objects : enable

// stuff coming from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

// shader output
layout(location = 0) out vec4 outColor;

// uniforms
layout(set = 0, binding = 2) uniform GlobalUniformBufferObject {
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
} gubo;

layout(set = 0, binding = 1) uniform sampler2D tex;

void main() {
    float gamma = 200.0f;

    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos - fragPos);

    vec3 lightDir = normalize(gubo.lightDir);
    vec3 lightColor = gubo.lightColor.rgb;

    vec3 diffuse = texture(tex, fragUV).rgb * 0.975f * max(dot(Norm, lightDir), 0.0);
    vec3 specular = vec3(pow(max(dot(Norm, normalize(lightDir + EyeDir)), 0.0), gamma));
    vec3 ambient = texture(tex, fragUV).rgb * 0.025f;

    vec3 col  = (diffuse + specular) * lightColor + ambient;

    outColor = vec4(col, 1.0f);
}