#version 330

uniform vec4 objectColor;
in vec3 vertexNormal;
in vec3 positionInModelSpace;

out vec4 outputColor;

uniform vec3 ufoLightPositionInModelSpace;
uniform vec3 cameraSpaceUfoPosition;
uniform vec4 ufoLightIntensity;
uniform vec4 ufoAmbientIntensity;
uniform float ufoAttenuation;

uniform vec3 sunLightPositionInModelSpace;
uniform vec3 cameraSpaceSunPosition;
uniform vec4 sunLightIntensity;
uniform vec4 sunAmbientIntensity;
uniform float sunAttenuation;

uniform UnProjection
{
    mat4 clipToCameraMatrix;
    ivec2 windowSize;
};

vec3 CalcCameraSpacePosition() {
    vec4 ndcPos;
    ndcPos.xy = ((gl_FragCoord.xy / windowSize.xy) * 2.0) - 1.0;
    ndcPos.z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) /
        (gl_DepthRange.far - gl_DepthRange.near);
    ndcPos.w = 1.0;

    vec4 clipPos = ndcPos / gl_FragCoord.w;

    return vec3(clipToCameraMatrix * clipPos);
}

vec4 ApplyLightIntensity(in vec3 cameraSpacePosition,
        in vec3 cameraSpaceLightPos,
        in vec4 lightIntensity,
        in float lightAttenuation) {
    vec3 lightDifference = cameraSpaceLightPos - cameraSpacePosition;
    float lightDistanceSqr = dot(lightDifference, lightDifference);

    float distFactor = lightDistanceSqr;

    return lightIntensity * (1 / (1.0 + lightAttenuation * distFactor));
}

float calculateCosAngleIncidence(vec3 lightPositionInModelSpace) {
	vec3 lightDirection = normalize(lightPositionInModelSpace - positionInModelSpace);
	float cosAngleIncidence = dot(normalize(vertexNormal), lightDirection);
	cosAngleIncidence = clamp(cosAngleIncidence, 0, 1);
	return cosAngleIncidence;
}

void main() {
    vec3 cameraSpacePosition = CalcCameraSpacePosition();

    vec4 attenIntensitySun = ApplyLightIntensity(cameraSpacePosition,
            cameraSpaceSunPosition, sunLightIntensity, sunAttenuation);
	float cosSunAngleIncidence = calculateCosAngleIncidence(sunLightPositionInModelSpace);

    vec4 attenIntensityUfo = ApplyLightIntensity(cameraSpacePosition,
            cameraSpaceUfoPosition, ufoLightIntensity, ufoAttenuation);
	float cosUfoAngleIncidence = calculateCosAngleIncidence(ufoLightPositionInModelSpace);

	outputColor =
        (objectColor * attenIntensityUfo * cosUfoAngleIncidence) +
		(objectColor * attenIntensitySun * cosSunAngleIncidence) +
		(objectColor * ufoAmbientIntensity);
}
