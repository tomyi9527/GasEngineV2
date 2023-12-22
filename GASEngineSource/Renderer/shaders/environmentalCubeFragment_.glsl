#extension GL_EXT_shader_texture_lod : enable
precision mediump float;

uniform vec4 cameraFrustum;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform vec3 cameraFront;
uniform vec2 uEnvironmentSize;
uniform float uEnvironmentExposure;
uniform float uBackgroundExposure;
uniform float uOrientation;

varying vec3 texcoord0;

const mat3 LUVInverse = mat3( 6.0013,    -2.700,   -1.7995,
                      -1.332,    3.1029,   -5.7720,
                      0.3007,    -1.088,    5.6268 );
uniform samplerCube environmentalCubeTexture; 

vec3 LUVToRGB( const in vec4 vLogLuv )
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

vec4 textureCubeFixed(const in samplerCube texture, const in vec3 direction )
{
	// http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
	float scale = 1.0 - 1.0 / uEnvironmentSize[0];
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


 void main() 
 { 
	float x = texcoord0.x*cameraFrustum.x*1.0;
	float y = texcoord0.y*cameraFrustum.y*1.0;
	float z = -cameraFrustum.z;
	vec3 pixelVec = normalize(vec3(x,y,z));
	mat3 cameraToWorld;
	cameraToWorld[0] = cameraRight; 
	cameraToWorld[1] = cameraUp; 
	cameraToWorld[2] = cameraFront;
	vec3 uvCube = cameraToWorld * pixelVec;

	vec3 uvCubeRotated = rotate_vertex_position(uvCube, vec3(0, 1, 0), uOrientation);

    vec4 vLogLuv = textureCubeFixed(environmentalCubeTexture, uvCubeRotated).rgba;
    vec3 rgb = LUVToRGB( vLogLuv ) * uEnvironmentExposure * uBackgroundExposure;

    gl_FragColor = vec4( linearTosRGB(rgb, 2.4 ), 0.0);
 }