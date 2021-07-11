#extension GL_OES_EGL_image_external:require

precision highp float;

uniform vec3 iResolution;
uniform samplerExternalOES iChannel0;
varying vec2 texCoord;

void mainImage( out vec4 fragColor, in vec2 fragCoord) {
    vec4 color = texture2D(iChannel0, fragCoord);
    float newR = abs(color.r + color.g * 2.0 - color.b) * color.r;
    float newG = abs(color.r + color.b * 2.0 - color.g) * color.r;
    float newB = abs(color.r + color.b * 2.0 - color.g) * color.g;
    vec4 newColor = vec4(newR, newG, newB, 1.0);
    fragColor = newColor;

    // 10
    // vec4 mask = texture2D(iChannel0, fragCoord);

    // float newR = float(int(mask.r * 10.0)) / 10.0;
    // float newG = float(int(mask.g * 10.0)) / 10.0;
    // float newB = float(int(mask.b * 10.0)) / 10.0;

    // vec4 tempColor = vec4(newR, newG, newB, 1.0);
    // fragColor = tempColor;
}

void main() {
    mainImage(gl_FragColor, texCoord);
}
