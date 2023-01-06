#version 400

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTextureCoords;

out vec2 fTextureCoords;
out vec3 ambient;
out vec3 diffuse;
out vec3 specular;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

uniform vec3 lightDir;
uniform vec3 lightColor;

float ambientStrength = 0.2f;
float specularStrength = 0.5f;
float shininess = 32.0f;

void main() {
	fTextureCoords = vTextureCoords;

	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	vec3 lightDirN = normalize(lightDir);
	vec3 normalEye = normalize(normalMatrix * vNormal);
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute the vertex position in eye coordinates
	vec4 vertPosEye = view * model * vec4(vPosition, 1.0f);
	//compute the view (Eye) direction (in eye coordinates, the camera is at the origin)
	vec3 viewDir = normalize(-vertPosEye.xyz);
	//compute the light's reflection (the reflect function requires a direction pointing towards the vertex, not away from it)
	vec3 reflectDir = normalize(reflect(-lightDir, normalEye)); //compute specular light
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
	
    gl_Position = projection * view * model * vec4(vPosition, 1.0);
}
