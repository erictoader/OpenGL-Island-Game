#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec3 fPos;
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

//positional lights data
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 lightPos3;
uniform vec3 lightPos4;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float sunAmbientStrength = 0.1f;
float moonAmbientStrength = 0.0f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

vec3 pLightAmbient;
vec3 pLightDiffuse;
vec3 pLightSpecular;

float computeFog() {
	float fogDensity = 0.0005f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

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

void computePointLight(vec3 lightPos, vec3 lightColor, float ambientStrength) {
	vec3 cameraPosEye = vec3(0.0f);
	
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	vec3 norm = normalize(fNormal);
	vec3 lightDir = normalize(lightPos - fPos);
	diffuse = max(dot(norm, lightDir), 0.0f) * lightColor;
	
	//compute specular light
	vec3 viewDir = normalize(cameraPosEye - fPos.xyz);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
	specular = specularStrength * spec * lightColor;
	
	// Attenuation
	float distance = length(fPos.xyz - lightPos);
	float attenuation = 10.0f / (1.0f + 0.5f * distance + 0.05f * (distance * distance));
	
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
}


void computeDirLight(vec3 lightDir, vec3 lightColor, float ambientStrength) {
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

void computePointLights() {
	computePointLight(lightPos1, lightColor, 1.0f);
	vec3 pLightAmbient1 = ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 pLightDiffuse1 = diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 pLightSpecular1 = specular * texture(specularTexture, fTexCoords).rgb;
	
	computePointLight(lightPos2, lightColor, 1.0f);
	vec3 pLightAmbient2 = ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 pLightDiffuse2 = diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 pLightSpecular2 = specular * texture(specularTexture, fTexCoords).rgb;
	
	computePointLight(lightPos3, lightColor, 1.0f);
	vec3 pLightAmbient3 = ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 pLightDiffuse3 = diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 pLightSpecular3 = specular * texture(specularTexture, fTexCoords).rgb;
	
	computePointLight(lightPos4, lightColor, 1.0f);
	vec3 pLightAmbient4 = ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 pLightDiffuse4 = diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 pLightSpecular4 = specular * texture(specularTexture, fTexCoords).rgb;
	
	pLightAmbient = pLightAmbient1 + pLightAmbient2 + pLightAmbient3 + pLightAmbient4;
	pLightDiffuse = pLightDiffuse1 + pLightDiffuse2 + pLightDiffuse3 + pLightDiffuse4;
	pLightSpecular = pLightSpecular1 + pLightSpecular2 + pLightSpecular3 + pLightSpecular4;
}

void main() {
	
	if (astralObject) {
		fColor = vec4(texture(diffuseTexture, fTexCoords).rgb, 1.0f);
		return;
	}
	
	// sun directional
	computeDirLight(lightDir, lightColor, sunAmbientStrength);
	vec3 sunAmbient = 0.8 * ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 sunDiffuse = 0.8 * diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 sunSpecular = specular * texture(specularTexture, fTexCoords).rgb;
	
	// moon directional
	computeDirLight(moonLightDir, moonLightColor, moonAmbientStrength);
	vec3 moonAmbient = 0.4 * ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 moonDiffuse = 0.1 * diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 moonSpecular = specular * texture(specularTexture, fTexCoords).rgb;
	
	// point lights
	computePointLights();
	
	float shadow = dayFactor * computeShadow();
	ambient = dayFactor * sunAmbient + (1.0f - dayFactor) * (moonAmbient + pLightAmbient);
	diffuse = dayFactor * sunDiffuse + (1.0f - dayFactor) * (moonDiffuse + pLightDiffuse);
	specular = dayFactor * sunDiffuse + (1.0f - dayFactor) * (moonDiffuse + pLightSpecular);
	
	float alpha = texture(diffuseTexture, fTexCoords).a;
	vec4 color = vec4(min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f), alpha);
	
	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	fColor = mix(fogColor, color, fogFactor);

}
