//Output to fragment

varying vec4 fragmentColor;
varying vec2 fragmentTexCoord0;
varying vec2 fragmentTexCoord1;
varying vec3 fragmentNormal;
varying vec4 fragmentTangent;
varying vec3 fragmentVextexViewPosition;

uniform vec4 lineColor;

void main() 
{   
    gl_FragColor = lineColor;
}
