
#if (SHADERKEY&0x01)
	#define POSITION
#endif

#if (SHADERKEY&0x02)
	#define NORMAL
#endif

#if (SHADERKEY&0x04)
	#define TANGENT
#endif

#if (SHADERKEY&0x08)
	#define UV0
#endif

#if (SHADERKEY&0x10)
	#define UV1
#endif

#if (SHADERKEY&0x20)
	#define COLOR0
#endif

#if (SHADERKEY&0x40)
	#define COLOR1
#endif

#if (SHADERKEY&0x80)
	#define SKINNING
#endif

#if (SHADERKEY&0x100)
	#define MORPHPOSITION
#endif

#if (SHADERKEY&0x200)
	#define MORPHNORMAL
#endif

#if (SHADERKEY&0x400)
	#define DEPTHBIAS
#endif

#if (SHADERKEY&0x800)
	#define VERTEXTEXTURE
#endif

#if (SHADERKEY&0x1000)
	#define FLOATTEXTURE
#endif

#if (SHADERKEY&0x2000)
	#define WORLDNORMAL
#endif

#if (SHADERKEY&0x4000)
	#define TAA
#endif

uniform mat4 worldMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;

#ifdef TAA
	uniform vec4 haltonLDS;
	uniform vec2 renderTargetSize;
#endif

#ifdef MORPHPOSITION
	uniform float morphTargetInfluences[4];
#endif

#ifdef SKINNING

	uniform sampler2D boneTexture;
	uniform int boneTextureSize;

	#ifdef FLOATTEXTURE

		mat4 getBoneMatrix(const in float i)
		{
			float j = i * 4.0;
			float x = mod(j, float(boneTextureSize));
			float y = floor(j / float(boneTextureSize));
			float dx = 1.0 / float(boneTextureSize);
			float dy = 1.0 / float(boneTextureSize);
			y = dy * (y + 0.5);
			vec4 v1 = texture2D(boneTexture, vec2(dx * (x + 0.5), y));
			vec4 v2 = texture2D(boneTexture, vec2(dx * (x + 1.5), y));
			vec4 v3 = texture2D(boneTexture, vec2(dx * (x + 2.5), y));
			vec4 v4 = texture2D(boneTexture, vec2(dx * (x + 3.5), y));
			mat4 bone = mat4(v1, v2, v3, v4);
			return bone;
		}

	#else

		float decodeRGBA2Float(float dx, float X, float Y, float index)
		{
			float i0 = index*2.0 + 0.5;
			float i1 = index*2.0 + 1.5;
			vec4 rgba0 = texture2D(boneTexture, vec2(dx * (X + i0), Y));
			vec4 rgba1 = texture2D(boneTexture, vec2(dx * (X + i1), Y));

			float frac_ = dot(rgba0, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/160581375.0));
			float decimal_ = floor(rgba1.r*255.0 + 0.5) + floor(rgba1.g*255.0 + 0.5)*255.0 + floor(rgba1.b*255.0 + 0.5)*65025.0;
			float f = (decimal_ + frac_) * (rgba1.a*2.0 - 1.0);
			return f;
		}

		mat4 getBoneMatrix(const in float i)
		{
			float j = i * 32.0;
			float x = mod(j, float(boneTextureSize));
			float y = floor(j / float(boneTextureSize));
			float dx = 1.0 / float(boneTextureSize);
			float dy = 1.0 / float(boneTextureSize);
			y = dy * (y + 0.5);

			float f00 = decodeRGBA2Float(dx, x, y, 0.0);
			float f01 = decodeRGBA2Float(dx, x, y, 1.0);
			float f02 = decodeRGBA2Float(dx, x, y, 2.0);
			float f03 = decodeRGBA2Float(dx, x, y, 3.0);
										
			float f10 = decodeRGBA2Float(dx, x, y, 4.0);
			float f11 = decodeRGBA2Float(dx, x, y, 5.0);
			float f12 = decodeRGBA2Float(dx, x, y, 6.0);
			float f13 = decodeRGBA2Float(dx, x, y, 7.0);
										
			float f20 = decodeRGBA2Float(dx, x, y, 8.0);
			float f21 = decodeRGBA2Float(dx, x, y, 9.0);
			float f22 = decodeRGBA2Float(dx, x, y, 10.0);
			float f23 = decodeRGBA2Float(dx, x, y, 11.0);
											
			float f30 = decodeRGBA2Float(dx, x, y, 12.0);
			float f31 = decodeRGBA2Float(dx, x, y, 13.0);
			float f32 = decodeRGBA2Float(dx, x, y, 14.0);
			float f33 = decodeRGBA2Float(dx, x, y, 15.0);

			mat4 bone = mat4(		vec4(f00, f01, f02, f03), 
									vec4(f10, f11, f12, f13), 
									vec4(f20, f21, f22, f23), 
									vec4(f30, f31, f32, f33));
			return bone;
		}

	#endif
	
