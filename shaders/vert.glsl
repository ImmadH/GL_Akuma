#version 460 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 model;

out vec2 vUV;

void main()
{
    vUV = aUV;
    gl_Position = uProjection * uView * model * vec4(aPos, 1.0);
}

