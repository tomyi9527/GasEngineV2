#define SHADER_NAME DepthMaterial
#define GAMMA_FACTOR 2
#define MAX_BONES 1024
#define BONE_TEXTURE

uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;

attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;
attribute vec4 skinIndex;
attribute vec4 skinWeight;

#define PI 3.14159
#define PI2 6.28318
#define RECIPROCAL_PI 0.31830988618
#define RECIPROCAL_PI2 0.15915494
#define LOG2 1.442695
#define EPSILON 1e-6
#define saturate(a) clamp( a, 0.0, 1.0 )
#define whiteCompliment(a) ( 1.0 - saturate( a ) )

uniform mat4 bindMatrix;
uniform mat4 bindMatrixInverse;
#ifdef BONE_TEXTURE
uniform sampler2D boneTexture;
uniform int boneTextureWidth;
uniform int boneTextureHeight;
mat4 getBoneMatrix(const in float i) {
	float j = i * 4.0;
	float x = mod(j, float(boneTextureWidth));
	float y = floor(j / float(boneTextureWidth));
	float dx = 1.0 / float(boneTextureWidth);
	float dy = 1.0 / float(boneTextureHeight);
	y = dy * (y + 0.5);
	vec4 v1 = texture2D(boneTexture, vec2(dx * (x + 0.5), y));
	vec4 v2 = texture2D(boneTexture, vec2(dx * (x + 1.5), y));
	vec4 v3 = texture2D(boneTexture, vec2(dx * (x + 2.5), y));
	vec4 v4 = texture2D(boneTexture, vec2(dx * (x + 3.5), y));
	mat4 bone = mat4(v1, v2, v3, v4);
	return bone;
}
#else
uniform mat4 boneGlobalMatrices[MAX_BONES];
mat4 getBoneMatrix(const in float i) {
	mat4 bone = boneGlobalMatrices[int(i)];
	return bone;
}
#endif

varying vec3 osg_FragEye;
varying vec3 osg_FragNormal;
varying vec4 osg_FragTangent;
varying vec2 osg_FragTexCoord0;

void main()
{
	osg_FragTexCoord0 = uv;

	osg_FragEye = vec3(modelViewMatrix * vec4(position, 1.0));

	osg_FragTangent = vec4(1, 0, 0, 0);

	mat4 boneMatX = getBoneMatrix(skinIndex.x);
	mat4 boneMatY = getBoneMatrix(skinIndex.y);
	mat4 boneMatZ = getBoneMatrix(skinIndex.z);
	mat4 boneMatW = getBoneMatrix(skinIndex.w);

	mat4 skinMatrix = mat4(0.0);
	skinMatrix += skinWeight.x * boneMatX;
	skinMatrix += skinWeight.y * boneMatY;
	skinMatrix += skinWeight.z * boneMatZ;
	skinMatrix += skinWeight.w * boneMatW;
	skinMatrix = skinMatrix * bindMatrix;

	vec4 worldNormal = skinMatrix * vec4(normal, 0.0);
	osg_FragNormal = (viewMatrix * worldNormal).xyz;

	vec4 skinVertex = bindMatrix * vec4(position, 1.0);
	vec4 skinned = vec4(0.0);
	skinned += boneMatX * skinVertex * skinWeight.x;
	skinned += boneMatY * skinVertex * skinWeight.y;
	skinned += boneMatZ * skinVertex * skinWeight.z;
	skinned += boneMatW * skinVertex * skinWeight.w;

	vec4 mvPosition = viewMatrix * skinned;

	gl_Position = projectionMatrix * mvPosition;
}