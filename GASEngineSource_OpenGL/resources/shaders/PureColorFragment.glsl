//Author: saralu
//Date: 2019-05-15

//notes: draw the helper entity

in vec4 fragmentColor;
uniform vec4 pureColor;

out vec4 FragColor;
void main()
{
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	//gl_FragColor = vec4(fragmentColor.rgb, opacity.x);
	FragColor = pureColor;
}