#endif

//Vertex inputs
attribute vec3 position;

#ifdef NORMAL
	attribute vec3 normal;
#endif

#ifdef TANGENT
	attribute vec4 tangent;
#endif

#ifdef UV0
	attribute vec2 uv;
#endif

#ifdef UV1
	attribute vec2 uv2;
#endif

#ifdef COLOR0
	attribute vec4 color;
#endif

#ifdef MORPHPOSITION
	attribute vec3 morphTarget0;
	attribute vec3 morphTarget1;
	attribute vec3 morphTarget2;
	attribute vec3 morphTarget3;	
#endif

#ifdef MORPHNORMAL
	attribute vec3 morphNormal0;
	attribute vec3 morphNormal1;
	attribute vec3 morphNormal2;
	attribute vec3 morphNormal3;
#endif

#ifdef SKINNING
	attribute vec4 skinIndex;
	attribute vec4 skinWeight;
#endif

vec3 morphTransform(const in float weights[4], const in vec3 vertex, const in vec3 target0, const in vec3 target1, const in vec3 target2, const in vec3 target3) 
{
	vec3 vecOut = vertex * (1.0 - (weights[0] + weights[1] + weights[2] + weights[3]));

	vecOut += target0 * weights[0];
	vecOut += target1 * weights[1];
	vecOut += target2 * weights[2];
	vecOut += target3 * weights[3];

	return vecOut;
}

//Output to fragment
varying vec4 fragmentColor;
varying vec2 fragmentTexCoord0;
varying vec2 fragmentTexCoord1;
varying vec3 fragmentNormal;
varying vec4 fragmentTangent;
varying vec3 fragmentVextexViewPosition;
varying vec3 fragmentVextexWorldPosition;

