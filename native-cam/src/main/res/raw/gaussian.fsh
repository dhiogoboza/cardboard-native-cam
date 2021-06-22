#extension GL_OES_EGL_image_external:require
precision mediump float;

uniform samplerExternalOES iChannel0;
varying vec2 texCoord;

void mainImage( out vec4 fragColor, in vec2 fragCoord) {
    vec2 resolution = vec2(1280.0f, 720.0f);
    float Pi = 6.28318530718; // Pi*2
    // GAUSSIAN BLUR SETTINGS {{{
    float Directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality = 3.0; // BLUR QUALITY (Default 4.0 - More is better but slower)
    float Size = 8.0; // BLUR SIZE (Radius)
    // GAUSSIAN BLUR SETTINGS }}}
    vec2 Radius = Size/resolution;
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/resolution;
    // Pixel colour
    vec4 Color = texture2D(iChannel0, uv);
    // Blur calculations
    for( float d=0.0; d<Pi; d+=Pi/Directions)
    {
        for(float i=1.0/Quality; i<=1.0; i+=1.0/Quality)
        {
            Color += texture2D( iChannel0, uv+vec2(cos(d),sin(d))*Radius*i);
        }
    }
    // Output to screen
    Color /= Quality * Directions - 15.0;
    fragColor =  Color;
}

void main() {
    mainImage(gl_FragColor, texCoord*(1280.0f, 720.0f));
}