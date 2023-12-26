uniform float glowIntensity;
uniform float glowRadius;
uniform int outputLinear;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;

varying vec2 texcoord0;

vec3 textureRGB(const in sampler2D tex, const in vec2 uv) 
{
	return texture2D(tex, uv.xy).rgb;
}

vec4 textureRGBA(const in sampler2D tex, const in vec2 uv) 
{
	return texture2D(tex, uv.xy).rgba;
}

float textureIntensity(const in sampler2D tex, const in vec2 uv) 
{
	return texture2D(tex, uv).r;
}

float textureAlpha(const in sampler2D tex, const in vec2 uv) 
{
	return texture2D(tex, uv.xy).a;
}

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
vec3 RGBMToRGB(const in vec4 rgba) 
{
	const float maxRange = 8.0;
	return rgba.rgb * maxRange * rgba.a;
}

const mat3 LUVInverse = mat3(
	6.0013, -2.700, -1.7995,
	-1.332, 3.1029, -5.7720,
	0.3007, -1.088, 5.6268);

vec3 LUVToRGB(const in vec4 vLogLuv) 
{
	float Le = vLogLuv.z * 255.0 + vLogLuv.w;
	vec3 Xp_Y_XYZp;
	Xp_Y_XYZp.y = exp2((Le - 127.0) / 2.0);
	Xp_Y_XYZp.z = Xp_Y_XYZp.y / vLogLuv.y;
	Xp_Y_XYZp.x = vLogLuv.x * Xp_Y_XYZp.z;
	vec3 vRGB = LUVInverse * Xp_Y_XYZp;
	return max(vRGB, 0.0);
}

// http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec4 encodeRGBM(const in vec3 col, const in float range) 
{
	if (range <= 0.0)
	{
		return vec4(col, 1.0);
	}
	vec4 rgbm;
	vec3 color = col / range;
	rgbm.a = clamp(max(max(color.r, color.g), max(color.b, 1e-6)), 0.0, 1.0);
	rgbm.a = ceil(rgbm.a * 255.0) / 255.0;
	rgbm.rgb = color / rgbm.a;
	return rgbm;
}

vec3 decodeRGBM(const in vec4 col, const in float range) 
{
	if (range <= 0.0)
	{
		return col.rgb;
	}
	return range * col.rgb * col.a;
}

void main()
{
	//vec4 encodedColor0 = textureRGBA(texture0, texcoord0);
	//vec3 tmp_0 = decodeRGBM(encodedColor0, 7.0); //main scene color encode with 7.0 range, which can achieve a hdr range of pow(7.0, 2.4)
	//vec4 color0 = vec4(tmp_0, 1.0);

	vec4 encodedColor1 = textureRGBA(texture1, texcoord0);
	vec3 tmp_1 = decodeRGBM(encodedColor1, 1.0); //bloom color encode with 1.0 range.
	vec4 color1 = vec4(tmp_1, 1.0);
	float tmp_2 = mix(1.1, 0.1, glowRadius);
	vec4 tmp_3 = color1.rgba*tmp_2;

	vec4 tmp_16 = textureRGBA(texture2, texcoord0);
	vec3 tmp_17 = decodeRGBM(tmp_16, 1.0);
	vec4 tmp_19 = vec4(tmp_17.rgb, 1.0);
	float tmp_14 = mix(0.9, 0.3, glowRadius);
	vec4 tmp_21 = tmp_19.rgba*tmp_14;

	vec4 tmp_26 = textureRGBA(texture3, texcoord0);
	vec3 tmp_27 = decodeRGBM(tmp_26, 1.0);
	vec4 tmp_29 = vec4(tmp_27.rgb, 1.0);
	float tmp_24 = mix(0.6, 0.6, glowRadius);
	vec4 tmp_31 = tmp_29.rgba*tmp_24;

	vec4 tmp_36 = textureRGBA(texture4, texcoord0);
	vec3 tmp_37 = decodeRGBM(tmp_36, 1.0);
	vec4 tmp_39 = vec4(tmp_37.rgb, 1.0);
	float tmp_34 = mix(0.3, 0.9, glowRadius);
	vec4 tmp_41 = tmp_39.rgba*tmp_34;

	vec4 tmp_46 = textureRGBA(texture5, texcoord0);
	vec3 tmp_47 = decodeRGBM(tmp_46, 1.0);
	vec4 tmp_49 = vec4(tmp_47.rgb, 1.0);
	float tmp_44 = mix(0.1, 1.1, glowRadius);
	vec4 tmp_51 = tmp_49.rgba*tmp_44;

	vec4 tmp_52 = tmp_3.rgba + tmp_21.rgba + tmp_31.rgba + tmp_41.rgba + tmp_51.rgba;
	//vec4 tmp_52 = tmp_3.rgba + tmp_51.rgba;
	//vec4 tmp_53 = color0 + (tmp_52 * bloomFactor);
	vec4 tmp_53 = tmp_52*glowIntensity;

	vec3 tmp_61;
	if (outputLinear == 1)
	{
		tmp_61.rgb = tmp_53.rgb;
	}
	else
	{
		tmp_61.rgb = linearTosRGB(tmp_53.rgb);
	}

	//gl_FragColor = vec4(tmp_61.rgb, color0.a);
	gl_FragColor = vec4(tmp_61.rgb, 1.0);
}