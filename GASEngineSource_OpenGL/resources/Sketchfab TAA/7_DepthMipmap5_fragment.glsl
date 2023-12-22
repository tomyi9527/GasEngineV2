#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif



uniform sampler2D Texture0;

varying vec2 vTexCoord0;


vec3 textureRGB(const in sampler2D tex, const in vec2 uv) {
    return texture2D(tex, uv.xy ).rgb;
}

vec4 textureRGBA(const in sampler2D tex, const in vec2 uv) {
    return texture2D(tex, uv.xy ).rgba;
}

float textureIntensity(const in sampler2D tex, const in vec2 uv) {
    return texture2D(tex, uv).r;
}

float textureAlpha(const in sampler2D tex, const in vec2 uv) {
    return texture2D(tex, uv.xy ).a;
}

void main() {
// vars

vec4 tmp_2;

// end vars

tmp_2 = textureRGBA( Texture0, vTexCoord0.xy );

float depth = dot(tmp_2.rgb, vec3(1.0, 1.0/255.0, 1.0/65025.0));
gl_FragColor.rgb = vec3(1.0, 255.0, 65025.0) * depth;
gl_FragColor.gb = fract(gl_FragColor.gb);
gl_FragColor.rg -= gl_FragColor.gb * (1.0/256.0);
}
#define SHADER_NAME DepthMipmap5
