//Output to fragment

in vec4 fragmentColor;
in vec2 fragmentTexCoord0;
in vec2 fragmentTexCoord1;
in vec3 fragmentNormal;
in vec4 fragmentTangent;
in vec3 fragmentVextexViewPosition;

out vec4 FragColor;

uniform vec3 lineColor;


void main() 
{   
    // FragColor = vec4(0.870588235, 0.894117647, 0.854901961, 1.0);
     FragColor = vec4(lineColor.rgb, 1.0);
}
