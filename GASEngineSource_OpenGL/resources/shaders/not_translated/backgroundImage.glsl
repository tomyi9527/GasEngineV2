varying vec2 texcoord0; 
uniform sampler2D diffuse; 
uniform vec2 ratio;

void main() 
{ 
	vec2 temp = (texcoord0 - vec2(0.5, 0.5))*ratio + vec2(0.5, 0.5);

    vec4 color = texture2D(diffuse, temp).rgba;
    gl_FragColor = vec4(color.rgb, 0.0);
}