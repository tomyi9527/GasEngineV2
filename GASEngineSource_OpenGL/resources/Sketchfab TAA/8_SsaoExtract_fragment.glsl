#version 100
#extension GL_OES_standard_derivatives : enable
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SSAO_TAP_EXTRACT(id) screenSpaceRadius = (float(id) + 0.5) * (1.0 / float(NB_SAMPLES));angle = screenSpaceRadius * (NB_SPIRAL_TURNS * 6.28) + randomAngle;screenSpaceRadius = max(0.75, screenSpaceRadius * ssRadius);offsetUnitVec = vec2(cos(angle), sin(angle));occludingPoint = getOffsetedPixelPos(texDepth, texDepth1, texDepth2, texDepth3, texDepth4, texDepth5, uv, offsetUnitVec, screenSpaceRadius, projInfo, texSize, nearFar);occludingPoint -= cameraSpacePosition;vv = dot(occludingPoint, occludingPoint);vn = dot(occludingPoint, normal);contrib += max(1.0 - vv * invRadius2, 0.0) * max((vn - bias) * inversesqrt(vv), 0.0);


uniform float uSsaoBias;
uniform float uSsaoIntensity;
uniform float uSsaoProjectionScale;
uniform float uSsaoRadius;
uniform float uTime;
uniform sampler2D TextureDepth1;
uniform sampler2D TextureDepth2;
uniform sampler2D TextureDepth3;
uniform sampler2D TextureDepth4;
uniform sampler2D TextureDepth5;
uniform sampler2D TextureDepth;
uniform vec2 RenderSize;
uniform vec2 uNearFar;
uniform vec4 uSsaoProjectionInfo;

varying vec2 vTexCoord0;


// the approximation :
// http://chilliant.blogspot.fr/2012/08/srgb-approximations-for-hlsl.html
// introduced slightly darker colors and more slight banding in the darks.
// The reference implementation (or even a single pow approx) did not introduced these effects.

// so for now we stick with the reference implementation :
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_sRGB.txt
// with the slight changes :
// - we always assume the color is >= 0.0 (so no check)
// - unlike the previous approximation, linear to srgb is monotonic so we don't need to check if the color is > 1

#define LIN_SRGB(x) x < 0.0031308 ? x * 12.92 : 1.055 * pow(x, 1.0/2.4) - 0.055
float linearTosRGB(const in float c) {
    return LIN_SRGB(c);
}
vec3 linearTosRGB(const in vec3 c) {
    return vec3(LIN_SRGB(c.r), LIN_SRGB(c.g), LIN_SRGB(c.b));
}
vec4 linearTosRGB(const in vec4 c) {
    return vec4(LIN_SRGB(c.r), LIN_SRGB(c.g), LIN_SRGB(c.b), c.a);
}

#define SRGB_LIN(x) x < 0.04045 ? x * (1.0 / 12.92) : pow((x + 0.055) * (1.0 / 1.055), 2.4)
float sRGBToLinear(const in float c) {
    return SRGB_LIN(c);
}
vec3 sRGBToLinear(const in vec3 c) {
    return vec3(SRGB_LIN(c.r), SRGB_LIN(c.g), SRGB_LIN(c.b));
}
vec4 sRGBToLinear(const in vec4 c) {
    return vec4(SRGB_LIN(c.r), SRGB_LIN(c.g), SRGB_LIN(c.b), c.a);
}

//http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec3 RGBMToRGB( const in vec4 rgba ) {
    const float maxRange = 8.0;
    return rgba.rgb * maxRange * rgba.a;
}

const mat3 LUVInverse = mat3( 6.0013,    -2.700,   -1.7995,
                              -1.332,    3.1029,   -5.7720,
                              0.3007,    -1.088,    5.6268 );

vec3 LUVToRGB( const in vec4 vLogLuv ) {
    float Le = vLogLuv.z * 255.0 + vLogLuv.w;
    vec3 Xp_Y_XYZp;
    Xp_Y_XYZp.y = exp2((Le - 127.0) / 2.0);
    Xp_Y_XYZp.z = Xp_Y_XYZp.y / vLogLuv.y;
    Xp_Y_XYZp.x = vLogLuv.x * Xp_Y_XYZp.z;
    vec3 vRGB = LUVInverse * Xp_Y_XYZp;
    return max(vRGB, 0.0);
}

