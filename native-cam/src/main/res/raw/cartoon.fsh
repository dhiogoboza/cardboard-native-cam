#extension GL_OES_EGL_image_external:require

precision highp float;

uniform vec3 iResolution;
uniform samplerExternalOES iChannel0;
varying vec2 texCoord;

//void gaussian() {
//     // get tex coordinates
//     vec2 texc = vec2(((vProjectedCoords.x / vProjectedCoords.w) + 1.0 ) / 2.0,
//     ((vProjectedCoords.y / vProjectedCoords.w) + 1.0 ) / 2.0 );
//
//     // compute the U & V step needed to read neighbor pixels
//     // for that you need to pass the texture dimensions to the shader,
//     // so let's say those are texWidth and texHeight
//     float step_u = 1.0 / texWidth;
//     float step_v = 1.0 / texHeight;
//
//     // read current pixel
//     vec4 centerPixel = texture2D(uTextureFilled, texc);
//
//     // read nearest right pixel & nearest bottom pixel
//     vec4 rightPixel  = texture2D(uTextureFilled, texc + vec2(step_u, 0.0));
//     vec4 bottomPixel = texture2D(uTextureFilled, texc + vec2(0.0, step_v));
//
//     // now manually compute the derivatives
//     float _dFdX = length(rightPixel - centerPixel) / step_u;
//     float _dFdY = length(bottomPixel - centerPixel) / step_v;
//}

void mainImage( out vec4 fragColor, in vec2 fragCoord) {
//    vec4 color = texture2D(iChannel0, fragCoord);
//    float newR = abs(color.r + color.g * 2.0 - color.b) * color.r;
//    float newG = abs(color.r + color.b * 2.0 - color.g) * color.r;
//    float newB = abs(color.r + color.b * 2.0 - color.g) * color.g;
//    vec4 newColor = vec4(newR, newG, newB, 1.0);
//    fragColor = newColor;

    // 10
     vec4 mask = texture2D(iChannel0, fragCoord);

     float newR = float(int(mask.r * 10.0)) / 10.0;
     float newG = float(int(mask.g * 10.0)) / 10.0;
     float newB = float(int(mask.b * 10.0)) / 10.0;

//     float newR = float(int(mask.r * 5.0)) / 5.0;
//     float newG = float(int(mask.g * 5.0)) / 5.0;
//     float newB = float(int(mask.b * 5.0)) / 5.0;

     vec4 tempColor = vec4(newR, newG, newB, 1.0);
     fragColor = tempColor;

     //////////
}

//void gaussian( out vec4 fragColor, in vec2 texCoord)
//{
//     width = 1280;
//     height = 720;
//
//     mediump vec4 total = vec4(0.0);
//     mediump vec4 grabPixel;
//
//     total +=        texture2D(tex2D, texCoord + vec2(-1.0 / width, -1.0 / height));
//     total +=        texture2D(tex2D, texCoord + vec2(1.0 / width, -1.0 / height));
//     total +=        texture2D(tex2D, texCoord + vec2(1.0 / width, 1.0 / height));
//     total +=        texture2D(tex2D, texCoord + vec2(-1.0 / width, 1.0 / height));
//
//     grabPixel = texture2D(tex2D, texCoord + vec2(0.0, -1.0 / height));
//     total += grabPixel * 2.0;
//
//     grabPixel = texture2D(tex2D, texCoord + vec2(0.0, 1.0 / height));
//     total += grabPixel * 2.0;
//
//     grabPixel =     texture2D(tex2D, texCoord + vec2(-1.0 / width, 0.0));
//     total += grabPixel * 2.0;
//
//     grabPixel =     texture2D(tex2D, texCoord + vec2(1.0 / width, 0.0));
//     total += grabPixel * 2.0;
//
//     grabPixel = texture2D(tex2D, texCoord);
//     total += grabPixel * 4.0;
//
//     total *= 1.0 / 16.0;
//
//     fragColor = total;
//}

void main() {
    mainImage(gl_FragColor, texCoord);
    //gaussian(gl_FragColor, texCoord);
}
