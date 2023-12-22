#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME Matcap15


uniform float uRGBMRange;
uniform int uOutputLinear;
uniform sampler2D Texture11;

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




void main() {
// vars

vec3 geoNormal; vec3 normal; vec3 frontNormal; vec2 tmp_5; vec3 tmp_6; vec3 tmp_7; vec3 eyeVector; float tmp_10 = -1.0; vec3 tmp_11; vec3 tmp_12; vec3 tmp_13;

// end vars

frontNormal = gl_FrontFacing ? vViewNormal : -vViewNormal ;
normal = normalize( frontNormal );

geoNormal = normal.rgb;
tmp_7 = vViewVertex.rgb;
tmp_6 = normalize( tmp_7 );

eyeVector = tmp_6.rgb*tmp_10;
tmp_5 = normalMatcap( geoNormal, eyeVector );

tmp_11 = textureRGB( Texture11, tmp_5.xy );

tmp_12 = sRGBToLinear( tmp_11 );

if(uOutputLinear == 1 ) {
tmp_13= tmp_12;
} else {
tmp_13 = linearTosRGB( tmp_12 );
}
gl_FragColor = encodeRGBM( tmp_13, uRGBMRange );

}