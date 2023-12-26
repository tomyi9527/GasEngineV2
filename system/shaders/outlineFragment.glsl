varying vec2 texcoord0; 
uniform sampler2D diffuse; 
uniform vec2 ratio;
uniform vec2 windowSize;
uniform vec3 outlineColor;

const float outlineWidth = 2.0;
void main() 
{ 
	vec2 texelSize = 1.0 / windowSize;
	vec2 temp0 = texcoord0 + texelSize*vec2(outlineWidth, outlineWidth);
	vec2 temp1 = texcoord0 + texelSize*vec2(-outlineWidth, outlineWidth);
	vec2 temp2 = texcoord0 + texelSize*vec2(outlineWidth, -outlineWidth);
	vec2 temp3 = texcoord0 + texelSize*vec2(-outlineWidth, -outlineWidth);

    vec4 color0 = texture2D(diffuse, temp0).rgba;
	vec4 color1 = texture2D(diffuse, temp1).rgba;
	vec4 color2 = texture2D(diffuse, temp2).rgba;
	vec4 color3 = texture2D(diffuse, temp3).rgba;

	float value = clamp(color0.r + color1.r + color2.r + color3.r, 0.0, 1.0);

	if (value == 0.0)
	{
		discard;
	}

    gl_FragColor = vec4(outlineColor, 1.0);
}