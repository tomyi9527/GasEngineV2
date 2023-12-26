precision mediump float;

#if (SHADERKEY&0x01)
	#define SOLIDCOLOR
#endif

#if (SHADERKEY&0x02)
	#define IMAGE
#endif

#if (SHADERKEY&0x04)
	#define CUBEMAP
#endif

#if (SHADERKEY&0x08)
	#define AMBIENT
#endif

#if defined(CUBEMAP) || defined(AMBIENT)
	uniform vec4 cameraFrustum;
	uniform vec3 cameraRight;
	uniform vec3 cameraUp;
	uniform vec3 cameraFront;
	uniform float lightExposure;
	uniform float backgroundExposure;
	uniform float orientation;

	#ifdef CUBEMAP
		uniform samplerCube cubeMap;
		uniform float cubeMapSize;
	#endif

	#ifdef AMBIENT
		uniform vec3 sph[9];
	#endif
#endif

#ifdef IMAGE
	uniform sampler2D image; 
	uniform vec2 imageRatio;
#endif

#ifdef SOLIDCOLOR
	uniform vec3 solidColor;
#endif

varying vec3 fragmentTexCoord0;

const mat3 LUVInverse = mat3( 
	6.0013,	-2.700,	-1.7995,
	-1.332,	3.1029,	-5.7720,
	0.3007,	-1.088,	5.6268); 

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

float linearrgb_to_srgb1(const in float c, const in float gamma)
{
    float v = 0.0;
    if(c < 0.0031308) 
    {
        if ( c > 0.0)
        {
            v = c * 12.92;
        } 
    }
    else 
    {
        v = 1.055 * pow(c, 1.0/ gamma) - 0.055;
    }
    return v;
}

vec3 linearTosRGB(const in vec3 col_from, const in float gamma)
{
    vec3 col_to;
    col_to.r = linearrgb_to_srgb1(col_from.r, gamma);
    col_to.g = linearrgb_to_srgb1(col_from.g, gamma);
    col_to.b = linearrgb_to_srgb1(col_from.b, gamma);
    return col_to;
}

vec3 cubemapSeamlessFixDirection(const in vec3 direction, const in float scale )
{
	vec3 dir = direction;
	// http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
	float M = max(max(abs(dir.x), abs(dir.y)), abs(dir.z));

	if (abs(dir.x) != M) dir.x *= scale;
	if (abs(dir.y) != M) dir.y *= scale;
	if (abs(dir.z) != M) dir.z *= scale;

	return dir;
}

vec4 textureCubeFixed(const in samplerCube texture, const in vec3 direction, const in float cubeMapSize)
{
	// http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
	float scale = 1.0 - 1.0 / cubeMapSize;
	vec3 dir = cubemapSeamlessFixDirection(direction, scale);
	return textureCube( texture, dir );
}

vec4 quat_from_axis_angle(vec3 axis, float angle)
{
	vec4 qr;
	float half_angle = (angle * 0.5) * 3.14159 / 180.0;
	qr.x = axis.x * sin(half_angle);
	qr.y = axis.y * sin(half_angle);
	qr.z = axis.z * sin(half_angle);
	qr.w = cos(half_angle);
	return qr;
}

vec4 quat_conj(vec4 q)
{
	return vec4(-q.x, -q.y, -q.z, q.w);
}

vec4 quat_mult(vec4 q1, vec4 q2)
{
	vec4 qr;
	qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
	qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
	qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
	qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
	return qr;
}

vec3 rotate_vertex_position(vec3 position, vec3 axis, float angle)
{
	vec4 qr = quat_from_axis_angle(axis, angle);
	vec4 qr_conj = quat_conj(qr);
	vec4 q_pos = vec4(position.x, position.y, position.z, 0);

	vec4 q_tmp = quat_mult(qr, q_pos);
	qr = quat_mult(q_tmp, qr_conj);

	return vec3(qr.x, qr.y, qr.z);
}

vec3 evaluateDiffuseSphericalHarmonics(const in vec3 s[9], const in vec3 n) {
	// https://github.com/cedricpinson/envtools/blob/master/Cubemap.cpp#L523
	vec3 result = (s[0] + s[1] * n.y + s[2] * n.z + s[3] * n.x + s[4] * n.y*n.x + s[5] * n.y*n.z + s[6] * (3.0*n.z*n.z - 1.0) + s[7] * (n.z*n.x) + s[8] * (n.x*n.x - n.y*n.y));
	return max(result, vec3(0.0));
}

 void main() 
 { 
#if defined(CUBEMAP) || defined(AMBIENT) 
	float x = fragmentTexCoord0.x*cameraFrustum.x*1.0;
	float y = fragmentTexCoord0.y*cameraFrustum.y*1.0;
	float z = -cameraFrustum.z;

	vec3 uvInViewSpace = normalize(vec3(x,y,z));

	mat3 matrixViewToWorld;
	matrixViewToWorld[0] = cameraRight;
	matrixViewToWorld[1] = cameraUp;
	matrixViewToWorld[2] = cameraFront;

	vec3 uvInWorldSpace = matrixViewToWorld * uvInViewSpace;
	vec3 uvWorldRotated = rotate_vertex_position(uvInWorldSpace, vec3(0.0, 1.0, 0.0), orientation);

	#ifdef AMBIENT
		vec3 rgb = evaluateDiffuseSphericalHarmonics(sph, uvWorldRotated);
		rgb *= lightExposure * backgroundExposure;
		gl_FragColor = vec4(linearTosRGB(rgb, 2.4), 0.0);
	#endif

	#ifdef CUBEMAP
		vec4 colorLUV = textureCubeFixed(cubeMap, uvWorldRotated, cubeMapSize).rgba;
		vec3 colorRGB = LUVToRGB(colorLUV) * lightExposure * backgroundExposure;

		gl_FragColor = vec4(linearTosRGB(colorRGB, 2.4), 0.0);
	#endif
#endif

#ifdef IMAGE
	vec2 uvBackgroundImage = (fragmentTexCoord0.xy*0.5)*imageRatio + vec2(0.5, 0.5);
    vec4 colorRGB = texture2D(image, uvBackgroundImage).rgba;
	gl_FragColor = vec4(colorRGB.rgb, 0.0);
#endif

#ifdef SOLIDCOLOR
	gl_FragColor = vec4(solidColor.x, solidColor.y, solidColor.z, 0.0);
#endif    
 }