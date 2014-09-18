#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform vec4 objectColor;
out vec3 vertexNormal;
out vec3 positionInModelSpace;

uniform mat4 modelToCameraMatrix;

uniform Projection
{
	mat4 cameraToClipMatrix;
};

void main()
{
	gl_Position = cameraToClipMatrix * (modelToCameraMatrix * vec4(position, 1.0));

	positionInModelSpace = position;
	vertexNormal = normal;
}
