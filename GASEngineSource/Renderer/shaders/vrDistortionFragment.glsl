uniform int uOutputLinear;		// 0/1
uniform sampler2D Texture0;
uniform sampler2D TextureExtra;
uniform vec2 uDistortion;      //[0.441 0.156]
uniform vec4 uProjectionLeft;	//[0.6497724, 0.5958768, -0.45477623, -0.5]
uniform vec4 uUnprojectionLeft; //[0.76368654, 0.67800343, -0.45368776, -0.5]

varying vec2 texcoord0;


// https://github.com/borismus/webvr-boilerplate/blob/master/src/distortion/barrel-distortion-fragment-v2.js

vec2 barrel(const in vec2 v, const in vec4 projection, const in vec4 unprojection, const in vec2 distort) {
	vec2 w = (v + unprojection.zw) / unprojection.xy;
	float val = dot(w, w);
	// float poly = (showCenter == 1 && val < 0.00010) ? 10000.0 : 1.0 + (distort.x + distort.y * val) * val;
	float poly = 1.0 + (distort.x + distort.y * val) * val;
	return projection.xy * (poly * w) - projection.zw;
}

vec2 getDistortUV(const in sampler2D tex, const in vec2 texCoord, const in vec2 distort, const in vec4 projectionLeft, const in vec4 unprojectionLeft) {

	if (abs(texCoord.x - 0.5) < .001) {
		return vec2(-1.0);
	}

	vec2 a;

	if (texCoord.x < 0.5) {
		a = barrel(vec2(texCoord.x / 0.5, texCoord.y), projectionLeft, unprojectionLeft, distort);
	}
	else {
		// right projections are shifted and vertically mirrored relative to left
		const vec3 swi = vec3(1.0, -1.0, 0.0);
		vec4 projectionRight = (projectionLeft + swi.zzxz) * swi.xxyx;
		vec4 unprojectionRight = (unprojectionLeft + swi.zzxz) * swi.xxyx;
		a = barrel(vec2((texCoord.x - 0.5) / 0.5, texCoord.y), projectionRight, unprojectionRight, distort);
	}

	if (a.x < 0.0 || a.x > 1.0 || a.y < 0.0 || a.y > 1.0) {
		return vec2(-1.0);
	}

	return vec2(a.x * 0.5 + (texCoord.x < 0.5 ? 0.0 : 0.5), a.y);
}

vec3 distortion(const in sampler2D tex, const in vec2 texCoord, const in vec2 distort, const in vec4 projectionLeft, const in vec4 unprojectionLeft, const in sampler2D texExtra) {
	vec2 distortUV = getDistortUV(tex, texCoord, distort, projectionLeft, unprojectionLeft);
	if (distortUV.x < 0.0) return vec3(0.0);

	vec3 color = texture2D(tex, distortUV).rgb;
	vec4 extra = texture2D(texExtra, distortUV);
	return extra.rgb + color.rgb * (1.0 - extra.a);
}

vec3 distortion(const in sampler2D tex, const in vec2 texCoord, const in vec2 distort, const in vec4 projectionLeft, const in vec4 unprojectionLeft) {
	vec2 distortUV = getDistortUV(tex, texCoord, distort, projectionLeft, unprojectionLeft);
	if (distortUV.x < 0.0) return vec3(0.0);
	return texture2D(tex, distortUV).rgb;
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
vec3 RGBMToRGB(const in vec4 rgba) {
	const float maxRange = 8.0;
	return rgba.rgb * maxRange * rgba.a;
}

const mat3 LUVInverse = mat3(6.0013, -2.700, -1.7995,
	-1.332, 3.1029, -5.7720,
	0.3007, -1.088, 5.6268);

vec3 LUVToRGB(const in vec4 vLogLuv) {
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
	if (range <= 0.0)
		return vec4(col, 1.0);
	vec4 rgbm;
	vec3 color = col / range;
	rgbm.a = clamp(max(max(color.r, color.g), max(color.b, 1e-6)), 0.0, 1.0);
	rgbm.a = ceil(rgbm.a * 255.0) / 255.0;
	rgbm.rgb = color / rgbm.a;
	return rgbm;
}

vec3 decodeRGBM(const in vec4 col, const in float range) {
	if (range <= 0.0)
		return col.rgb;
	return range * col.rgb * col.a;
}
vec3 textureRGB(const in sampler2D texture, const in vec2 uv) {
	return texture2D(texture, uv.xy).rgb;
}

vec4 textureRGBA(const in sampler2D texture, const in vec2 uv) {
	return texture2D(texture, uv.xy).rgba;
}

float textureIntensity(const in sampler2D texture, const in vec2 uv) {
	return texture2D(texture, uv).r;
}

float textureAlpha(const in sampler2D texture, const in vec2 uv) {
	return texture2D(texture, uv.xy).a;
}

void main() {
	// vars

	vec3 tmp_0; vec3 tmp_7; vec4 tmp_9;

	// end vars

	//tmp_0 = distortion(Texture0, texcoord0, uDistortion, uProjectionLeft, uUnprojectionLeft, TextureExtra);
	tmp_0 = distortion(Texture0, texcoord0, uDistortion, uProjectionLeft, uUnprojectionLeft);

	if (uOutputLinear == 1) {
		tmp_7 = tmp_0;
	}
	else {
		tmp_7 = linearTosRGB(tmp_0);
	}
	tmp_9 = textureRGBA(Texture0, texcoord0.xy);

	gl_FragColor = vec4(tmp_7.rgb, tmp_9.a);
}
