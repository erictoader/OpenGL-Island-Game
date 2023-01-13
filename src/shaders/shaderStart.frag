#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform vec3 moonLightDir;
uniform vec3 moonLightColor;

//astral object: bypass all light calculations
uniform bool astralObject;

//sun set and is below map: bypass all shadow calculations
uniform float dayFactor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float sunAmbientStrength = 0.2f;
float moonAmbientStrength = 0.0f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float computeShadow() {
	//perform perspective divide
	vec3 normalizedCoords= fragPosLightSpace.xyz / fragPosLightSpace.w;

	//tranform from [-1,1] range to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	//get closest depth value from lights perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	//get depth of current fragment from lights perspective
	float currentDepth = normalizedCoords.z;

	//if the current fragments depth is greater than the value in the depth map, the current fragment is in shadow
	//else it is illuminated
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	return shadow;
}

void computeLightComponents(vec3 lightDir, vec3 lightColor, float ambientStrength) {
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

void main() {
	if (astralObject) {
		fColor = vec4(texture(diffuseTexture, fTexCoords).rgb, 1.0f);
		return;
	}
	
	computeLightComponents(lightDir, lightColor, sunAmbientStrength);
	vec3 sunAmbient = ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 sunDiffuse = diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 sunSpecular = specular * texture(specularTexture, fTexCoords).rgb;
	
	computeLightComponents(moonLightDir, moonLightColor, moonAmbientStrength);
	vec3 moonAmbient = 0.6 * ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 moonDiffuse = 0.2 * diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 moonSpecular = specular * texture(specularTexture, fTexCoords).rgb;
	
	float shadow = dayFactor * computeShadow();
	ambient = dayFactor * sunAmbient + (1.0f - dayFactor) * moonAmbient;
	diffuse = dayFactor * sunDiffuse + (1.0f - dayFactor) * moonDiffuse;
	specular = dayFactor * sunDiffuse + (1.0f - dayFactor) * moonDiffuse;
	
	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
	
	fColor = vec4(color, 1.0f);

}
