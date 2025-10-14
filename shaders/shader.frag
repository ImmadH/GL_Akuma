#version 460 core

in vec2 vUV;
in vec3 vNormal;
in vec3 vFragPos;
in vec4 vPosLight;

layout(location=0) out vec4 FragColor;

uniform sampler2D uTex0;
uniform int   uHasDiffuse;      // set by model_draw()
uniform vec4  uTint;            // set by model_draw()

uniform vec3  uLightDir;       
uniform vec3  uLightColor;
uniform vec3  uViewPos;
uniform float uShininess;

uniform vec2 uUVScale;   
uniform vec2 uUVOffset;  

uniform int   uUseEmissive;
uniform vec3  uEmissiveColor;

#define MAX_POINT_LIGHTS 16
uniform int   uPLCount;
uniform vec3  uPLPos[MAX_POINT_LIGHTS];
uniform vec3  uPLColor[MAX_POINT_LIGHTS];
uniform float uPLIntensity[MAX_POINT_LIGHTS];
uniform float uPLRange[MAX_POINT_LIGHTS];

uniform sampler2D uShadowMap;

float calcShadow(vec3 normal, vec3 lightDirWS, vec4 posLightCS)
{
    vec3 ndcPos      = posLightCS.xyz / posLightCS.w;
    vec2 shadowUV    = ndcPos.xy * 0.5 + 0.5;
    float shadowDepth = ndcPos.z * 0.5 + 0.5;

    if (shadowUV.x < 0.0 || shadowUV.x > 1.0 || shadowUV.y < 0.0 || shadowUV.y > 1.0)
        return 1.0;

    float cosNL      = max(dot(normalize(normal), normalize(-lightDirWS)), 0.0);
    float shadowBias = max(0.0005 * (1.0 - cosNL), 0.0001);

    float visibility = 0.0;
    vec2 texelSize   = 1.0 / textureSize(uShadowMap, 0);
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x) {
        float sampledDepth = texture(uShadowMap, shadowUV + vec2(x, y) * texelSize).r;
        visibility += (shadowDepth - shadowBias) <= sampledDepth ? 1.0 : 0.0;
    }
    return visibility / 9.0;
}

void main()
{
    // Emissive override
    if (uUseEmissive == 1) {
        vec3 emissive = uEmissiveColor;
        FragColor = vec4(pow(emissive, vec3(1.0 / 2.2)), 1.0);
        return;
    }

    vec2 baseUV   = vUV * uUVScale + uUVOffset;
    vec4 baseSample = (uHasDiffuse != 0) ? texture(uTex0, baseUV) : vec4(1.0);
    if (baseSample.a < 0.3) discard;
    vec3 baseColor = pow(baseSample.rgb, vec3(2.2)) * uTint.rgb;

    // Unlit path
    if (uShininess == 0.0) {
        FragColor = vec4(pow(baseColor, vec3(1.0 / 2.2)), 1.0);
        return;
    }

    // Night path 
    if (uShininess < 0.0) {
        vec3 normal = normalize(vNormal);
        if (!gl_FrontFacing) normal = -normal;
        vec3 viewDir = normalize(uViewPos - vFragPos);

        const vec3 skyColor   = vec3(0.05, 0.07, 0.10);
        const vec3 groundColor= vec3(0.01, 0.01, 0.01);
        float upFactor = clamp(normal.y, 0.0, 1.0);
        vec3 ambient = mix(groundColor, skyColor, upFactor) * baseColor * 0.6;

        const vec3 fresnel0 = vec3(0.04);
        float NoV = max(dot(normal, viewDir), 0.0);
        vec3  fresnelView = fresnel0 + (1.0 - fresnel0) * pow(1.0 - NoV, 5.0);

        vec3 diffuse  = vec3(0.0);
        vec3 specular = vec3(0.0);

        int pointCount = min(uPLCount, MAX_POINT_LIGHTS);
        for (int i = 0; i < pointCount; ++i) {
            vec3 toLight   = uPLPos[i] - vFragPos;
            float distance = length(toLight);
            float range    = max(uPLRange[i], 1e-3);
            float normDist = clamp(distance / range, 0.0, 1.0);
            float attenuation = (1.0 - normDist) * (1.0 - normDist);

            vec3 lightDir = (distance > 0.0) ? (toLight / distance) : vec3(0, 1, 0);
            float NoL = max(dot(normal, lightDir), 0.0);

            diffuse += NoL * baseColor * uPLColor[i] * (uPLIntensity[i] * attenuation);

            float shininessNight = 8.0;
            vec3 halfVec = normalize(lightDir + viewDir);
            float NoH = max(dot(normal, halfVec), 0.0);
            float specTerm = pow(NoH + 1e-4, shininessNight);
            specular += specTerm * fresnelView * uPLColor[i] * (uPLIntensity[i] * attenuation);
        }

        vec3 finalColor = ambient + diffuse + specular;
        FragColor = vec4(pow(finalColor, vec3(1.0 / 2.2)), 1.0);
        return;
    }

    // Regular lighting directional + point lights
    vec3 normal = normalize(vNormal);
    if (!gl_FrontFacing) normal = -normal; // twosided for leaves
    vec3 viewDir = normalize(uViewPos - vFragPos);

    const vec3 skyColor   = vec3(0.30, 0.35, 0.45);
    const vec3 groundColor= vec3(0.02, 0.02, 0.02);
    float upFactor = clamp(normal.y, 0.0, 1.0);
    vec3 ambient = mix(groundColor, skyColor, upFactor) * baseColor;

    vec3 lightDir = normalize(-uLightDir);
    float NoL_dir = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = NoL_dir * baseColor * uLightColor;

    vec3 halfDir = normalize(lightDir + viewDir);
    float NoH_dir = max(dot(normal, halfDir), 0.0);
    float specTermDir = pow(NoH_dir, uShininess);

    const vec3 fresnel0 = vec3(0.04);
    float NoV = max(dot(normal, viewDir), 0.0);
    vec3  fresnel = fresnel0 + (1.0 - fresnel0) * pow(1.0 - NoV, 5.0);

    vec3 specular = specTermDir * fresnel * uLightColor;

    float shadowVisibility = calcShadow(normal, lightDir, vPosLight);

    // Point lights
    int pointCount2 = min(uPLCount, MAX_POINT_LIGHTS);
    for (int i = 0; i < pointCount2; ++i) {
        vec3 toLight   = uPLPos[i] - vFragPos;
        float distance = length(toLight);
        float range    = max(uPLRange[i], 1e-3);
        float normDist = clamp(distance / range, 0.0, 1.0);
        float attenuation = (1.0 - normDist) * (1.0 - normDist);

        vec3 lightDirPL = (distance > 0.0) ? (toLight / distance) : vec3(0, 1, 0);
        float NoL = max(dot(normal, lightDirPL), 0.0);

        diffuse += NoL * baseColor * uPLColor[i] * (uPLIntensity[i] * attenuation);

        vec3 halfVec = normalize(lightDirPL + viewDir);
        float NoH = max(dot(normal, halfVec), 0.0);
        float specTerm = (NoH > 0.0) ? pow(NoH, uShininess) : 0.0;
        specular += specTerm * fresnel * uPLColor[i] * (uPLIntensity[i] * attenuation);
    }

    diffuse  *= shadowVisibility;
    specular *= shadowVisibility;

    vec3 finalColor = ambient + diffuse + specular;
    FragColor = vec4(pow(finalColor, vec3(1.0 / 2.2)), 1.0);
}

