
attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;
//attribute vec4 tangent;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

varying vec3 osg_FragEye;
varying vec3 osg_FragNormal;
//varying vec4 osg_FragTangent;
varying vec2 osg_FragTexCoord0;

void main(void) 
{
    osg_FragEye = vec3(modelViewMatrix * vec4(position, 1.0));

	vec3 n = normalMatrix * normal;
	osg_FragNormal = n;

	//osg_FragTangent = vec4(1, 0, 0, 0);
    osg_FragTexCoord0 = uv;

    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}
