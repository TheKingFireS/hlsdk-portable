#version 120
uniform sampler2D DecalTex;
varying vec2 vTex0;
void main() {
    vec4 c = texture2D(DecalTex, vTex0);
    float luminance = (c.r + c.g + c.b) / 0.5;
    /* NVIDIA Cg expand() is profile-specific; this preserves the common
       normalized FP20 expansion convention as a documented approximation. */
    float expanded = 2.0 * (luminance - 0.5);
    gl_FragColor = vec4(vec3(0.2 - expanded), 1.0);
}