void main() 
{
#ifdef COLOR0
	fragmentColor = color;
#else
	fragmentColor = vec4(1.0, 1.0, 1.0, 1.0);
#endif

#ifdef UV0
	fragmentTexCoord0 = uv;
#else
	fragmentTexCoord0 = vec2(0, 0);
#endif

#ifdef UV1
	fragmentTexCoord1 = uv2;
#else
	fragmentTexCoord1 = vec2(0, 0);
#endif

//TODO: THE NORMAL AND TANGENT WORLD TRANSFORM IS ONLY APPLICABLE FOR UNIFORM SCALE.
#ifdef NORMAL
	vec3 objectNormal = vec3(normal);
	#ifdef MORPHNORMAL
		objectNormal += (morphNormal0 - normal)*morphTargetInfluences[0];
		objectNormal += (morphNormal1 - normal)*morphTargetInfluences[1];
		objectNormal += (morphNormal2 - normal)*morphTargetInfluences[2];
		objectNormal += (morphNormal3 - normal)*morphTargetInfluences[3];
	#endif

	#ifdef SKINNING
		mat4 boneMatX = getBoneMatrix(skinIndex.x);
		mat4 boneMatY = getBoneMatrix(skinIndex.y);
		mat4 boneMatZ = getBoneMatrix(skinIndex.z);
		mat4 boneMatW = getBoneMatrix(skinIndex.w);

		mat4 skinMatrix = mat4(0.0);
		skinMatrix += skinWeight.x*boneMatX;
		skinMatrix += skinWeight.y*boneMatY;
		skinMatrix += skinWeight.z*boneMatZ;
		skinMatrix += skinWeight.w*boneMatW;

		//The matrix which convert position from model to world is different from that for normal when nonuniform scale exist.
		//So the code below could not handle the normal transformation of skinning mesh correctly when nonuniform scale exist,
		//Because this code just simplely replace the normal matrix with the position modelView matrix. So does the tangent transformation.
		vec3 worldNormal = vec4(skinMatrix*vec4(objectNormal, 0.0)).xyz; //World normal
	#endif

	#ifdef WORLDNORMAL
		#ifdef SKINNING
			fragmentNormal = worldNormal;
		#else			
			fragmentNormal = vec4(worldMatrix*vec4(objectNormal, 0.0)).xyz;
		#endif		
	#else
		#ifdef SKINNING
			fragmentNormal = vec4(viewMatrix*vec4(worldNormal, 0.0)).xyz;
		#else			
			fragmentNormal = normalMatrix*objectNormal; //view space
		#endif
	#endif
#else
	fragmentNormal = vec3(0, 1, 0);
#endif

#ifdef TANGENT
	#ifdef SKINNING
		vec3 worldTangent = vec4(skinMatrix*vec4(tangent.xyz, 0.0)).xyz;
	#endif

	#ifdef WORLDNORMAL
		#ifdef SKINNING
			fragmentTangent.xyz = worldTangent;
			fragmentTangent.w = tangent.w; //handness
		#else
			fragmentTangent.xyz = vec4(worldMatrix*vec4(tangent.xyz, 0.0)).xyz;;
			fragmentTangent.w = tangent.w;
		#endif
	#else
		#ifdef SKINNING
			fragmentTangent.xyz = vec4(viewMatrix*vec4(worldTangent, 0.0)).xyz;
			fragmentTangent.w = tangent.w;
		#else
			fragmentTangent.xyz = normalMatrix*tangent.xyz;
			fragmentTangent.w = tangent.w;
		#endif
	#endif
#else
	fragmentTangent = vec4(1, 0, 0, 1);
#endif

	//Position	
	vec3 objectPosition = vec3(position);

#ifdef MORPHPOSITION
	objectPosition += (morphTarget0 - position)*morphTargetInfluences[0];
	objectPosition += (morphTarget1 - position)*morphTargetInfluences[1];
	objectPosition += (morphTarget2 - position)*morphTargetInfluences[2];
	objectPosition += (morphTarget3 - position)*morphTargetInfluences[3];
#endif

#ifdef SKINNING	
	vec4 worldPosition = skinMatrix*vec4(objectPosition, 1.0);
	vec4 viewPosition = viewMatrix*worldPosition;
	fragmentVextexWorldPosition = worldPosition.xyz;
#else
	vec4 viewPosition = worldViewMatrix*vec4(objectPosition, 1.0);
	fragmentVextexWorldPosition = (worldMatrix*vec4(objectPosition, 1.0)).xyz;
#endif

#ifdef DEPTHBIAS
	viewPosition[3] += 0.005/projectionMatrix[1][1]; 
#endif
	fragmentVextexViewPosition = viewPosition.xyz;

	// In OpenGL matrix is referred with col and row, instead of row and col.
	// For instance, Matrix[2][1] means column 2 and row 1.
	// OpenGL Projection Matrix(Column Major).
	//
	//  | n/r		0			0		0			|   | x |
	//  |											|   |   |
	//  | 0			n/t			0		0			|   | y |
	//  |											| * |   |
	//  | 0			0	-(f+n)/(f-n)	-2fn/(f-n)	|   | z |
	//  |											|   |   |
	//  | 0			0			-1		0			|   | 1 |

	mat4 jitteredProjectionMatrix = projectionMatrix;

#ifdef TAA
	vec2 halt = haltonLDS.xy / renderTargetSize.xy;
	jitteredProjectionMatrix[2][0] += halt.x;
	jitteredProjectionMatrix[2][1] += halt.y;
#endif

	gl_Position = jitteredProjectionMatrix*viewPosition;
}