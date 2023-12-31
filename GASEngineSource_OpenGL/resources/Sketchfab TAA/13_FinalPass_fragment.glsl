#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif



uniform int uOutputLinear;
uniform sampler2D Texture0;
uniform vec2 RenderSize;

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

// G3D version of FXAA
// green (y) as luma
// Quality 12
vec3 fxaa(const in sampler2D tex, const in vec2 pos, const in vec2 texSize) {

    vec2 fxaaQualityRcpFrame = 1.0 / texSize.xy;
    float fxaaQualitySubpix = 0.75;
    float fxaaQualityEdgeThreshold = 0.125;
    float fxaaQualityEdgeThresholdMin = 0.0625;

    vec2 posM = pos.xy;
    vec4 rgbyM = texture2D(tex, posM);
    float lumaM = rgbyM.y;

    // Swizzler Offseter Util
    vec4 sw = vec4(-1.0, 1.0, 1.0, -1.0) * fxaaQualityRcpFrame.xxyy;

    float lumaS = texture2D(tex, posM + vec2(0.0, sw.z)).y;
    float lumaE = texture2D(tex, posM + vec2(sw.y, 0.0)).y;
    float lumaN = texture2D(tex, posM + vec2(0.0, sw.w)).y;
    float lumaW = texture2D(tex, posM + vec2(sw.x, 0.0)).y;

    float maxSM = max(lumaS, lumaM);
    float minSM = min(lumaS, lumaM);

    float maxESM = max(lumaE, maxSM);
    float minESM = min(lumaE, minSM);

    float maxWN = max(lumaN, lumaW);
    float minWN = min(lumaN, lumaW);

    float rangeMax = max(maxWN, maxESM);
    float rangeMin = min(minWN, minESM);

    float rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;
    float range = rangeMax - rangeMin;
    float rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);

    bool earlyExit = range < rangeMaxClamped;
    if(earlyExit)
        return rgbyM.rgb;

    float lumaNW = texture2D(tex, posM + sw.xw).y;
    float lumaSE = texture2D(tex, posM + sw.yz).y;
    float lumaNE = texture2D(tex, posM + sw.yw).y;
    float lumaSW = texture2D(tex, posM + sw.xy).y;

    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float subpixRcpRange = 1.0/range;
    float subpixNSWE = lumaNS + lumaWE;
    float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
    float edgeVert1 = (-2.0 * lumaM) + lumaWE;

    float lumaNESE = lumaNE + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;
    float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
    float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;

    float lumaNWSW = lumaNW + lumaSW;
    float lumaSWSE = lumaSW + lumaSE;
    float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
    float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
    float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
    float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
    float edgeHorz = abs(edgeHorz3) + edgeHorz4;
    float edgeVert = abs(edgeVert3) + edgeVert4;

    float subpixNWSWNESE = lumaNWSW + lumaNESE;
    float lengthSign = fxaaQualityRcpFrame.x;
    bool horzSpan = edgeHorz >= edgeVert;
    float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;

    if(!horzSpan) lumaN = lumaW;
    if(!horzSpan) lumaS = lumaE;
    if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;
    float subpixB = (subpixA * (1.0/12.0)) - lumaM;

    float gradientN = lumaN - lumaM;
    float gradientS = lumaS - lumaM;
    float lumaNN = lumaN + lumaM;
    float lumaSS = lumaS + lumaM;
    bool pairN = abs(gradientN) >= abs(gradientS);
    float gradient = max(abs(gradientN), abs(gradientS));
    if(pairN) lengthSign = -lengthSign;
    float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);

    vec2 posB = posM.xy;
    vec2 offNP;
    offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;
    offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;
    if(!horzSpan) posB.x += lengthSign * 0.5;
    if( horzSpan) posB.y += lengthSign * 0.5;

    vec2 posN;
    posN.x = posB.x - offNP.x;
    posN.y = posB.y - offNP.y;
    vec2 posP;
    posP.x = posB.x + offNP.x;
    posP.y = posB.y + offNP.y;
    float subpixD = ((-2.0)*subpixC) + 3.0;
    float lumaEndN = texture2D(tex, posN).y;
    float subpixE = subpixC * subpixC;
    float lumaEndP = texture2D(tex, posP).y;

    if(!pairN) lumaNN = lumaSS;
    float gradientScaled = gradient * 1.0/4.0;
    float lumaMM = lumaM - lumaNN * 0.5;
    float subpixF = subpixD * subpixE;
    bool lumaMLTZero = lumaMM < 0.0;

    lumaEndN -= lumaNN * 0.5;
    lumaEndP -= lumaNN * 0.5;
    bool doneN = abs(lumaEndN) >= gradientScaled;
    bool doneP = abs(lumaEndP) >= gradientScaled;
    if(!doneN) posN.x -= offNP.x * 1.5;
    if(!doneN) posN.y -= offNP.y * 1.5;
    bool doneNP = (!doneN) || (!doneP);
    if(!doneP) posP.x += offNP.x * 1.5;
    if(!doneP) posP.y += offNP.y * 1.5;

    if(doneNP) {
        if(!doneN) lumaEndN = texture2D(tex, posN.xy).y;
        if(!doneP) lumaEndP = texture2D(tex, posP.xy).y;
        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
        doneN = abs(lumaEndN) >= gradientScaled;
        doneP = abs(lumaEndP) >= gradientScaled;
        if(!doneN) posN.x -= offNP.x * 2.0;
        if(!doneN) posN.y -= offNP.y * 2.0;
        doneNP = (!doneN) || (!doneP);
        if(!doneP) posP.x += offNP.x * 2.0;
        if(!doneP) posP.y += offNP.y * 2.0;

        if(doneNP) {
            if(!doneN) lumaEndN = texture2D(tex, posN.xy).y;
            if(!doneP) lumaEndP = texture2D(tex, posP.xy).y;
            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
            doneN = abs(lumaEndN) >= gradientScaled;
            doneP = abs(lumaEndP) >= gradientScaled;
            if(!doneN) posN.x -= offNP.x * 4.0;
            if(!doneN) posN.y -= offNP.y * 4.0;
            doneNP = (!doneN) || (!doneP);
            if(!doneP) posP.x += offNP.x * 4.0;
            if(!doneP) posP.y += offNP.y * 4.0;

            if(doneNP) {
                if(!doneN) lumaEndN = texture2D(tex, posN.xy).y;
                if(!doneP) lumaEndP = texture2D(tex, posP.xy).y;
                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                doneN = abs(lumaEndN) >= gradientScaled;
                doneP = abs(lumaEndP) >= gradientScaled;
                if(!doneN) posN.x -= offNP.x * 12.0;
                if(!doneN) posN.y -= offNP.y * 12.0;
                doneNP = (!doneN) || (!doneP);
                if(!doneP) posP.x += offNP.x * 12.0;
                if(!doneP) posP.y += offNP.y * 12.0;

            }

        }

    }

    float dstN = posM.x - posN.x;
    float dstP = posP.x - posM.x;
    if(!horzSpan) dstN = posM.y - posN.y;
    if(!horzSpan) dstP = posP.y - posM.y;

    bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
    float spanLength = (dstP + dstN);
    bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
    float spanLengthRcp = 1.0/spanLength;

    bool directionN = dstN < dstP;
    float dst = min(dstN, dstP);
    bool goodSpan = directionN ? goodSpanN : goodSpanP;
    float subpixG = subpixF * subpixF;
    float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
    float subpixH = subpixG * fxaaQualitySubpix;

    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
    float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
    if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
    if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;

    return texture2D(tex, posM).xyz;
}

