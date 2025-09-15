#version 460 core
in vec2 vUV;
layout(location=0) out vec4 FragColor;

uniform sampler2D uTex0;
uniform int  uHasDiffuse;   // set by mesh_draw()
uniform vec4 uTint;         // set by mesh_draw(), defaults to 1

void main()
{
    vec4 base = (uHasDiffuse != 0) ? texture(uTex0, vUV) : vec4(1.0);
    FragColor = base * uTint;
}
