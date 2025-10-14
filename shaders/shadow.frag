#version 460 core
in vec2 vUV;


uniform sampler2D uTex0;
uniform int  uHasDiffuse;  
uniform vec2 uUVScale;     
uniform vec2 uUVOffset;    

void main() {
        if (uHasDiffuse != 0) {
        vec2 uv = vUV * uUVScale + uUVOffset;
        if (texture(uTex0, uv).a < 0.3) discard;       }
    // no color outputs — depth only
}
