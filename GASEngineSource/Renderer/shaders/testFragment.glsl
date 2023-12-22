varying vec3 osg_FragColor;
varying vec2 osg_FragTexCoord0;
varying vec2 osg_FragTexCoord1;
varying vec3 osg_FragNormal;
varying vec3 osg_FragTangent;
varying vec3 osg_FragEye;

void main() 
{   
    gl_FragColor = vec4(osg_FragColor, 1.0);
}
