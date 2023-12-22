varying vec4 fragmentColor;
varying vec2 fragmentTexCoord0;
varying vec2 fragmentTexCoord1;
varying vec3 fragmentNormal;
varying vec4 fragmentTangent;
varying vec3 fragmentVextexViewPosition;

uniform vec3 lightVec;

void main() 
{   
    vec3 N = normalize(fragmentNormal);
    vec3 L = normalize(lightVec);

    float diffuse = 0.5*max(dot(N,L),0.0)+0.5;

    gl_FragColor = vec4(diffuse*fragmentColor.rgb,1.0);
}
