#if (SHADERKEY&(1 << 0))
	#define MATCAP
#endif

#if (SHADERKEY&(1 << 1))
	#define MATCAPMAP
#endif

varying vec4 fragmentColor;
varying vec2 fragmentTexCoord0;
varying vec2 fragmentTexCoord1;
varying vec3 fragmentNormal;
varying vec4 fragmentTangent;
varying vec3 fragmentVextexViewPosition;

#ifdef MATCAP
	#ifdef MATCAPMAP
		uniform sampler2D matCapMap;
	#endif
	uniform float matCapCurvature;
	uniform vec3 matCapColor;
#endif

#ifdef NORMAL
	#ifdef NORMALMAP
		uniform sampler2D normalMap;
	#endif
	uniform float normalFactor;
	uniform int normalFlipY;
#endif

#ifdef TRANSPARENCY
	#ifdef TRANSPARENCYMAP
		uniform sampler2D transparencyMap;
		uniform vec4 transparencyChannel;
	#endif
    //uniform int drawOpaquePass;
	uniform float transparencyFactor;
	uniform int transparencyAlphaInvert;
#endif

uniform float rgbmRange;
uniform int outputLinear;

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
float linearTosRGB(const in float c) 
{
    return LIN_SRGB(c);
}

vec3 linearTosRGB(const in vec3 c) 
{
    return vec3(LIN_SRGB(c.r), LIN_SRGB(c.g), LIN_SRGB(c.b));
}

vec4 linearTosRGB(const in vec4 c) 
{
    return vec4(LIN_SRGB(c.r), LIN_SRGB(c.g), LIN_SRGB(c.b), c.a);
}

#define SRGB_LIN(x) x < 0.04045 ? x * (1.0 / 12.92) : pow((x + 0.055) * (1.0 / 1.055), 2.4)

float sRGBToLinear(const in float c) 
{
    return SRGB_LIN(c);
}

vec3 sRGBToLinear(const in vec3 c) 
{
    return vec3(SRGB_LIN(c.r), SRGB_LIN(c.g), SRGB_LIN(c.b));
}

vec4 sRGBToLinear(const in vec4 c) 
{
    return vec4(SRGB_LIN(c.r), SRGB_LIN(c.g), SRGB_LIN(c.b), c.a);
}

//http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec3 RGBMToRGB( const in vec4 rgba ) 
{
    const float maxRange = 8.0;
    return rgba.rgb * maxRange * rgba.a;
}

// http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec4 encodeRGBM(const in vec3 col, const in float range) 
{
    if(range <= 0.0)
    {
        return vec4(col, 1.0);
    }

    vec4 rgbm;
    vec3 color = col / range;
    rgbm.a = clamp( max( max( color.r, color.g ), max( color.b, 1e-6 ) ), 0.0, 1.0 );
    rgbm.a = ceil( rgbm.a * 255.0 ) / 255.0;
    rgbm.rgb = color / rgbm.a;
    return rgbm;
}

vec3 decodeRGBM(const in vec4 col, const in float range) 
{
    if(range <= 0.0)
        return col.rgb;
    return range * col.rgb * col.a;
}

vec2 normalMatcap(const in vec3 normal, const in vec3 eyeVector) 
{
    vec3 nm_x = vec3(-eyeVector.z, 0.0, eyeVector.x);
    vec3 nm_y = cross(nm_x, eyeVector);
    return vec2(dot(normal.xz, nm_x.xz), dot(normal, nm_y)) * vec2(0.5)  + vec2(0.5) ; //MADD vector form
}

void main() 
{
#ifdef MATCAP
    #ifdef MATCAPMAP
        vec3 frontNormal = gl_FrontFacing ? fragmentNormal : -fragmentNormal ;
        frontNormal = normalize(frontNormal);
        vec3 eyeVector = -1.0 * normalize(fragmentVextexViewPosition);
        vec2 matcapUV = normalMatcap(frontNormal, eyeVector);
        vec3 srgbMatColor = texture2D(matCapMap, matcapUV).rgb;
        vec3 linearMatColor = sRGBToLinear(srgbMatColor) * matCapColor;
    #else
        vec3 linearMatColor = sRGBToLinear(matCapColor);
    #endif
#else
    vec3 linearMatColor = vec3(1, 0, 0);
#endif

    vec3 finalColor;
    if(outputLinear == 1) 
    {
        finalColor = linearMatColor;
    } 
    else
    {
        finalColor = linearTosRGB(linearMatColor);
    }

    gl_FragColor = encodeRGBM(finalColor, rgbmRange);
}