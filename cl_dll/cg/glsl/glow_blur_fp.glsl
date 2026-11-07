#version 120
#extension GL_ARB_texture_rectangle : enable
varying vec2 vTex0;
varying vec2 vTex1;
varying vec2 vTex2;
varying vec2 vTex3;
uniform sampler2DRect tex0;
uniform sampler2DRect tex1;
uniform sampler2DRect tex2;
uniform sampler2DRect tex3;
void main() {
    vec4 c0 = texture2DRect(tex0, vTex0) * 0.5;
    vec4 c1 = texture2DRect(tex1, vTex1) * 0.5;
    vec4 c2 = texture2DRect(tex2, vTex2) * 0.5;
    vec4 c3 = texture2DRect(tex3, vTex3) * 0.5;
    gl_FragColor = (c0 + c1 + c2 + c3) * 0.5;
}
