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
    vec3 lightDir;
    vec4 lightColor;
    vec3 lightPos;
    vec3 eyePos;
} gubo;

layout(set = 0, binding = 1) uniform sampler2D tex;

vec3 pointLightDir(vec3 pos) {
    // Point light - direction vector
    // Position of the light in <gubo.lightPos>
    return normalize(gubo.lightPos - pos);
}

vec3 pointLightColor(vec3 pos) {
    // Point light - color
    // Color of the light in <gubo.lightColor.rgb>
    // Scaling factor g in <gubo.lightColor.a>
    // Decay power beta: constant and fixed to 2.0
    // Position of the light in <gubo.lightPos>
    float g = gubo.lightColor.a;
    float beta = 2.0;
    vec3 p = gubo.lightPos;
    return gubo.lightColor.rgb * pow(g/length(p-pos), beta);
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

    vec3 lightDir = pointLightDir(fragPos);
    vec3 lightColor = pointLightColor(fragPos);

    L += BRDF(albedo, norm, eyeDir, lightDir) * lightColor;

    vec3 ambient = albedo * 0.025f;
    L += ambient;

    outColor = vec4(L, 1.0f);
}