// http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec4 encodeRGBM(const in vec3 col, const in float range) {
    if(range <= 0.0)
        return vec4(col, 1.0);
    vec4 rgbm;
    vec3 color = col / range;
    rgbm.a = clamp( max( max( color.r, color.g ), max( color.b, 1e-6 ) ), 0.0, 1.0 );
    rgbm.a = ceil( rgbm.a * 255.0 ) / 255.0;
    rgbm.rgb = color / rgbm.a;
    return rgbm;
}

vec3 decodeRGBM(const in vec4 col, const in float range) {
    if(range <= 0.0)
        return col.rgb;
    return range * col.rgb * col.a;
}

#define NB_SAMPLES 11
// Constant used to scale the depth view value that is given to the blur pass
#define MIN_RADIUS 1.0
#define NB_SPIRAL_TURNS 3.0

float zValueFromScreenSpacePosition(const in sampler2D texDepth, const in vec2 uv, const in vec2 nearFar) {
    float d = dot(texture2D(texDepth, uv).rgb, vec3(1.0, 1.0/255.0, 1.0/65025.0));
    return nearFar.x + (nearFar.y - nearFar.x) * d;
}

vec3 reconstructCSPosition(const in vec2 ssP, const in float z, const in vec4 projInfo) {
    return vec3((ssP.xy * projInfo.xy + projInfo.zw) * z, z);
}

vec3 getPosition(const in sampler2D texDepth, const in vec2 uv, const in vec2 texSize, const in vec4 projInfo, const in vec2 nearFar) {
    return reconstructCSPosition(uv * texSize, zValueFromScreenSpacePosition(texDepth, uv, nearFar), projInfo);
}

# define MAX_MIP_LEVEL 5
// Determines at which point we should switch mip level
// if number is too small (~3) will lead to flashing (bad variance as many taps give same pixel)
// if number is too high, mip level are not used well and the cache is not used efficiently
# define LOG_MAX_OFFSET 3

vec3 getOffsetedPixelPos(
    const in sampler2D texmip0,
    const in sampler2D texmip1,
    const in sampler2D texmip2,
    const in sampler2D texmip3,
    const in sampler2D texmip4,
    const in sampler2D texmip5,
    const in vec2 uv,
    const in vec2 unitOffset,
    const in float screenSpaceRadius,
    const in vec4 projInfo,
    const in vec2 texSize,
    const in vec2 nearFar ) {

    int mipLevel = int(clamp(floor(log2(screenSpaceRadius)) - float(LOG_MAX_OFFSET), 0.0, float(MAX_MIP_LEVEL)));
    vec2 uvOff = uv + floor(screenSpaceRadius * unitOffset) / texSize;

    float d;
    if (mipLevel == 1) d = zValueFromScreenSpacePosition(texmip1, uvOff, nearFar);
    else if (mipLevel == 2) d = zValueFromScreenSpacePosition(texmip2, uvOff, nearFar);
    else if (mipLevel == 3) d = zValueFromScreenSpacePosition(texmip3, uvOff, nearFar);
    else if (mipLevel == 4) d = zValueFromScreenSpacePosition(texmip4, uvOff, nearFar);
    else if (mipLevel == 5) d = zValueFromScreenSpacePosition(texmip5, uvOff, nearFar);
    else d = zValueFromScreenSpacePosition(texmip0, uvOff, nearFar);

    return reconstructCSPosition(uvOff * texSize, d, projInfo);
}

float nrand(const in vec3 uvt) {
  vec3 p3 = fract(uvt * 443.8975);
  p3 += dot(p3, p3.yzx + 19.19);
  return fract((p3.x + p3.y) * p3.z);
}

#define PIOVER8 0.39269908169
vec3 unpackNormal1(const in float pack1) {
    float pack8 = floor(pack1 * 255.0);
    float th = PIOVER8 * float(pack8 / 16.0);
    float len = sqrt(mod(float(pack8), 16.0) / 15.001);
    vec2 nv = vec2(cos(th), sin(th)) * len;
    return -vec3(nv.x, nv.y, sqrt(max(0.0, 1.0 - nv.x * nv.x - nv.y * nv.y)));
}

