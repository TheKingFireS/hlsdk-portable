#version 120
varying vec2 vTex0;
void main() {
    vec4 c = texture2D(DecalTex, vTex0);
    float expanded = 2.0 * ((c.r + c.g + c.b) / 0.5 - 0.5);
    gl_FragColor = vec4(vec3(0.2 - expanded), 1.0);
}
