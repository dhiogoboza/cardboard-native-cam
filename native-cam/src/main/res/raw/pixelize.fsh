#extension GL_OES_EGL_image_external:require

precision mediump float;
uniform vec3 iResolution;
uniform samplerExternalOES iChannel0;
varying vec2 texCoord;

// The cell size.
#define S (1280.0f / 6e1)

void mainImage(out vec4 c, vec2 p) {
    c = texture2D(iChannel0, floor((p + .5) / S) * S / (1280.0f, 720.0f));
}

void main() {
    mainImage(gl_FragColor, texCoord*(1280.0f, 720.0f));
}