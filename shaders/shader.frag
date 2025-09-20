#version 460 core
in vec2 vUV;
layout(location=0) out vec4 FragColor;

uniform sampler2D uTex0;
uniform int  uHasDiffuse;   // set by mesh_draw()
uniform vec4 uTint;         // set by mesh_draw(), defaults to 1

void main()
{
    if (uHasDiffuse != 0) {
        vec4 base = texture(uTex0, vUV);

        if (base.a < 0.5) discard;   // tweak threshold if needed (0.3–0.6)

        FragColor = base * uTint;
    } else {
        FragColor = uTint;
    }
}
