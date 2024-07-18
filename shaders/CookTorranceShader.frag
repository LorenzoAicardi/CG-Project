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

#define PI 3.14159

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
    float beta = 1.0f;
    vec3 p = gubo.lightPos[i];
    return gubo.lightColor[i].rgb * pow(g / length(p-pos), beta);
}

/**
 * compute the g_ggx() value for the G term
 * of Cook-Torrance GGX
 */
float gGGX(float rho, vec3 n, vec3 a) {
    float na2 = pow(dot(n, a), 2.0);
    return 2 / (1 + sqrt(1 + (rho*rho*((1-na2)/na2))));
}

/**
 * Compute BRDF following Cook-Torrance model
 */
vec3 BRDF(vec3 albedo, vec3 norm, vec3 eyeDir, vec3 lightDir) {
    float rho = 0.5;
    float F0 = 0.8;
    float k = 0.6;

    vec3 diffuse = albedo * max(dot(norm, lightDir), 0.0);

    vec3 h = normalize(lightDir + eyeDir);
    float D = (rho*rho) / PI * pow(pow(max(dot(norm, h), 0.0), 2) * (rho*rho - 1) + 1, 2);
    float F = F0 + (1 - F0) * pow(1 - max(dot(eyeDir, h), 0.0), 5);
    float G = gGGX(rho, norm, eyeDir) * gGGX(rho, norm, lightDir);
    vec3 specular = vec3(D*F*G / 4*max(dot(eyeDir, norm), 0.0));

    return k*diffuse + (1-k)*specular;
}

void main() {
    vec3 norm = normalize(fragNorm);
    vec3 eyeDir = normalize(gubo.eyePos - fragPos);
    vec3 albedo = texture(tex, fragUV).rgb;
    vec3 L = vec3(0);// solution of rendering equation

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

    outColor = vec4(L, 1.0f);
}