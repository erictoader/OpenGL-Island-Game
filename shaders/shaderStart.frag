#version 400

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 sunLightDir;
uniform vec3 sunLightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

uniform bool astralObject;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

void computeDirLight()
{
	//compute eye space coordinates
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	vec3 normalEye = normalize(normalMatrix * fNormal);

	//normalize light direction
	vec3 sunLightDirN = vec3(normalize(view * vec4(sunLightDir, 0.0f)));

	//compute view direction (in eye coordinates, the viewer is situated at the origin
	vec3 viewDir = normalize(- fPosEye.xyz);

	//compute ambient light
	ambient = ambientStrength * sunLightColor;

	//compute diffuse light
	diffuse = max(dot(normalEye, sunLightDirN), 0.0f) * sunLightColor;

	//compute specular light
	vec3 reflectDir = reflect(-sunLightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	specular = specularStrength * specCoeff * sunLightColor;
}

void main()
{
	vec3 color;
	
	if (astralObject) {
		color = min(sunLightColor * texture(diffuseTexture, fTexCoords).rgb, 1.0f);
	} else {
		computeDirLight();
		color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
	}

	fColor = vec4(color, 1.0f);
}
