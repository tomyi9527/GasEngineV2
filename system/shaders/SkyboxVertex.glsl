
attribute vec3 position;

varying vec3 fragmentTexCoord0;

void main() 
{ 
	fragmentTexCoord0 = position;

    gl_Position = vec4(position.xy, 1.0, 1.0);
}