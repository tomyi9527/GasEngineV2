#version 100
#extension GL_OES_standard_derivatives : enable
precision highp float;

uniform float uFrameMod;
uniform float uMatcapCurvature;
uniform float uRGBMRange;
uniform int uDrawOpaque;
uniform int uInspectorView;
uniform int uOutputLinear;
uniform mat4 uProjectionMatrix;
uniform sampler2D Texture0;
uniform vec2 uNearFar;
uniform vec3 uMatcapColor;
uniform vec4 uHalton;

varying vec3 vViewNormal;
varying vec4 vViewVertex;
#define SHADER_NAME Matcap_Opaque(Material_12)

float linearTosRGB(const in float color) { return  color < 0.0031308 ? color * 12.92 : 1.055 * pow(color, 1.0/2.4) - 0.055; }
vec3 linearTosRGB(const in vec3 color) { return vec3( color.r < 0.0031308 ? color.r * 12.92 : 1.055 * pow(color.r, 1.0/2.4) - 0.055,  color.g < 0.0031308 ? color.g * 12.92 : 1.055 * pow(color.g, 1.0/2.4) - 0.055,  color.b < 0.0031308 ? color.b * 12.92 : 1.055 * pow(color.b, 1.0/2.4) - 0.055); }
vec4 linearTosRGB(const in vec4 color) { return vec4( color.r < 0.0031308 ? color.r * 12.92 : 1.055 * pow(color.r, 1.0/2.4) - 0.055,  color.g < 0.0031308 ? color.g * 12.92 : 1.055 * pow(color.g, 1.0/2.4) - 0.055,  color.b < 0.0031308 ? color.b * 12.92 : 1.055 * pow(color.b, 1.0/2.4) - 0.055, color.a); }

float sRGBToLinear(const in float color) { return  color < 0.04045 ? color * (1.0 / 12.92) : pow((color + 0.055) * (1.0 / 1.055), 2.4); }
vec3 sRGBToLinear(const in vec3 color) { return vec3( color.r < 0.04045 ? color.r * (1.0 / 12.92) : pow((color.r + 0.055) * (1.0 / 1.055), 2.4),  color.g < 0.04045 ? color.g * (1.0 / 12.92) : pow((color.g + 0.055) * (1.0 / 1.055), 2.4),  color.b < 0.04045 ? color.b * (1.0 / 12.92) : pow((color.b + 0.055) * (1.0 / 1.055), 2.4)); }
vec4 sRGBToLinear(const in vec4 color) { return vec4( color.r < 0.04045 ? color.r * (1.0 / 12.92) : pow((color.r + 0.055) * (1.0 / 1.055), 2.4),  color.g < 0.04045 ? color.g * (1.0 / 12.92) : pow((color.g + 0.055) * (1.0 / 1.055), 2.4),  color.b < 0.04045 ? color.b * (1.0 / 12.92) : pow((color.b + 0.055) * (1.0 / 1.055), 2.4), color.a); }

vec3 RGBMToRGB( const in vec4 rgba ) {
    const float maxRange = 8.0;
    return rgba.rgb * maxRange * rgba.a;
}

const mat3 LUVInverse = mat3( 6.0013, -2.700, -1.7995, -1.332, 3.1029, -5.7720, 0.3007, -1.088, 5.6268 );

vec3 LUVToRGB( const in vec4 vLogLuv ) {
    float Le = vLogLuv.z * 255.0 + vLogLuv.w;
    vec3 Xp_Y_XYZp;
    Xp_Y_XYZp.y = exp2((Le - 127.0) / 2.0);
    Xp_Y_XYZp.z = Xp_Y_XYZp.y / vLogLuv.y;
    Xp_Y_XYZp.x = vLogLuv.x * Xp_Y_XYZp.z;
    vec3 vRGB = LUVInverse * Xp_Y_XYZp;
    return max(vRGB, 0.0);
}

vec4 encodeRGBM(const in vec3 color, const in float range) {
    if(range <= 0.0) return vec4(color, 1.0);
    vec4 rgbm;
    vec3 col = color / range;
    rgbm.a = clamp( max( max( col.r, col.g ), max( col.b, 1e-6 ) ), 0.0, 1.0 );
    rgbm.a = ceil( rgbm.a * 255.0 ) / 255.0;
    rgbm.rgb = col / rgbm.a;
    return rgbm;
}

vec3 decodeRGBM(const in vec4 color, const in float range) {
    if(range <= 0.0) return color.rgb;
    return range * color.rgb * color.a;
}



