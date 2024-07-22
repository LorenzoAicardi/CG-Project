// FRAGMENT SHADER
#version 450
#extension GL_ARB_separate_shader_objects : enable

// Shader input
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

// Shader output
layout(location = 0) out vec4 outColor;

// Uniforms
layout(set = 0, binding = 3) uniform GlobalUniformBufferObject {
    vec3 lightDir[3];
    vec3 lightPos[3];
    vec4 lightColor[3];
    vec3 eyePos;
    vec4 eyeDir;
    float cosIn;
    float cosOut;
    int spotlightOn;
} gubo;

// Textures
layout(set = 0, binding = 1) uniform sampler2D texDiffuse;
layout(set = 0, binding = 2) uniform sampler2D texRoughness;

#define PI 3.14159

// Coefficients for the spherical harmonics ambient light term
const vec3 C00  = vec3(.38f, .43f, .45f)/8.0f;
const vec3 C1m1 = vec3(.29f, .36f, .41f)/8.0f;
const vec3 C10  = vec3(.04f, .03f, .01f)/8.0f;
const vec3 C11  = vec3(-.10f, -.10f, -.09f)/8.0f;
const vec3 C2m2 = vec3(-.06f, -.06f, -.04f)/8.0f;
const vec3 C2m1 = vec3(.01f, -.01f, -.05f)/8.0f;
const vec3 C20  = vec3(-.09f, -.13f, -.15f)/8.0f;
const vec3 C21  = vec3(-.06f, -.05f, -.04f)/8.0f;
const vec3 C22  = vec3(.02f, .00f, -.05f)/8.0f;

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

vec3 spotLightDir(vec3 pos, int i) {
    return normalize(gubo.lightPos[i] - pos);
}

vec3 spotLightColor(vec3 pos, int i) {
    vec3 baseColor = pointLightColor(pos, i);
    vec3 lx = spotLightDir(pos, i);
    vec3 d = gubo.lightDir[i];
    return baseColor * clamp((dot(lx, d) - gubo.cosOut) / (gubo.cosIn - gubo.cosOut), 0.0, 1.0);
}

/**
 * Compute the g_ggx() value for the G term
 * of Cook-Torrance GGX
 */
float gGGX(float rho, vec3 n, vec3 a) {
    float na2 = pow(dot(n, a), 2.0);
    return 2 / (1 + sqrt(1 + (rho*rho*((1-na2)/na2))));
}

/**
 * Compute BRDF following Cook-Torrance model
 */
vec3 BRDF(vec3 albedo, float rho, vec3 norm, vec3 eyeDir, vec3 lightDir) {
    float F0 = 0.2;
    float k = 0.45;

    vec3 diffuse = albedo * max(dot(norm, lightDir), 0.0);

    vec3 h = normalize(lightDir + eyeDir);
    float D = (rho*rho) / (PI * pow(pow(max(dot(norm, h), 0.0), 2) * (rho*rho - 1) + 1, 2));
    float F = F0 + (1 - F0) * pow(1 - max(dot(eyeDir, h), 0.0), 5);
    float G = gGGX(rho, norm, eyeDir) * gGGX(rho, norm, lightDir);
    vec3 specular = albedo * vec3(D*F*G / 4*max(dot(eyeDir, norm), 0.0));

    return k*diffuse + (1-k)*specular;
}

void main() {
    vec3 norm = normalize(fragNorm);
    vec3 eyeDir = normalize(gubo.eyePos - fragPos);
    vec3 albedo = texture(texDiffuse, fragUV).rgb;
    float roughness = texture(texRoughness, fragUV).r;
    vec3 L = vec3(0);// solution of rendering equation

    // Lights
    vec3 lightDir = directLightDir(fragPos, 0);
    vec3 lightColor = directLightColor(fragPos, 0);
    L += BRDF(albedo, roughness, norm, eyeDir, lightDir) * lightColor * (1-gubo.spotlightOn);

    lightDir = pointLightDir(fragPos, 1);
    lightColor = pointLightColor(fragPos, 1);
    L += BRDF(albedo, roughness, norm, eyeDir, lightDir) * lightColor * (1-gubo.spotlightOn);

    lightDir = spotLightDir(fragPos, 2);
    lightColor = spotLightColor(fragPos, 2);
    L += BRDF(albedo, roughness, norm, eyeDir, lightDir) * lightColor * gubo.spotlightOn;

    // Ambient lighting
    vec3 La = C00 + (norm.x*C11) + (norm.y*C1m1) + (norm.z*C10) + (norm.x*norm.y*C2m2) +
    (norm.y*norm.z*C1m1) + (norm.z*norm.x*C11) + ((norm.x*norm.x - norm.y*norm.y) * C22) +
    ((3*norm.z*norm.z - 1) * C20);
    vec3 ambient = La * albedo;
    L += ambient;

    outColor = vec4(clamp(L, 0.0f, 1.0f), 1.0f);
}