float unpackDepth(const in sampler2D texDepth, const in vec2 uv) {
    return dot(texture2D(texDepth, uv).rgb, vec3(1.0, 1.0/255.0, 1.0/65025.0));
}

vec3 reconstructWSPosition(const in vec2 uv, const in vec3 corners[4], const in mat4 invView, const in float depth) {
    vec2 finalUv = uv;
#ifdef VR_ENABLED
    float xViewportOffset = uv.x >= 0.5 ? -0.5 : 0.0;
    finalUv = vec2( ( uv.x + xViewportOffset ) * 2.0, uv.y );
#endif

    // the corners are in view space, it means if we interpolate between them we can retrieve the view space direction of the fragment
    vec3 A = mix(corners[0], corners[1], finalUv.x);
    vec3 B = mix(corners[2], corners[3], finalUv.x);

    // multiply this view space direction by the depth gives us the view space position
    vec3 vsPos = mix( A, B, finalUv.y ) * depth;

    // multiply by the inverse of the view matrix gives us the world space position
    return (invView * vec4(vsPos, 1.0)).xyz;
}

// access a linear texture as if a nearest texture
vec4 textureNearest2D(const in sampler2D tex, const in vec2 uv, const in vec2 texSize)
{
  return texture2D(tex, (floor(uv*texSize)+0.5) / texSize, -99999.0);
}