vec3 unpackNormal2(const in vec2 pack2) {
    vec3 nv = pack2.rgg * 2.0 - 1.0;
    return -vec3(nv.x, nv.y, sqrt(max(0.0, 1.0 - nv.x * nv.x - nv.y * nv.y)));
}

#ifndef SSAO_TAP_EXTRACT
#define SSAO_TAP_EXTRACT(id)  
#endif

vec4 ssaoExtract(
        const in sampler2D texDepth,
        const in vec2 uv,
        const in vec2 nearFar,
        const in float radius,
        const in float intensity,
        const in float bias,
        const in vec4 projInfo,
        const in float projScale,
        const in vec2 texSize,
        const in float time,
        const in sampler2D texDepth1,
        const in sampler2D texDepth2,
        const in sampler2D texDepth3,
        const in sampler2D texDepth4,
        const in sampler2D texDepth5) {

    vec3 depthPacked = texture2D(texDepth, uv).rgb;
    vec3 cameraSpacePosition = getPosition(texDepth, uv, texSize, projInfo, nearFar);
    float ssRadius = -projScale * radius / cameraSpacePosition.z;

#ifdef SSAO_NORMAL
    vec3 normal = unpackNormal2(texture2D(texDepth, uv).ba); // g buffer normal 2 component on ba
#elif defined(GL_OES_standard_derivatives) && !defined(MOBILE)
    vec3 normal = cross(dFdy(cameraSpacePosition), dFdx(cameraSpacePosition));
#else
    vec3 cam0 = getPosition(texDepth, uv - vec2(1.0, 0.0) / texSize, texSize, projInfo, nearFar);
    vec3 cam1 = getPosition(texDepth, uv + vec2(1.0, 0.0) / texSize, texSize, projInfo, nearFar);
    vec3 cam2 = getPosition(texDepth, uv - vec2(0.0, 1.0) / texSize, texSize, projInfo, nearFar);
    vec3 cam3 = getPosition(texDepth, uv + vec2(0.0, 1.0) / texSize, texSize, projInfo, nearFar);
    vec3 normal = cross(cam0 - cam1, cam3 - cam2);
#endif

    // early return background or radius too small (should be check after derivatives usage)
    if (depthPacked.x == 1.0 || ssRadius < MIN_RADIUS) {
        return vec4(1.0, depthPacked);
    }

    normal = normalize(normal);
    float nFalloff = mix(1.0, max(0.0, 1.5 * normal.z), 0.35);

    float randomAngle = nrand(vec3(uv, 0.07 * fract(time))) * 6.3; // needs to be > 2PI

    float invRadius2 = 1.0 / (radius * radius);
    float contrib = 0.0;
    // for (int i = 0; i < NB_SAMPLES; ++i) {
    //     float screenSpaceRadius = (float(i) + 0.5) * (1.0 / float(NB_SAMPLES));
    //     float angle = screenSpaceRadius * (NB_SPIRAL_TURNS * 6.28) + randomAngle;
    //     screenSpaceRadius = max(0.75, screenSpaceRadius * ssRadius);
    //     vec2 offsetUnitVec = vec2(cos(angle), sin(angle));
    //     vec3 occludingPoint = getOffsetedPixelPos(texDepth, texDepth1, texDepth2, texDepth3, texDepth4, texDepth5, uv, offsetUnitVec, screenSpaceRadius, projInfo, texSize, nearFar);
    //     occludingPoint -= cameraSpacePosition;
    //     float vv = dot(occludingPoint, occludingPoint);
    //     float vn = dot(occludingPoint, normal);
    //     contrib += max(1.0 - vv * invRadius2, 0.0) * max((vn - bias) * inversesqrt(vv), 0.0);
    // }

    // ---- UNROLL ----
    float vv;
    float vn;
    float screenSpaceRadius;
    float angle;
    vec3 occludingPoint;
    vec2 offsetUnitVec;
    SSAO_TAP_EXTRACT(0);
    SSAO_TAP_EXTRACT(1);
    SSAO_TAP_EXTRACT(2);
    SSAO_TAP_EXTRACT(3);
    SSAO_TAP_EXTRACT(4);
    SSAO_TAP_EXTRACT(5);
    SSAO_TAP_EXTRACT(6);
    SSAO_TAP_EXTRACT(7);
    SSAO_TAP_EXTRACT(8);
    SSAO_TAP_EXTRACT(9);
    SSAO_TAP_EXTRACT(10);
    // ---- UNROLL ----

    float aoValue = max(0.0, 1.0 - sqrt(contrib * nFalloff / float(NB_SAMPLES)));
    aoValue = pow(aoValue, 10.0 * intensity);

    vec4 aoDepth;
    aoDepth.r = mix(1.0, aoValue, clamp(ssRadius - MIN_RADIUS, 0.0, 1.0));
    aoDepth.gba = depthPacked;

    return aoDepth;
}

