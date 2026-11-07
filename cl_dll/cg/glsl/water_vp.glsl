#version 120
varying vec2 vTex0;
varying vec2 vTex1;
varying vec2 vTex2;
varying vec2 vTex3;
uniform float flScreen;
void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vec4 uv = gl_MultiTexCoord0;
    vTex0 = uv.xy;
    vTex1 = uv.xy;
    vTex2 = uv.xy + vec2(0.0, flScreen);
    vTex3 = uv.xy;
}
