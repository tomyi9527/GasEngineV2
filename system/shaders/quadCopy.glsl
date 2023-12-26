varying vec2 texcoord0; 
uniform sampler2D diffuse; 

void main() 
{ 
	vec4 color = texture2D(diffuse, texcoord0).rgba;
    gl_FragColor = color;
}