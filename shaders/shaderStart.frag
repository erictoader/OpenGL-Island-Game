#version 400

in vec2 fTextureCoords;
in vec3 ambient;
in vec3 diffuse;
in vec3 specular;
	
out vec4 fColor;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

void main() {
	vec3 diffuseColor = texture(diffuseTexture, fTextureCoords).rgb;
	vec3 specularColor = texture(specularTexture, fTextureCoords).rgb;
	
	vec3 color = min((ambient + diffuse) * diffuseColor + specular * specularColor, 1.0f);
	fColor = vec4(color, 1.0f);
}
