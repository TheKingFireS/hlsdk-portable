#version 120
varying vec2 vTex0;
varying vec2 vTex1;
varying vec2 vTex2;
varying vec2 vTex3;
void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vTex0 = gl_MultiTexCoord0.xy;
    vTex1 = gl_MultiTexCoord1.xy;
    vTex2 = gl_MultiTexCoord2.xy;
    vTex3 = gl_MultiTexCoord3.xy;
}
