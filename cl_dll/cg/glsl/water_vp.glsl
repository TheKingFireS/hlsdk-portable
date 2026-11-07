#version 120
precision mediump float;

uniform mat4 ModelViewProj;
uniform float flScreen;

varying vec4 vTexCoord0;
varying vec4 vTexCoord1;
varying vec4 vTexCoord2;
varying vec4 vTexCoord3;

void main()
{
    gl_Position = ModelViewProj * gl_Vertex;
    vTexCoord0 = gl_MultiTexCoord0;
    vTexCoord1 = gl_MultiTexCoord0;
    vTexCoord2 = gl_MultiTexCoord0;
    vTexCoord2.y = gl_MultiTexCoord0.y + flScreen;
    vTexCoord3 = gl_MultiTexCoord0;
}
