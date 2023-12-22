
varying vec2 texcoord0;

uniform int outputLinear;
uniform float glowThreshold;
uniform sampler2D diffuse;
uniform sampler2D TextureDepth;

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
	//max value = pow(range, 2.2). 2.2 is gamma correction. If range equals 7.0, then the maximum value is 72.31.
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
	else if (contr < 0.0)
		col = (col - 0.5) * (1.0 + contr) + 0.5;

	// simple reinhard and filmic
	// http://filmicgames.com/archives/190
	if (method == 1) { // reinhard (on luminance)
		col /= 1.0 + getLuminance(col);
	}
	else if (method == 2) { // filmic
		vec3 x = max(vec3(0.0), col - 0.004);
		col = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
		col = pow(col, vec3(2.2)); // filmic curve already encode srgb to linear
	}
	return col;
}

// ------- COLOR BALANCE ---------

vec3 colorBalance(const in vec3 color, const in vec3 lrgb, const in vec3 mrgb, const in vec3 hrgb) {
	float cLen = length(color);
	if (cLen < 1e-5)
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

vec3 extractBright(const in vec3 color, const in float threshold) 
{
	// TODO manage hdr pixel with high frequency? (use derivative??), for now we clamp the extracted pixel :(
	return clamp(color * clamp(getLuminance(color) - threshold, 0.0, 1.0), 0.0, 1.0);
}

vec3 extractBright(const in vec3 color, const in float threshold, const in float alpha) {
	return alpha == 0.0 ? vec3(0.0) : extractBright(color * alpha, threshold);
}

// ------- VIGNETTE ---------

float vignetteDithered(const in vec2 uv, in vec2 lens) {
	// smoothesp is not realiable with edge0 == edge1
	lens.y = min(lens.y, lens.x - 1e-4);

	// same nrand as dof and noise, a blue noise might be better
	vec3 p3 = fract(uv.xyx * 443.8975);
	p3 += dot(p3, p3.yzx + 19.19);
	float jitter = fract((p3.x + p3.y) * p3.z);

	// jitter a bit the uv to remove the banding in some cases
	// (lens.x - lens.y) reduce flickering when the vignette is harder (hardness)
	// (lens.x + lens.y) reduce the flickering when the vignette has a strong radius
	jitter = (lens.x - lens.y) * (lens.x + lens.y) * 0.03 * (jitter - 0.5);
	return smoothstep(lens.x, lens.y, jitter + distance(uv, vec2(0.5)));
}

vec3 vignette(const in vec3 color, const in vec2 uv, const in vec2 lens) {
	return color.rgb * vignetteDithered(uv, lens);
}

// when applied in the aa/ss pass srgb (no background case, also for performance reason if vignette is the ONLY mergeable postprocess)
// there is a colorspace conversion to match the exact same vignette behaviour as with the other vignette (mergeable pass)
vec4 vignette(const in vec4 color, const in vec2 uv, const in vec2 lens) {
	float factor = vignetteDithered(uv, lens);
	return vec4(linearTosRGB(sRGBToLinear(color.rgb) * factor), clamp(color.a + (1.0 - factor), 0.0, 1.0));
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

vec3 sharpen(const in vec3 color, const in sampler2D tex, const in vec2 uv, const in vec2 texSize, const in float sharp, const in float pixelRatio) {
	vec4 sw = pixelRatio * vec4(-1.0, 1.0, 1.0, -1.0) / texSize.xxyy;

	// use useAA, 4 texFetch already in cache. perhaps we could cache ourself.
	vec3 rgbNW = texture2D(tex, uv + sw.xw).rgb;
	vec3 rgbNE = texture2D(tex, uv + sw.yw).rgb;
	vec3 rgbSW = texture2D(tex, uv + sw.xz).rgb;
	vec3 rgbSE = texture2D(tex, uv + sw.yz).rgb;

	return color.rgb + sharp * (4.0 * color.rgb - rgbNW - rgbNE - rgbSW - rgbSE);
}

vec3 sharpen(const in vec3 color, const in sampler2D tex, const in vec2 uv, const in vec2 texSize, const in float sharp, const in float pixelRatio, const in float alpha) {
	if (alpha == 0.0) return color;
	return sharpen(color, tex, uv, texSize, sharp * alpha, pixelRatio);
}

// ------- COMPOSITION ---------

vec4 composeExtra(const in vec3 color, const in float alpha, const in vec4 extra) {
	vec4 outCol = extra + vec4(color.rgb, alpha) * (1.0 - extra.a);
	return outCol;
}

vec3 composeBackgroundColor(const in vec3 color, const in float alpha, const in vec3 background) {
	return color + background * (1.0 - alpha);
}

void main() 
{
	//float tmp_5 = 7e+0;
	float tmp_5 = 0.0;

	vec4 tmp_3 = textureRGBA(diffuse, texcoord0);

	vec3 tmp_4 = decodeRGBM(tmp_3, tmp_5);

	vec4 tmp_6 = vec4(tmp_4.rgb, 1.0);

	//float alpha = textureAlpha(TextureDepth, vTexCoord0.xy);
	float alpha = 1.0;
	vec3 tmp_0 = extractBright(tmp_6.rgb, glowThreshold, alpha);

	vec3 tmp_11;
	if (outputLinear == 1) 
	{
		tmp_11 = tmp_0;
	}
	else 
	{
		tmp_11 = linearTosRGB(tmp_0);
	}

	gl_FragColor = encodeRGBM(tmp_11, 1e+0);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}