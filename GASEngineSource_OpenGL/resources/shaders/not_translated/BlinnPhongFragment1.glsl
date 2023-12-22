//Author: saralu
//Date: 2019-05-05

//TODO: saralu

varying vec4 fragmentColor;
varying vec2 fragmentTexCoord0;
varying vec2 fragmentTexCoord1;
varying vec3 fragmentNormal;
varying vec4 fragmentTangent;
varying vec3 fragmentVextexViewPosition;
varying vec3 fragmentVextexWorldPosition;

//ambient
//环境反射颜色 = 入射光颜色 * 物体表面基底色
vec3 ambientLightColor = vec3(0.5, 0.5, 0.0);

//diffuse
//漫反射颜色 = 入射光颜色 * 物体表面基底色 * cosx
//normal和lightDirection的夹角
//光的漫反射强度值
//材质的漫反射系数
//vec3 diffuseColor = vec3(0.5, 0.0, 0.0);


//specular
vec3 specularLightColor = vec3(1.0, 1.0, 1.0);
//vec3 viewPos = vec3(0.0, 0.0, 0.0);

//directionalLight
struct DirectionalLight
{
  vec3 direction;
  vec4 color;
  vec3 position;
  float intensity;
};
uniform DirectionalLight directionalLight;

//pointLight
struct PointLight
{
  vec4 color;
  vec3 position;
  float intensity;
  float decay;
};
uniform PointLight pointLight;


void main() 
{
  vec3 eyeVector = normalize(fragmentVextexViewPosition.xyz) * -1.0;
  
  vec3 resultColor = vec3(1.0, 1.0, 1.0);

  vec3 frontNormal = gl_FrontFacing ? fragmentNormal : -fragmentNormal;
	frontNormal = normalize(frontNormal);

  vec3 normal = frontNormal;

  //directionl diffuse
  vec3 directionalDiffuseColor = directionalLight.color.rgb * directionalLight.intensity * fragmentColor.rgb * dot(normalize(directionalLight.direction), normal);
  //resultColor += directionalDiffuseColor;

  //ambient
  vec3 ambientColor = ambientLightColor * fragmentColor.rgb;

  //pointLight
  vec3 lightDirection = normalize(pointLight.position - vec3(fragmentVextexViewPosition.xyz));
  float nDotL = max(dot(lightDirection, normal), 0.0);
  vec3 diffuseColor = pointLight.color.rgb * pointLight.intensity * fragmentColor.rgb * nDotL * pointLight.decay;
  vec3 viewDir = normalize(eyeVector - fragmentVextexWorldPosition);
  vec3 halfDir = normalize(lightDirection + viewDir);
  float specAngle = max(dot(halfDir, normal), 0.0);
  float specular = pow(specAngle, 16.0);


  resultColor = ambientColor + diffuseColor * nDotL + specularLightColor * specular;
  //resultColor = ambientColor;
  //resultColor = diffuseColor * nDotL;
  //resultColor = specularLightColor * specular;

	gl_FragColor = vec4(resultColor, 1.0);
}
    
