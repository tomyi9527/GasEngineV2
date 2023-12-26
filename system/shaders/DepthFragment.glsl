//Author: saralu
//Date: 2019-04-30

//notes: draw the depth buffer and write in depthRT frameBuffer

uniform vec2 cameraNearFar;
varying vec3 fragmentVextexViewPosition;

vec4 encodeDepth1(float depth)
{
    vec4 tmp = fract(vec4(1.0, 255.0, 65025.0, 16581375.0) * depth);
    tmp -= tmp.yzww * vec4(1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0, 0.0);
    tmp.a = 1.0;

    return tmp;
}

vec4 encodeDepth2(float depth)
{
    depth *= (256.0*256.0*256.0 - 1.0) / (256.0*256.0*256.0); 
    vec4 encode = fract( depth * vec4(1.0, 256.0, 256.0*256.0, 256.0*256.0*256.0) );
    return vec4( encode.xyz - encode.yzw / 256.0, encode.w ) + 1.0/512.0; 
}

void main()
{
    float depth = (-fragmentVextexViewPosition.z * 1.0 - cameraNearFar.x) / (cameraNearFar.y - cameraNearFar.x);
    vec4 encodedDepth = encodeDepth1(depth);
	gl_FragColor = encodedDepth;
}