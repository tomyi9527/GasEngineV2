attribute vec3 position;
varying vec3 texcoord0;
void main() 
{ 
    texcoord0 = position; 
    gl_Position = vec4(position.xy, 1.0, 1.0);
}