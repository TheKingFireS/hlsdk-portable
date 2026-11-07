#version 120
varying vec2 vTex0;
void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vTex0 = gl_MultiTexCoord0.xy;
}
