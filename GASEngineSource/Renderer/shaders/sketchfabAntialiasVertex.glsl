attribute vec3 Vertex;
varying vec2 vTexCoord0;
void main(void) {
  gl_Position = vec4(Vertex*2.0 - 1.0,1.0);
  vTexCoord0 = Vertex.xy;
}

#define SHADER_NAME Antialias