#define SHADER_NAME MorphVertex

uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;

uniform float morphTargetInfluences[4];

attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;
//attribute vec3 color;

attribute vec3 morphTarget0;
attribute vec3 morphTarget1;
attribute vec3 morphTarget2;
attribute vec3 morphTarget3;

attribute vec3 morphNormal0;
attribute vec3 morphNormal1;
attribute vec3 morphNormal2;
attribute vec3 morphNormal3;

#define PI 3.14159265359
#define PI2 6.28318530718
#define RECIPROCAL_PI 0.31830988618
#define RECIPROCAL_PI2 0.15915494
#define LOG2 1.442695
#define EPSILON 1e-6
#define saturate(a) clamp( a, 0.0, 1.0 )
#define whiteCompliment(a) ( 1.0 - saturate( a ) )

float pow2( const in float x ) 
{ 
	return x*x; 
}

float pow3( const in float x ) 
{ 
	return x*x*x;
}

float pow4( const in float x ) 
{ 
	float x2 = x*x; 
	return x2*x2;
}

float average( const in vec3 color ) 
{ 
	return dot( color, vec3( 0.3333 ) ); 
}

highp float rand( const in vec2 uv ) 
{
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}


varying vec3 osg_FragEye;
varying vec3 osg_FragNormal;
varying vec4 osg_FragTangent;
varying vec2 osg_FragTexCoord0;

void main(void) 
{
	vec3 transformed = vec3( position );
	transformed += ( morphTarget0 - position ) * morphTargetInfluences[ 0 ];
	transformed += ( morphTarget1 - position ) * morphTargetInfluences[ 1 ];
	transformed += ( morphTarget2 - position ) * morphTargetInfluences[ 2 ];
	transformed += ( morphTarget3 - position ) * morphTargetInfluences[ 3 ];

    osg_FragEye = vec3(modelViewMatrix * vec4(transformed, 1.0));

	vec3 objectNormal = vec3( normal );
	objectNormal += ( morphNormal0 - normal ) * morphTargetInfluences[ 0 ];
	objectNormal += ( morphNormal1 - normal ) * morphTargetInfluences[ 1 ];
	objectNormal += ( morphNormal2 - normal ) * morphTargetInfluences[ 2 ];
	objectNormal += ( morphNormal3 - normal ) * morphTargetInfluences[ 3 ];

	vec3 n = normalMatrix * objectNormal;
	osg_FragNormal = n;

    osg_FragTangent = vec4(1, 0, 0, 0);
    osg_FragTexCoord0 = uv;

    gl_Position = projectionMatrix * modelViewMatrix * vec4(transformed, 1.0);
}