#define NEIGHBOUR_CREATE_FROM_OFFSET(x,y) vec3(x, y, unpackDepth(texDepth, uv + vec2(x,y) * texelSize) )
#define NEIGHBOUR_SET_IF_DMIN_Z_GREATER(neighbour) if(neighbour.z < dmin.z) dmin = neighbour;

// Neighbourhood clamping from Tiago Sousa
vec3 closestFragment(const in sampler2D texDepth, const in vec2 uv, const in vec2 texelSize) {
    const float size = 2.0;

    vec3 tl = NEIGHBOUR_CREATE_FROM_OFFSET(-size, size);
    vec3 t = NEIGHBOUR_CREATE_FROM_OFFSET(0.0, size);
    vec3 tr = NEIGHBOUR_CREATE_FROM_OFFSET(size, size);
    vec3 ml = NEIGHBOUR_CREATE_FROM_OFFSET(-size, 0.0);
    vec3 m = NEIGHBOUR_CREATE_FROM_OFFSET(0.0, 0.0);
    vec3 mr = NEIGHBOUR_CREATE_FROM_OFFSET(size, 0.0);
    vec3 bl = NEIGHBOUR_CREATE_FROM_OFFSET(-size, -size);
    vec3 b = NEIGHBOUR_CREATE_FROM_OFFSET(0.0, -size);
    vec3 br = NEIGHBOUR_CREATE_FROM_OFFSET(size, -size);

    vec3 dmin = tl;

    //NEIGHBOUR_SET_IF_DMIN_Z_GREATER(t)
    NEIGHBOUR_SET_IF_DMIN_Z_GREATER(tr)
    //NEIGHBOUR_SET_IF_DMIN_Z_GREATER(ml)
    NEIGHBOUR_SET_IF_DMIN_Z_GREATER(m)
    //NEIGHBOUR_SET_IF_DMIN_Z_GREATER(mr)
    NEIGHBOUR_SET_IF_DMIN_Z_GREATER(bl)
    //NEIGHBOUR_SET_IF_DMIN_Z_GREATER(b)
    NEIGHBOUR_SET_IF_DMIN_Z_GREATER(br)

    return vec3(uv + dmin.xy * texelSize, dmin.z);
}

// weighting by unbiased luminance diff from https://www.youtube.com/watch?v=WzpLWzGvFK4&t=18m
float luminance(vec3 color) {
    return dot(color, vec3(0.22, 0.707, 0.071));
}

