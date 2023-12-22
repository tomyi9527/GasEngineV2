#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif



uniform int uOutputLinear;
uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform sampler2D TextureExtra;
uniform vec2 RenderSize;
uniform vec4 uHalton;

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

// access a linear texture as if a nearest texture
vec4 textureNearest2D(const in sampler2D tex, const in vec2 uv, const in vec2 texSize)
{
  return texture2D(tex, (floor(uv*texSize)+0.5) / texSize, -99999.0);
}

// static in place amortized supersampling
vec3 supersample(const in vec3 currFragColor,  const in sampler2D tex, const in vec2 uv, const in vec2 texSize, const in vec4 halton)
{
    float haltz = abs(halton.z);
    if ( haltz == 1.0 ){
         // jut return the color.
        return currFragColor.rgb;
    }

    // same pixel for same projected interpolated vertex on prev frame
    vec3 accumColorN = textureNearest2D(tex, uv.xy, texSize).rgb;

    // mix frames
    float lerpFac = 1.0 / halton.w;

    if ( haltz == 3.0 ){
        // No supersample, crude motion blur by accumulation
       return mix( currFragColor, accumColorN, lerpFac);
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

vec3 textureRGB(const in sampler2D texture, const in vec2 uv) {
    return texture2D(texture, uv.xy ).rgb;
}

vec4 textureRGBA(const in sampler2D texture, const in vec2 uv) {
    return texture2D(texture, uv.xy ).rgba;
}

float textureIntensity(const in sampler2D texture, const in vec2 uv) {
    return texture2D(texture, uv).r;
}

float textureAlpha(const in sampler2D texture, const in vec2 uv) {
    return texture2D(texture, uv.xy ).a;
}



float getLuminance(const in vec3 color) {
    // http://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color
    const vec3 colorBright = vec3(0.2126, 0.7152, 0.0722);
    return dot(color, colorBright);
}

float RGBToL(const in vec3 color) {
    float fmin = min(min(color.r, color.g), color.b);
    float fmax = max(max(color.r, color.g), color.b);
    return (fmax + fmin) / 2.0;
}

// ------- TONE MAPPING ---------

vec3 toneMapping(const in vec3 color, const in float expos, const in float bright, const in float contr, const in float satur, const in int method) {
    vec3 col = color * expos;
    float luminance = dot(col * (1.0 + bright), vec3(0.2126, 0.7152, 0.0722));
    col = mix(vec3(luminance), col * (1.0 + bright), vec3(satur));
    if (contr > 0.0)
        col = (col - 0.5) / (1.0 - contr) + 0.5;
    else if(contr < 0.0)
        col = (col - 0.5) * (1.0 + contr) + 0.5;

    // simple reinhard and filmic
    // http://filmicgames.com/archives/190
    if(method == 1) { // reinhard (on luminance)
        col /= 1.0 + getLuminance(col);
    } else if( method == 2) { // filmic
        vec3 x = max(vec3(0.0), col - 0.004);
        col = (x * (6.2 * x + 0.5) ) / ( x * (6.2 * x + 1.7) + 0.06);
        col = pow(col, vec3(2.2)); // filmic curve already encode srgb to linear
    }
    return col;
}

// ------- COLOR BALANCE ---------

vec3 colorBalance(const in vec3 color, const in vec3 lrgb, const in vec3 mrgb, const in vec3 hrgb) {
    float cLen = length(color);
    if(cLen < 1e-5)
        return color;

    // TODO : I have no idea what I am doing, we should use a curve widge
    float lightness = RGBToL(color);
    // '  lightness=(lightness*0.9-1.0)/(lightness*2.0+1.0) +1.0;
    lightness = (1.5 * lightness) / (lightness + 0.5);
    // https://github.com/liovch/GPUImage/blob/master/framework/Source/GPUImageColorBalanceFilter
    float a = 0.25;
    float b = 0.333;
    float scale = 0.7;
    vec3 low = lrgb * clamp((lightness - b) / -a + 0.5, 0.0, 1.0);
    vec3 mid = mrgb * clamp((lightness - b) / a + 0.5, 0.0, 1.0) * clamp((lightness + b - 1.0) / -a + 0.5, 0.0, 1.0);
    vec3 high = hrgb * clamp((lightness + b - 1.0) / a + 0.5, 0.0, 1.0);
    vec3 newColor = max(color + (low + mid + high) * scale, vec3(0.0));
    // newColor can be negative
    float len = length(newColor);
    return len < 1e-5 ? newColor * cLen : newColor * cLen / len;
}

// ------- BLOOM ---------

vec3 extractBright(const in vec3 color, const in float threshold) {
    // TODO manage hdr pixel with high frequency? (use derivative??), for now we clamp the extracted pixel :(
    return clamp(color * clamp(getLuminance(color) - threshold, 0.0, 1.0), 0.0, 1.0);
}

vec3 extractBright(const in vec3 color, const in float threshold, const in float alpha) {
    return alpha == 0.0 ? vec3(0.0) : extractBright(color * alpha, threshold);
}

// ------- VIGNETTE ---------

vec3 vignette(const in vec3 color, const in vec2 uv, const in vec2 lens) {
    float factor = clamp(smoothstep(lens.x, lens.y, distance(uv, vec2(0.5))), 0.0, 1.0);
    return color.rgb * factor;
}

// when applied in the aa/ss pass srgb (no background case, also for performance reason if vignette is the ONLY mergeable postprocess)
// there is a colorspace conversion to match the exact same vignette behaviour as with the other vignette (mergeable pass)
vec4 vignette(const in vec4 color, const in vec2 uv, const in vec2 lens) {
    float factor = clamp(smoothstep(lens.x, lens.y, distance(uv, vec2(0.5))), 0.0, 1.0);
    return vec4( linearTosRGB(sRGBToLinear(color.rgb) * factor), clamp(color.a + (1.0 - factor), 0.0, 1.0));
}

// ------- CHROMA ---------

vec3 chromaticAberration(const in sampler2D tex, const in vec2 uv, const in float factor, const in float range) {
    vec2 dist = uv - 0.5;
    vec2 offset = factor * dist * length(dist);
    vec3 col;
    col.r = decodeRGBM(texture2D(tex, uv - offset), range).r;
    col.g = decodeRGBM(texture2D(tex, uv), range).g;
    col.b = decodeRGBM(texture2D(tex, uv + offset), range).b;
    return col;
}

// ------- SHARPEN ---------

vec3 sharpen(const in vec3 color, const in sampler2D tex,  const in vec2 uv, const in vec2 texSize, const in float sharp, const in float pixelRatio) {
    vec4 sw = pixelRatio * vec4(-1.0, 1.0, 1.0, -1.0) / texSize.xxyy;

    // use useAA, 4 texFetch already in cache. perhaps we could cache ourself.
    vec3 rgbNW = texture2D(tex, uv + sw.xw).rgb;
    vec3 rgbNE = texture2D(tex, uv + sw.yw).rgb;
    vec3 rgbSW = texture2D(tex, uv + sw.xz).rgb;
    vec3 rgbSE = texture2D(tex, uv + sw.yz).rgb;

    return color.rgb + sharp * (4.0 * color.rgb - rgbNW - rgbNE - rgbSW - rgbSE);
}

vec3 sharpen(const in vec3 color, const in sampler2D tex,  const in vec2 uv, const in vec2 texSize, const in float sharp, const in float pixelRatio, const in float alpha) {
    if(alpha == 0.0) return color;
    return sharpen(color, tex, uv, texSize, sharp * alpha, pixelRatio);
}

// ------- COMPOSITION ---------

vec4 composeExtra(const in vec3 color, const in float alpha, const in vec4 extra) {
    return extra + vec4(color.rgb, alpha) * (1.0 - extra.a);
}

vec3 composeBackgroundColor(const in vec3 color, const in float alpha, const in vec3 background) {
    return color + background * (1.0 - alpha);
}



void main() {
// vars

vec3 tmp_0; float tmp_4 = 1.0; vec4 tmp_5; vec4 tmp_7; vec3 tmp_8; vec3 tmp_11;

// end vars

tmp_0 = fxaa( Texture0, vTexCoord0, RenderSize );

tmp_7 = textureRGBA( TextureExtra, vTexCoord0.xy );

tmp_5 = composeExtra( tmp_0, tmp_4, tmp_7 );

tmp_8 = supersample( tmp_5.rgb, Texture1, vTexCoord0, RenderSize, uHalton );

if(uOutputLinear == 1 ) {
tmp_11= tmp_8;
} else {
tmp_11 = linearTosRGB( tmp_8 );
}
gl_FragColor = vec4( tmp_11.rgb, tmp_5.a );
}
#define SHADER_NAME Antialias
