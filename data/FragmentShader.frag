#version 330

uniform vec4 objectColor;
in vec3 vertexNormal;
in vec3 positionInModelSpace;

out vec4 outputColor;

uniform vec3 ufoLightPositionInModelSpace;
uniform vec4 ufoLightIntensity;
uniform vec4 ufoAmbientIntensity;

uniform vec3 sunLightPositionInModelSpace;
uniform vec4 sunLightIntensity;
uniform vec4 sunAmbientIntensity;

float calculateCosAngleIncidence(vec3 lightPositionInModelSpace)
{
	vec3 lightDirection = normalize(lightPositionInModelSpace - positionInModelSpace);
	float cosAngleIncidence = dot(normalize(vertexNormal), lightDirection);
	cosAngleIncidence = clamp(cosAngleIncidence, 0, 1);
	return cosAngleIncidence;
}

void main()
{
	float cosSunAngleIncidence = calculateCosAngleIncidence(sunLightPositionInModelSpace);
	float cosUfoAngleIncidence = calculateCosAngleIncidence(ufoLightPositionInModelSpace);
	
	outputColor = (objectColor * ufoLightIntensity * cosUfoAngleIncidence) +
		(objectColor * sunLightIntensity * cosSunAngleIncidence) +
		(objectColor * ufoAmbientIntensity);
}