#define SSAO_FILTER_RADIUS 3

#ifndef SSAO_TAP_BLUR
#define SSAO_TAP_BLUR(id, absid)  
#endif

vec4 ssaoBlur(const in sampler2D texSsao, const in vec2 uv, const in vec2 axis, const in vec2 nearFar) {

    vec4 aoDepth = texture2D(texSsao, uv);

    // background
    if(aoDepth.y == 1.0){
        return aoDepth;
    }

    float initialZ = dot(aoDepth.gba, vec3(1.0, 1.0/255.0, 1.0/65025.0));

    float gaussian[SSAO_FILTER_RADIUS + 2]; // dummy because of intel off-by-one bug
    gaussian[0] = 0.153170;
    gaussian[1] = 0.144893;
    gaussian[2] = 0.122649;
    gaussian[3] = 0.092902;
    gaussian[4] = 0.0;

    float totalWeight = gaussian[0];
    float sum = aoDepth.r * totalWeight;
    float sharpnessFactor = 400.0;

    // for (int r = - SSAO_FILTER_RADIUS; r <= SSAO_FILTER_RADIUS; ++r) {
    //     if (r != 0) {
    //         vec4 sample = texture2D(texSsao, uv + axis * float(r) * SSAO_SCALE);
    //         float z = dot(sample.gba, vec3(1.0, 1.0/255.0, 1.0/65025.0));
    //         float weight = 0.3 + gaussian[int(abs(float(r)))];
    //         weight *= max(0.0, 1.0 - sharpnessFactor * abs(z - initialZ));
    //         sum += sample.r * weight;
    //         totalWeight += weight;
    //     }
    // }

    // ---- UNROLL ----
    vec2 ofs;
    float z;
    float weight;
    vec4 sampleTex;

    SSAO_TAP_BLUR(-3, 3);
    SSAO_TAP_BLUR(-2, 2);
    SSAO_TAP_BLUR(-1, 1);
    SSAO_TAP_BLUR(1, 1);
    SSAO_TAP_BLUR(2, 2);
    SSAO_TAP_BLUR(3, 3);
    // ---- UNROLL ----

    aoDepth.r = sum / totalWeight;
    return aoDepth;
}

vec4 ssaoBlurH(const in sampler2D texSsao, const in vec2 uv, const in vec2 texSize, const in vec2 nearFar) {
    return ssaoBlur(texSsao, uv, vec2(1.0, 0.0) / texSize, nearFar);
}

vec3 ssaoBlurV(const in sampler2D texSsao, const in vec2 uv, const in vec2 texSize, const in vec2 nearFar, const in vec3 color) {
    return color * ssaoBlur(texSsao, uv, vec2(0.0, 1.0) / texSize, nearFar).rrr;
}
void main() {
gl_FragColor = ssaoExtract( TextureDepth, vTexCoord0, uNearFar, uSsaoRadius, uSsaoIntensity, uSsaoBias, uSsaoProjectionInfo, uSsaoProjectionScale, RenderSize, uTime, TextureDepth1, TextureDepth2, TextureDepth3, TextureDepth4, TextureDepth5 );

}
#define SHADER_NAME SsaoExtract
