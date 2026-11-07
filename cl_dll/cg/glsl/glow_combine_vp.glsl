#version 120
varying vec2 vTex0;
varying vec2 vTex1;
void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
    vTex0 = gl_MultiTexCoord0.xy * 2.0;
    vTex1 = gl_MultiTexCoord0.xy;
}
