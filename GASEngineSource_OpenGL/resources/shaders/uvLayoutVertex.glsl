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

uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;

uniform float uvScale;
uniform vec2 offset;

//Vertex inputs
in vec3 position;

#if defined(UV0)
	in vec2 uv;
#elif defined(UV1)
	in vec2 uv1;
#else

#endif

//Output to fragment
out vec4 fragmentColor;
out vec2 fragmentTexCoord0;
out vec2 fragmentTexCoord1;
out vec3 fragmentNormal;
out vec4 fragmentTangent;
out vec3 fragmentEye;

void main() 
{
	fragmentColor = vec4(1.0, 1.0, 1.0, 1.0);

#if defined(UV0)
	fragmentTexCoord0 = uv;
    fragmentTexCoord1 = vec2(0, 0);
#elif defined(UV1)
    fragmentTexCoord0 = vec2(0, 0);
	fragmentTexCoord1 = uv1;
#else
    fragmentTexCoord0 = vec2(0, 0);
	fragmentTexCoord1 = vec2(0, 0);
#endif

	fragmentNormal = vec3(0, 1, 0);
	fragmentTangent = vec4(1, 0, 0, 1);

	//Position	
	vec3 objectPosition = vec3(position);
	vec4 viewPosition = modelViewMatrix*vec4(objectPosition, 1.0);

#ifdef DEPTHBIAS
	viewPosition[3] += 0.005/projectionMatrix[1][1]; 
#endif

	fragmentEye = viewPosition.xyz;

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

#if defined(UV0)
	vec2 tempUV = (fragmentTexCoord0 - offset) * (2.0 * uvScale, 2.0 * uvScale);
#elif defined(UV1)
	vec2 tempUV = (fragmentTexCoord1 - vec2(0.5, 0.5)) * (2.0 * uvScale, 2.0 * uvScale);	
#else
	vec2 tempUV = vec2(0, 0);
#endif

    gl_Position = vec4(tempUV.x, tempUV.y, 0, 1);
}