vec3 clip_aabb(const in vec3 bbmin, const in vec3 bbmax, vec3 p, vec3 q) {
    const float eps = 0.00000001;

    // unoptimized version
    vec3 r = q - p;
    vec3 rmax = bbmax - p;
    vec3 rmin = bbmin - p;

    if( r.x > rmax.x + eps )
        r *= ( rmax.x / r.x );
    if( r.y > rmax.y + eps )
        r *= ( rmax.y / r.y );
    if( r.z > rmax.z + eps )
        r *= ( rmax.z / r.z );

    if( r.x < rmin.x - eps )
        r *= ( rmin.x / r.x );
    if( r.y < rmin.y - eps )
        r *= ( rmin.y / r.y );
    if( r.z < rmin.z - eps )
        r *= ( rmin.z / r.z );

    return p + r;
}

// this function is a version with less branches than clip_aabb
// no visual differences as been observed between the two version, thus this one is used
vec3 clip_aabb_opti(const in vec3 bbmin, const in vec3 bbmax, const in vec3 p, const in vec3 q ) {
    const float eps = 0.00000001;

    vec3 p_clip = 0.5 * ( bbmax + bbmin );
    vec3 e_clip = 0.5 * ( bbmax - bbmin ) + eps;

    vec3 v_clip = q - p_clip;
    vec3 v_unit = v_clip / e_clip;
    vec3 a_unit = abs( v_unit );

    float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

    if(ma_unit > 1.0)
        return p_clip + v_clip / ma_unit;

    return q;
}

vec3 taa(const in sampler2D currentTex, const in sampler2D prevTex, const in vec2 uv, const in vec2 ssVel, const in vec2 texelSize, const in vec2 jitter,
    const in bool useSharpen) {

    vec2 unjittered_uv = uv - jitter;

    vec3 texel0 = texture2D(currentTex, unjittered_uv).rgb;
    vec3 texel1 = texture2D(prevTex, uv - ssVel).rgb;

    unjittered_uv = uv;

    vec3 tl = texture2D(currentTex, unjittered_uv + vec2(-texelSize.x, texelSize.y)).rgb;
    vec3 t = texture2D(currentTex, unjittered_uv + vec2(0.0, texelSize.y)).rgb;
    vec3 tr = texture2D(currentTex, unjittered_uv + vec2(texelSize.x, texelSize.y)).rgb;
    vec3 ml = texture2D(currentTex, unjittered_uv + vec2(-texelSize.x, 0.0)).rgb;
    vec3 m = texture2D(currentTex, unjittered_uv + vec2(0.0, 0.0 )).rgb;
    vec3 mr = texture2D(currentTex, unjittered_uv + vec2(texelSize.x, 0.0)).rgb;
    vec3 bl = texture2D(currentTex, unjittered_uv + vec2(-texelSize.x, -texelSize.y)).rgb;
    vec3 b = texture2D(currentTex, unjittered_uv + vec2(0.0, -texelSize.y)).rgb;
    vec3 br = texture2D(currentTex, unjittered_uv + vec2(texelSize.x, -texelSize.y)).rgb;

    // https://github.com/Unity-Technologies/PostProcessing/blob/v1/PostProcessing/Resources/Shaders/TAA.cginc#L143
    // reduces blurring, but increases flickering with dense geometry
    if(useSharpen){
        vec3 corners = 2.0 * (tr + bl + br + tl) - 2.0 * texel0;
        texel0 += (texel0 - (corners * 0.166667)) * 2.718282 * 0.3;
        texel0 = max(vec3(0.0), texel0);
    }

    vec3 cmin = min(tl, min(t, min(tr, min(ml, min(m, min(mr, min(bl, min(b, br))))))));
    vec3 cmax = max(tl, max(t, max(tr, max(ml, max(m, max(mr, max(bl, max(b, br))))))));

    vec3 cavg = tl + t + tr + ml + m + mr + bl + b + br;
    cavg /= 9.0;

    // Brian Karis neighbourhood rounding: http://advances.realtimerendering.com/s2014/epic/TemporalAA.pptx
    vec3 cmin5 = min(mr, min(m, min(ml, min(t, b))));
    vec3 cmax5 = max(mr, max(m, max(ml, max(t, b))));
    vec3 cavg5 = (mr + m + ml + t + b) / 5.0;

    cmin = 0.5 * (cmin + cmin5);
    cmax = 0.5 * (cmax + cmax5);
    cavg = 0.5 * (cavg + cavg5);

    texel1 = clip_aabb_opti(cmin, cmax, clamp(cavg, cmin, cmax), texel1);

#ifndef FEEDBACK_MIN
#define FEEDBACK_MIN 0.0
#define FEEDBACK_MAX 1.0
#endif
    const vec2 feedbackRange = vec2(FEEDBACK_MIN, FEEDBACK_MAX); // playdead's default values

    float lum0 = luminance(texel0.rgb);
    float lum1 = luminance(texel1.rgb);

    float diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
    float unbiased_weight = 1.0 - diff;
    float feedback = mix(feedbackRange.x, feedbackRange.y, unbiased_weight * unbiased_weight);

    vec3 color = mix(texel0, texel1, feedback);
    return color;
}