float getCurvature(const in vec3 normal) {
    if (uMatcapCurvature == 0.0) return 1.0;
    
    vec3 dx = dFdx(normal);
    vec3 dy = dFdy(normal);
    vec3 xneg = normal - dx;
    vec3 xpos = normal + dx;
    vec3 yneg = normal - dy;
    vec3 ypos = normal + dy;
    
    float factor =  uMatcapCurvature / (length(vViewVertex.xyz) * atan(-1.0 / uProjectionMatrix[2][3]));
    float curvature = (cross(xneg, xpos).y - cross(yneg, ypos).x) * factor;
    
    float curvDirt = clamp(-curvature * 1.5, 0.0, 1.0);
    float curvEdge = clamp(curvature * 2.5, 0.0, 1.0);
    
    float factorDirt = mix(1.0, 0.3, curvDirt);
    float factorEdge = 2.0;
    
    return mix(factorDirt, factorEdge, curvEdge);
}

int decodeProfile(const in vec4 pack) {
    float packValue = floor(pack.b * 255.0 + 0.5);
    
    float profile = mod(packValue, 2.0);
    profile += mod(packValue - profile, 4.0);
    return int(profile);
}

float decodeDepth(const in vec4 pack) {
    if(decodeProfile(pack) == 0){
        const vec3 decode = 1.0 / vec3(1.0, 255.0, 65025.0);
        return dot(pack.rgb, decode);
    }
    
    return pack.r + pack.g / 255.0;
}

float decodeScatter(const in vec4 pack) {
    float scatter = pack.b - mod(pack.b, 4.0 / 255.0);
    return scatter * 255.0 / 4.0 / 63.0;
}

float decodeAlpha(const in vec4 pack) {
    return pack.a;
}

float distanceToDepth(const in sampler2D depth, const in vec2 uv, const in vec4 viewPos, const vec2 nearFar) {
    float fragDepth = clamp( (-viewPos.z * viewPos.w - nearFar.x) / (nearFar.y - nearFar.x), 0.0, 1.0);
    return fragDepth - decodeDepth(texture2D(depth, uv));
}

float pseudoRandom(const in vec2 fragCoord) {
    vec3 p3  = fract(vec3(fragCoord.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float interleavedGradientNoise(const in vec2 fragCoord, const in float frameMod) {
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(fragCoord.xy + frameMod * vec2(47.0, 17.0) * 0.695, magic.xy)));
}

float ditheringNoise(const in vec2 fragCoord, const in float frameMod) {
    
    float fm = frameMod;
    float dither5 = fract((fragCoord.x + fragCoord.y * 2.0 - 1.5 + fm) / 5.0);
    float noise = fract(dot(vec2(171.0, 231.0) / 71.0, fragCoord.xy));
    return (dither5 * 5.0 + noise) * (1.2 / 6.0);
}

void ditheringMaskingDiscard(
const in vec4 fragCoord,
const in int dithering,
const in float alpha,
const in float factor,

const in float thinLayer,

const in float frameMod,
const in vec2 nearFar,

const in vec4 halton) {
    
    if (dithering != 1) {
        if (alpha < factor) discard;
        return;
    }
    
    float rnd;
    
    if (thinLayer == 0.0) {
        float linZ = (1.0 / fragCoord.w - nearFar.x) / (nearFar.y - nearFar.x);
        float sliceZ = floor(linZ * 500.0) / 500.0;
        rnd = interleavedGradientNoise(fragCoord.xy + sliceZ, frameMod);
        } else {
        rnd = pseudoRandom(fragCoord.xy + halton.xy * 1000.0 + fragCoord.z * (abs(halton.z) == 2.0 ? 1000.0 : 1.0));
    }
    
    if (alpha * factor < rnd) discard;
}

void main() {
    
    vec3 eyeVector = -normalize(vViewVertex.xyz);
    vec3 frontNormal = normalize(gl_FrontFacing ? vViewNormal : -vViewNormal);
    
    vec3 materialNormal = frontNormal;
    
    vec3 materialDiffuse = uMatcapColor;
    
    vec3 nm_x = vec3(-eyeVector.z, 0.0, eyeVector.x);
    vec3 nm_y = cross(nm_x, eyeVector);
    vec2 uvMatcap = vec2(dot(materialNormal.xz, -nm_x.xz), dot(materialNormal, nm_y)) * 0.5 + 0.5;
    vec3 diffuse = materialDiffuse * sRGBToLinear(texture2D(Texture0, uvMatcap).rgb) * getCurvature(materialNormal);
    
    vec3 frag = diffuse;
    
    if (uInspectorView == 2) {
        frag = vec3(0.9, 0.75, 0.5) * max(0.25, dot(materialNormal.xyz, vec3(0.15, 0.3, 0.9)));
    }
    
    if (uOutputLinear != 1) frag = linearTosRGB(frag);
    
    gl_FragColor = encodeRGBM(frag, uRGBMRange);
}