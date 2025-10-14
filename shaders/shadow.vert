#version 460 core
layout(location=0) in vec3 aPos;
layout(location=2) in vec2 aUV;   // pass UVs for alpha cutout

uniform mat4 uLightVP;
uniform mat4 model;

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = uLightVP * model * vec4(aPos, 1.0);
}