vec2 computeSSVelocity(const in vec3 wsPos, const in mat4 currentFrameProjView, const in mat4 lastFrameProjView, const in bool rightEye)
{
    vec4 ssCurrentPos = currentFrameProjView * vec4(wsPos, 1.0);
    vec4 ssPrevPos = lastFrameProjView * vec4(wsPos, 1.0);

    vec2 ndcCurrent = ssCurrentPos.xy / ssCurrentPos.w;
    vec2 ndcPrev = ssPrevPos.xy / ssPrevPos.w;

    //ndcPrev = clamp(ndcPrev, vec2(-1.0), vec2(1.0));
    if( ndcPrev.x >= 1.0 || ndcPrev.x <= -1.0 || ndcPrev.x >= 1.0 || ndcPrev.y <= -1.0 )
        return vec2(0.0);

#ifdef VR_ENABLED
    ndcCurrent.x /= 2.0;
    ndcPrev.x /= 2.0;

    if( rightEye ) {
        ndcCurrent.x += 0.5;
        ndcPrev.x += 0.5;
    }
#endif

    return 0.5 * (ndcCurrent - ndcPrev);
}

// static in place amortized supersampling
vec3 supersample(const in sampler2D currentTex, const in sampler2D prevTex, const in vec2 uv,
    const in vec2 texSize, const in vec4 halton) {

    vec3 currFragColor = texture2D(currentTex, uv).rgb;

    float haltz = abs(halton.z);
    if(haltz == 1.0) {
        // just return the color.
        return currFragColor;
    }

    // same pixel for same projected interpolated vertex on prev frame
    vec3 accumColorN = textureNearest2D(prevTex, uv.xy, texSize).rgb;

    // mix frames
    float lerpFac = 1.0 / halton.w;

#ifdef TAA_ACCUM
    if(uHalton.w == 1.0) lerpFac = 0.0;
#endif

    if (haltz == 3.0) {
        // No supersample, crude motion blur by accumulation
        return mix(currFragColor, accumColorN, lerpFac);
    }

    // Supsersample, accumulation + jittering
    // http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
    // cumulative moving average over frameNum (which starts at 2)
    // n is previous frame
    // accumColorN = Accum(n)
    // return value is accumColor( n +1 )
    // Accum(n+1) = Accum(n) + ((x(n+1) - Accum(n)) / (n + 1)))
    // formula above is equal to glsl  mix
    return mix(accumColorN, currFragColor, lerpFac);
}

