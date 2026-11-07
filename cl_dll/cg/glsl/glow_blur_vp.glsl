#version 120
varying vec2 vTex0;
varying vec2 vTex1;
varying vec2 vTex2;
varying vec2 vTex3;
uniform float XOffset;
uniform float YOffset;
void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vec2 uv = gl_MultiTexCoord0.xy;
    vTex0 = uv + vec2(XOffset, YOffset);
    vTex1 = uv + vec2(2.0 * XOffset, 2.0 * YOffset);
    vTex2 = uv - vec2(XOffset, YOffset);
    vTex3 = uv - vec2(2.0 * XOffset, 2.0 * YOffset);
}
