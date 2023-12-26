varying vec2 texCoord0;
varying vec4 vertexViewPos;
varying vec4 vertexColor;

uniform sampler2D diffuse;
uniform sampler2D depth; //depthRT
uniform vec2 frameBufferSize;
uniform vec2 cameraNearFar;

const float BIAS_VISIBILITY = 0.001;

float decodeDepth1(vec4 encodedDepth)
{
	float depth = dot(encodedDepth, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
    return depth;
}


float decodeDepth2(vec4 encodedDepth)
{
	float depth = dot( encodedDepth, 1.0 / vec4(1.0, 256.0, 256.0*256.0, 256.0*256.0*256.0) );
    return depth * (256.0*256.0*256.0) / (256.0*256.0*256.0 - 1.0);
}

void main(void) 
{
	float fragDepth = (-vertexViewPos.z * vertexViewPos.w - cameraNearFar.x) / (cameraNearFar.y - cameraNearFar.x);

	vec4 encodedDepth = texture2D(depth, gl_FragCoord.xy / frameBufferSize);

	float depthF = decodeDepth1(encodedDepth);
	float alpha = (fragDepth - BIAS_VISIBILITY < depthF) ? 0.9 : 0.1;

    //float alpha = 0.9;
	
	vec4 baseColor = texture2D(diffuse, texCoord0);

	//gl_FragColor = vec4(vertexColor.rgb* baseColor.rgb* alpha*baseColor.a*vertexColor.a, baseColor.a*alpha*vertexColor.a);
   	//gl_FragColor = vec4(vertexColor.rgb* baseColor.rgb, baseColor.a);
    //gl_FragColor = vec4(baseColor.a, baseColor.a, baseColor.a, 1.0);
	//gl_FragColor = vec4(vertexColor.rgb * baseColor.rgb, 1.0);
	//gl_FragColor = vec4(baseColor.rgb, alpha);
	gl_FragColor = vec4(vertexColor.rgb* baseColor.rgb, baseColor.a*alpha*vertexColor.a);
}
