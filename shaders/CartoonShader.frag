//FRAGMENT SHADER
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
    vec3 lightPos;
    vec4 lightColor;
    vec3 eyePos;
    vec4 eyeDir;
} gubo;

layout(set = 0, binding = 1) uniform sampler2D tex;

void main(){
    vec3 norm = normalize(fragNorm);
    vec3 lightDir = normalize(gubo.lightDir);

    float diff = max(dot(norm, lightDir), 0.0f);

    float quantLevels = 4.0f;
    diff = floor(diff * quantLevels) / quantLevels;

    vec3 albedo = texture(tex, fragUV.xy).rgb;

    vec3 color = albedo * gubo.lightColor.rgb * diff;

    float edgeFactor = dot(norm, vec3(0.0f, 0.0f, 1.0f));
    edgeFactor = step(0.1f, edgeFactor);

    color = mix(vec3(0.0f), color, edgeFactor);

    outColor = vec4(color, 1.0f);

}