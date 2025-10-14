#version 460 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 model;
uniform mat4 uLightVP;

out vec2 vUV;
out vec3 vNormal;
out vec3 vFragPos;
out vec4 vPosLight;

void main()
{
    vUV = aUV;

    vec4 posWS = model * vec4(aPos, 1.0);
    vFragPos   = posWS.xyz;

    vNormal = normalize(mat3(model) * aNormal);

    // light-space position for shadow mapping
    vPosLight = uLightVP * posWS;

    gl_Position = uProjection * uView * posWS;
}
