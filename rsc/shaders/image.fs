#version 330 core

out vec4 FragColor;

in vec4 vertexColor;
in vec2 vertexUV;

uniform sampler2D iChannel0;             // input channel (texture id).
uniform sampler2D iChannel1;             // input mask
uniform vec3      iResolution;           // viewport resolution (in pixels)

uniform vec4 color;
uniform float stipple;

void main()
{
    // color is a mix of texture (manipulated with brightness & contrast), vertex and uniform colors
    vec4 textureColor = texture(iChannel0, vertexUV);
    vec3 RGB = textureColor.rgb * vertexColor.rgb * color.rgb;

    // alpha is a mix of texture alpha, vertex alpha, and uniform alpha affected by stippling
    vec4 maskColor = texture(iChannel1, vertexUV);
    float maskIntensity = (maskColor.r + maskColor.g + maskColor.b) / 3.0;

    float A = textureColor.a * vertexColor.a * color.a * maskIntensity;
    A *= int(gl_FragCoord.x + gl_FragCoord.y) % 2 > (1 - int(stipple)) ? 0.0 : 1.0;

    // output RGBA
    FragColor = vec4(RGB, A);
}
