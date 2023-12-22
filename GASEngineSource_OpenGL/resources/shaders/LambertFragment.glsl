in vec4 fragmentColor;
in vec2 fragmentTexCoord0;
in vec2 fragmentTexCoord1;
in vec3 fragmentNormal;
in vec4 fragmentTangent;
in vec3 fragmentVextexViewPosition;

uniform vec3 lightVec;
out vec4 FragColor;

void main() 
{   
    vec3 N = normalize(fragmentNormal);
    vec3 L = normalize(lightVec);

    float diffuse = 0.5*max(dot(N,L),0.0)+0.5;

    FragColor = vec4(diffuse * fragmentColor.rgb, 1.0);
}