vec3 computeTaa(const in sampler2D currentTex, const in sampler2D prevTex, const in vec2 uv, const in vec2 texSize, const in vec4 halton,
    const in sampler2D texDepth, const in vec2 nearFar, const in mat4 invView, const in mat4 currentFrameProjView, const in mat4 lastFrameProjView,
    const in vec3 corners[4], const in float alpha, const in bool useSharpen) {

    vec2 texelSize = vec2(1.0) / texSize;

    // halton offsets vertices in ndc space, thus we have to convert them to screenspace in order to use it
    vec2 jitter = ( halton.xy * vec2(0.5) ) * texelSize;

    float haltz = abs(halton.z);

    if (haltz == 1.0) {
        vec3 closest = closestFragment(texDepth, uv, texelSize);

        // discard background fragments to save fillrate
        // make sure the mesh isn't transparent
        if( closest.z >= 1.0
#ifdef TAA_TRANSPARENT
            && alpha == 0.0
#endif
        )
#ifdef TAA_VR_FXAA
            return fxaa(currentTex, uv-jitter, texSize);
#else
            return texture2D(currentTex, uv-jitter).rgb;
#endif

        float depth = -(nearFar.x + (nearFar.y - nearFar.x) * closest.z);
        vec3 ws = reconstructWSPosition(closest.xy, corners, invView, depth);

        vec2 ssVel = computeSSVelocity(ws, currentFrameProjView, lastFrameProjView, uv.x >= 0.5);

        return taa(currentTex, prevTex, uv, ssVel, texelSize, jitter, useSharpen);
    }

    return supersample(currentTex, prevTex, uv, texSize, halton);
}

// temporal anti-aliasing from Playdead's Inside
// slides: http://www.gdcvault.com/play/1022970/Temporal-Reprojection-Anti-Aliasing-in
// sources: https://github.com/playdeadgames/temporal
vec3 supersampleTaa(const in sampler2D currentTex, const in sampler2D prevTex, const in vec2 uv, const in vec2 texSize, const in vec4 halton,
    const in sampler2D texDepth, const in vec2 nearFar, const in float enableTaa, const in float alpha, const in mat4 invViewLeft,
    const in mat4 currentFrameProjViewLeft, const in mat4 lastFrameProjViewLeft, const in vec3 cornersLeft[4]
#ifdef VR_ENABLED
    , const in mat4 invViewRight, const in mat4 currentFrameProjViewRight, const in mat4 lastFrameProjViewRight, const in vec3 cornersRight[4]
#endif
    ) {

    if( enableTaa == 0.0 )
        return supersample(currentTex, prevTex, uv, texSize, halton);

#ifdef VR_ENABLED
    // left and right eyes have different matrices
    if( uv.x >= 0.5 )
        return computeTaa(currentTex, prevTex, uv, texSize, halton, texDepth, nearFar, invViewRight, currentFrameProjViewRight, lastFrameProjViewRight, cornersRight, alpha, true);
    else
        return computeTaa(currentTex, prevTex, uv, texSize, halton, texDepth, nearFar, invViewLeft, currentFrameProjViewLeft, lastFrameProjViewLeft, cornersLeft, alpha, true);
#else
#ifdef TAA_SPLIT_SCREEN
    if( uv.x > 0.5015 ) {
        return supersample(currentTex, prevTex, uv, texSize, halton);
    }
    else if( uv.x > 0.4985 )
        return vec3( 0.0 );
    else
        return computeTaa(currentTex, prevTex, uv, texSize, halton, texDepth, nearFar, invViewLeft, currentFrameProjViewLeft, lastFrameProjViewLeft,
            cornersLeft, alpha, true);
#endif
    return computeTaa(currentTex, prevTex, uv, texSize, halton, texDepth, nearFar, invViewLeft, currentFrameProjViewLeft, lastFrameProjViewLeft,
            cornersLeft, alpha, true);
#endif
}

void main() {
// vars

vec3 tmp_0; float tmp_4 = 1.0; vec3 tmp_5;

// end vars

tmp_0 = fxaa( Texture0, vTexCoord0, RenderSize );

if(uOutputLinear == 1 ) {
tmp_5= tmp_0;
} else {
tmp_5 = linearTosRGB( tmp_0 );
}
gl_FragColor = vec4( tmp_5.rgb, tmp_4 );
}
#define SHADER_NAME FinalPass
