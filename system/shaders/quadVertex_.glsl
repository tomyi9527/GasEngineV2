attribute vec3 position;

varying vec2 texcoord0;

void main() 
{ 
    vec2 uv = (position.xy + 1.0) / 2.0;
	//uv.y = 1.0 - uv.y;
	texcoord0 = uv;
    gl_Position = vec4(position.xyz, 1.0 );
}