#version 120
precision mediump float;

uniform mat4 ModelViewProj;

varying vec2 vTex0;

void main()
{
    gl_Position = ModelViewProj * gl_Vertex;
    vTex0 = gl_MultiTexCoord0.xy;
}
