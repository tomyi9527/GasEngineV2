#version 100
#extension GL_OES_standard_derivatives : enable
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME Matcap(DE_Lingerie00_Body)


uniform float uMatcapCurvature;
uniform float uMatcapFactor;
uniform float uRGBMRange;
uniform int uOutputLinear;
uniform mat4 uProjectionMatrix;
uniform sampler2D Texture0;
uniform vec3 uMatcapColor;

varying vec3 vViewNormal;
varying vec4 vViewVertex;


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

float adjustSpecular( const in float specular, const in vec3 normal ) {
    // Based on The Order : 1886 SIGGRAPH course notes implementation (page 21 notes)
    float normalLen = length(normal);
    if ( normalLen < 1.0) {
        float normalLen2 = normalLen * normalLen;
        float kappa = ( 3.0 * normalLen -  normalLen2 * normalLen )/( 1.0 - normalLen2 );
        // http://www.frostbite.com/2014/11/moving-frostbite-to-pbr/
        // page 91 : they use 0.5/kappa instead
        return 1.0-min(1.0, sqrt( (1.0-specular) * (1.0-specular) + 1.0/kappa ));
    }
    return specular;
}

vec3 mtexNspaceTangent(const in vec4 tangent, const in vec3 normal, const in vec3 texnormal) {
    vec3 tang = vec3(0.0,1.0,0.0);
    float l = length(tangent.xyz);
    if (l != 0.0) {
        //normalize reusing length computations
        // tang =  normalize(tangent.xyz);
        tang =  tangent.xyz / l;
    }
    vec3 B = tangent.w * normalize(cross(normal, tang));
    return normalize( texnormal.x*tang + texnormal.y*B + texnormal.z*normal);
}

vec2 normalMatcap(const in vec3 normal, const in vec3 nm_z) {
    vec3 nm_x = vec3(-nm_z.z, 0.0, nm_z.x);
    vec3 nm_y = cross(nm_x, nm_z);
    return vec2(dot(normal.xz, nm_x.xz), dot(normal, nm_y)) * vec2(0.5)  + vec2(0.5) ; //MADD vector form
}

vec3 rgbToNormal(const in vec3 texel, const in int flipNormalY) {
    vec3 rgb = texel * vec3(2.0) + vec3(-1.0); // MADD vec form
    rgb[1] = flipNormalY == 1 ? -rgb[1] : rgb[1];
    return rgb;
}

vec3 bumpMap(const in vec4 tangent, const in vec3 normal, const in vec2 gradient) {
    vec3 outnormal;
    float l = length(tangent.xyz);
    if (l != 0.0) {
        //normalize reusing length computations
        // vec3 tang =  normalize(tangent.xyz);
        vec3 tang =  tangent.xyz / l;
        vec3 binormal = tangent.w * normalize(cross(normal, tang));
        outnormal = normal + gradient.x * tang + gradient.y * binormal;
    }
    else {
       outnormal = vec3(normal.x + gradient.x, normal.y + gradient.y, normal.z);
    }
    return normalize(outnormal);
}


// http://madebyevan.com/shaders/curvature/
vec3 applyCurvature(const in vec3 viewNormal, const in vec3 baseColor, const in float str) {
  if(str == 0.0)
    return baseColor;

#ifndef GL_OES_standard_derivatives
    return baseColor;
#else
  vec3 n = normalize(viewNormal);
  // Compute curvature
  vec3 dx = dFdx(n);
  vec3 dy = dFdy(n);
  vec3 xneg = n - dx;
  vec3 xpos = n + dx;
  vec3 yneg = n - dy;
  vec3 ypos = n + dy;

  float curvature = (cross(xneg, xpos).y - cross(yneg, ypos).x) * str;

  float curvDirt = clamp(-curvature * 1.5, 0.0, 1.0);
  float curvEdge = clamp(curvature * 2.5, 0.0, 1.0);

  float factorDirt = mix(1.0, 0.3, curvDirt);
  float factorEdge = 2.0;

  return baseColor * mix(factorDirt, factorEdge, curvEdge);
#endif
}



void main() {
// vars

vec3 channelMatcap; vec2 uvMatcap; vec3 geoNormal; vec3 nFrontViewNormal; vec3 frontViewNormal; vec3 tmp_7; vec3 tmp_8; vec3 eyeVector; float tmp_11 = -1.0; vec3 tmp_12; vec3 tmp_13; const vec3 vec3White = vec3(1.0); vec3 tmp_16; float tmp_18; vec3 tmp_21; vec3 tmp_22;

// end vars

frontViewNormal = gl_FrontFacing ? vViewNormal : -vViewNormal ;
nFrontViewNormal = normalize( frontViewNormal );

geoNormal = nFrontViewNormal.rgb;
tmp_8 = vViewVertex.rgb;
tmp_7 = normalize( tmp_8 );

eyeVector = tmp_7.rgb*tmp_11;
uvMatcap = normalMatcap( geoNormal, eyeVector );

tmp_12 = texture2D(Texture0, uvMatcap).rgb;
tmp_13 = sRGBToLinear( tmp_12 );

channelMatcap = mix(vec3White, tmp_13, uMatcapFactor);
tmp_16 = channelMatcap.rgb*uMatcapColor.rgb;
tmp_18 = uMatcapCurvature / (length(vViewVertex.xyz) * atan(-1.0 / uProjectionMatrix[2][3]));
tmp_21 = applyCurvature( geoNormal, tmp_16, tmp_18 );

if(uOutputLinear == 1 ) {
tmp_22= tmp_21;
} else {
tmp_22 = linearTosRGB( tmp_21 );
}
gl_FragColor = encodeRGBM( tmp_22, uRGBMRange );

}