varying vec2 texcoord0;
uniform vec2 renderSize;
uniform sampler2D diffuse;

vec4 unpack(const in sampler2D tex, const in vec2 uv) 
{
	vec4 col = texture2D(tex, uv);
	return vec4(1e+0 * col.rgb * col.a, 1.0);
}

vec4 pack(const in vec4 col) 
{
	vec4 rgbm;
	vec3 color = col.rgb / 1e+0;
	rgbm.a = clamp(max(max(color.r, color.g), max(color.b, 1e-6)), 0.0, 1.0);
	rgbm.a = ceil(rgbm.a * 255.0) / 255.0;
	rgbm.rgb = color / rgbm.a;
	return rgbm;
}

//vec4 unpack(const in sampler2D tex, const in vec2 uv)
//{
//	vec4 col = texture2D(tex, uv);
//	return vec4(col.rgb, 1.0);
//}
//
//vec4 pack(const in vec4 col)
//{
//	return col;
//}

void main(void)
{
#if (BLUR == 4)

	float weight = 0.375;
	float weight0 = 0.3125;
	float offsetFactor0 = 1.2;

#elif (BLUR == 6)

	float weight = 0.3125;
	float weight0 = 0.328125;
	float offsetFactor0 = 1.2857142857142858;

#elif (BLUR == 8)
	float weight = 0.2734375;
	float weight0 = 0.328125;
	float weight1 = 0.03515625;
	float offsetFactor0 = 1.3333333333333333;
	float offsetFactor1 = 3.111111111111111;

#elif (BLUR == 10)

	float weight = 0.24609375;
	float weight0 = 0.322265625;
	float weight1 = 0.0537109375;
	float offsetFactor0 = 1.3636363636363635;
	float offsetFactor1 = 3.1818181818181817;

#elif (BLUR == 12)

	float weight = 0.2255859375;
	float weight0 = 0.314208984375;
	float weight1 = 0.06982421875;
	float weight2 = 0.003173828125;
	float offsetFactor0 = 1.3846153846153846;
	float offsetFactor1 = 3.230769230769231;
	float offsetFactor2 = 5.076923076923077;
#endif

#ifdef VERTICAL
	vec2 offset = vec2(0.0, offsetFactor0 / renderSize[1]);
#else
	vec2 offset = vec2(offsetFactor0 / renderSize[0], 0.0);
#endif

	vec3 pixel = weight*unpack(diffuse, texcoord0).rgb;
	pixel += weight0*unpack(diffuse, (texcoord0 + offset)).rgb;
	pixel += weight0*unpack(diffuse, (texcoord0 - offset)).rgb;

#if (BLUR == 8) || (BLUR == 10) || (BLUR == 12)
	
	#ifdef VERTICAL
		offset = vec2(0.0, offsetFactor1 / renderSize[1]);
	#else
		offset = vec2(offsetFactor1 / renderSize[0], 0.0);
	#endif

	pixel += weight1*unpack(diffuse, (texcoord0 + offset)).rgb;
	pixel += weight1*unpack(diffuse, (texcoord0 - offset)).rgb;
#endif

#if (BLUR == 12)
	
	#ifdef VERTICAL
		offset = vec2(0.0, offsetFactor2 / renderSize[1]);
	#else
		offset = vec2(offsetFactor2 / renderSize[0], 0.0);
	#endif

	pixel += weight2*unpack(diffuse, (texcoord0 + offset)).rgb;
	pixel += weight2*unpack(diffuse, (texcoord0 - offset)).rgb;
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	//return;
#endif

	gl_FragColor = pack(vec4(pixel, 1.0));
}