uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;

#ifdef USE_MORPHTARGETS
	uniform float morphTargetInfluences[4];
#endif

#ifdef USE_SKINNING
	uniform mat4 bindMatrix;
	uniform mat4 bindMatrixInverse;

	#ifdef BONE_TEXTURE
		uniform sampler2D boneTexture;
		uniform int boneTextureWidth;
		uniform int boneTextureHeight;

		#ifdef FLOAT_TEXTURE

			mat4 getBoneMatrix(const in float i)
			{
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
				float x = mod(j, float(boneTextureWidth));
				float y = floor(j / float(boneTextureWidth));
				float dx = 1.0 / float(boneTextureWidth);
				float dy = 1.0 / float(boneTextureHeight);
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
	#else
		uniform mat4 boneGlobalMatrices[MAX_BONES];

		mat4 getBoneMatrix(const in float i) 
		{
			return boneGlobalMatrices[int(i)];
		}
	#endif
#endif

//Vertex inputs
attribute vec3 position;

#ifdef USE_NORMAL
	attribute vec3 normal;
#endif

#ifdef USE_TANGENT
	attribute vec4 tangent;
#endif

#ifdef USE_UV0
	attribute vec2 uv;
#endif

#ifdef USE_UV1
	attribute vec2 uv2;
#endif

#ifdef USE_COLOR
	attribute vec4 color;
#endif

#ifdef USE_MORPHTARGETS
	attribute vec3 morphTarget0;
	attribute vec3 morphTarget1;
	attribute vec3 morphTarget2;
	attribute vec3 morphTarget3;	
#endif

#ifdef USE_MORPHNORMALS
	attribute vec3 morphNormal0;
	attribute vec3 morphNormal1;
	attribute vec3 morphNormal2;
	attribute vec3 morphNormal3;
#endif

#ifdef USE_SKINNING
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
varying vec3 fragmentViewPosition;

void main() 
{
#ifdef USE_COLOR
	fragmentColor = color;
#else
	fragmentColor = vec4(1.0, 1.0, 1.0, 1.0);
#endif

#ifdef USE_UV0
	fragmentTexCoord0 = uv;
#else
	fragmentTexCoord0 = vec2(0, 0);
#endif

#ifdef USE_UV1
	fragmentTexCoord1 = uv2;
#else
	fragmentTexCoord1 = vec2(0, 0);
#endif
	
#ifdef USE_NORMAL
	vec3 objectNormal = vec3(normal);

	#ifdef USE_MORPHNORMALS
		//objectNormal = morphTransform(morphTargetInfluences, normal, morphNormal0, morphNormal1, morphNormal2, morphNormal3);
		objectNormal += (morphNormal0 - normal)*morphTargetInfluences[0];
		objectNormal += (morphNormal1 - normal)*morphTargetInfluences[1];
		objectNormal += (morphNormal2 - normal)*morphTargetInfluences[2];
		objectNormal += (morphNormal3 - normal)*morphTargetInfluences[3];
	#endif

	#ifdef USE_SKINNING
		mat4 boneMatX = getBoneMatrix( skinIndex.x );
		mat4 boneMatY = getBoneMatrix( skinIndex.y );
		mat4 boneMatZ = getBoneMatrix( skinIndex.z );
		mat4 boneMatW = getBoneMatrix( skinIndex.w );
	#endif

	#ifdef USE_SKINNING
		mat4 skinMatrix = mat4( 0.0 );
		skinMatrix += skinWeight.x * boneMatX;
		skinMatrix += skinWeight.y * boneMatY;
		skinMatrix += skinWeight.z * boneMatZ;
		skinMatrix += skinWeight.w * boneMatW;
		skinMatrix  = bindMatrixInverse * skinMatrix * bindMatrix;
		objectNormal = vec4(skinMatrix * vec4(objectNormal, 0.0)).xyz;
	#endif

	#ifdef OUTPUT_NORMAL_ONLY
		fragmentNormal = (modelMatrix*vec4(objectNormal, 0.0)).xyz;
	#else
		fragmentNormal = normalMatrix*objectNormal;
	#endif
#else
	fragmentNormal = vec3(0, 1, 0);
#endif

#ifdef USE_TANGENT
	vec3 objectTangent = tangent.xyz;

	#ifdef USE_SKINNING
		objectTangent = vec4(skinMatrix*vec4(tangent.xyz, 0.0)).xyz;
	#endif

	#ifdef OUTPUT_NORMAL_ONLY
		fragmentTangent.xyz = (modelMatrix*vec4(objectTangent.xyz, 0.0)).xyz;
		fragmentTangent.w = tangent.w;
	#else
		fragmentTangent.xyz = normalMatrix*objectTangent;
		fragmentTangent.w = tangent.w; //handness
	#endif	
#else
	fragmentTangent = vec4(1, 0, 0, 1);
#endif
	
	vec3 transformed = vec3(position);

#ifdef USE_MORPHTARGETS
	transformed += ( morphTarget0 - position ) * morphTargetInfluences[ 0 ];
	transformed += ( morphTarget1 - position ) * morphTargetInfluences[ 1 ];
	transformed += ( morphTarget2 - position ) * morphTargetInfluences[ 2 ];
	transformed += ( morphTarget3 - position ) * morphTargetInfluences[ 3 ];
#endif

#ifdef USE_SKINNING
	vec4 skinVertex = bindMatrix * vec4( transformed, 1.0 );
	vec4 skinned = vec4( 0.0 );
	skinned += boneMatX * skinVertex * skinWeight.x;
	skinned += boneMatY * skinVertex * skinWeight.y;
	skinned += boneMatZ * skinVertex * skinWeight.z;
	skinned += boneMatW * skinVertex * skinWeight.w;
	skinned  = bindMatrixInverse * skinned;
#endif

#ifdef USE_SKINNING
	vec4 mvPosition = modelViewMatrix * skinned;
#else
	vec4 mvPosition = modelViewMatrix * vec4(transformed, 1.0);
#endif

#ifdef USE_DEPTHBIAS
	mvPosition[3] += 0.005 / projectionMatrix[1][1];
#endif

	fragmentViewPosition = mvPosition.xyz;

	gl_Position = projectionMatrix * mvPosition;
}