// FRAGMENT SHADER
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
    vec3 lightDir[2];
    vec3 lightPos[2];
    vec4 lightColor[2];
    vec3 eyePos;
    vec4 eyeDir;
} gubo;

layout(set = 0, binding = 1) uniform sampler2D tex;

vec3 directLightDir(vec3 pos, int i) {
    return normalize(gubo.lightDir[i]);
}

vec3 directLightColor(vec3 pos, int i) {
    return gubo.lightColor[i].rgb;
}

vec3 pointLightDir(vec3 pos, int i) {
    return normalize(gubo.lightPos[i] - pos);
}

vec3 pointLightColor(vec3 pos, int i) {
    float g = gubo.lightColor[i].a;
    float beta = 2.0;
    vec3 p = gubo.lightPos[i];
    return gubo.lightColor[i].rgb * pow(g / length(p-pos), beta);
}

/**
 * Compute BRDF following Lambert + Blinn models
 */
vec3 BRDF(vec3 Albedo, vec3 Norm, vec3 EyeDir, vec3 lightDir) {
    float gamma = 200.0f;
    vec3 diffuse = Albedo * 0.975f * max(dot(Norm, lightDir), 0.0);
    vec3 specular = vec3(pow(max(dot(Norm, normalize(lightDir + EyeDir)), 0.0), gamma));

    return diffuse + specular;
}

void main() {
    vec3 norm = normalize(fragNorm);
    vec3 eyeDir = normalize(gubo.eyePos - fragPos);
    vec3 albedo = texture(tex, fragUV).rgb;
    vec3 L = vec3(0);// solution of rendering equation

    // self-emission
    L += albedo;

    // lights
    vec3 lightDir = directLightDir(fragPos, 0);
    vec3 lightColor = directLightColor(fragPos, 0);
    L += BRDF(albedo, norm, eyeDir, lightDir) * lightColor;

    lightDir = pointLightDir(fragPos, 1);
    lightColor = pointLightColor(fragPos, 1);
    L += BRDF(albedo, norm, eyeDir, lightDir) * lightColor;

    // ambient lighting
    vec3 ambient = albedo * 0.025f;
    L += ambient;

    outColor = vec4(fragUV, 0.0f, 1.0f);
}