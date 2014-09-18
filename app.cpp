#include <string>
#include <vector>
#include <stack>
#include <math.h>
#include <stdio.h>
#include <glload/gl_3_3.h>
#include <glutil/glutil.h>
#include <GL/freeglut.h>
#include "framework/framework.h"
#include "framework/Mesh.h"
#include "framework/Timer.h"
#include "framework/MousePole.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct SimpleProgramData {
	GLuint theProgram;
	GLuint objectColorUniform;
	GLuint modelToCameraMatrixUniform;
};

struct ProgramData: SimpleProgramData {
	GLuint carLightPositionInModelSpaceUniform;
	GLuint carLightIntensityUniform;
	GLuint carAmbientIntensityUniform;

	GLuint sunLightPositionInModelSpaceUniform;
	GLuint sunLightIntensityUniform;
	GLuint sunAmbientIntensityUniform;
};

struct ProjectionBlock {
	glm::mat4 cameraToClipMatrix;
};

float zNear = 1.0f;
float zFar = 1000.0f;

ProgramData program;
SimpleProgramData lightProgram;

Framework::Mesh *planeMesh = NULL;
Framework::Mesh *sunMesh = NULL;
Framework::Mesh *carBodyMesh = NULL;
Framework::Mesh *carLightMesh = NULL;
Framework::Mesh *cubeMesh = NULL;
Framework::Mesh *cylinderMesh = NULL;
Framework::Mesh *sphereMesh = NULL;

glutil::ViewData initialViewData = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::fquat(1.0f, 0.5f, 0.0f, 0.0f),
		50.0f, 0.0f
};

glutil::ViewScale viewScale = {
		5.0f, 150.0f,
		1.5f, 0.5f,
		0.0f, 0.0f,
		90.0f / 250.0f
};

glutil::ViewPole viewPole = glutil::ViewPole(initialViewData, viewScale,
		glutil::MB_LEFT_BTN);

glm::vec3 carPosition(0.0f, 0.5f, 0.0f);
glm::vec3 carLightPosition(0.0f, 0.5f, 6.0f);
glm::vec3 cubePosition = glm::vec3(9.0f, 0.0f, 19.0f);
glm::vec3 cylinderPosition = glm::vec3(23.0f, -0.5f, -26.0f);
glm::vec3 spherePosition = glm::vec3(-20.0f, 1.5f, -12.0f);
float carAngleInDegrees = 0.0f;

glutil::ObjectData carObjectData = {
		carPosition,
		glm::fquat(1.0f, 0.0f, 0.0f, 0.0f)
};

glutil::ObjectPole carObjectPole = glutil::ObjectPole(carObjectData, 1.0f,
		glutil::MB_RIGHT_BTN, &viewPole);

const int projectionBlockIndex = 2;
GLuint projectionUniformBuffer = 0;

glm::vec3 sunLightIntensity = glm::vec3(0.5f, 0.5f, 0.5f);
glm::vec3 sunAmbientIntensity = glm::vec3(0.3f, 0.3f, 0.3f);
glm::vec3 carLightIntensity = glm::vec3(0.8f, 0.8f, 0.8f);
glm::vec3 carAmbientIntensity = glm::vec3(0.2f, 0.2f, 0.2f);

glm::vec3 planeColor = glm::vec3(0.2f, 0.8f, 0.3f);
glm::vec3 sunLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 carBodyColor = glm::vec3(0.9f, 0.1f, 0.3f);
glm::vec3 carLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 cubeColor = glm::vec3(0.2f, 0.2f, 1.0f);
glm::vec3 cylinderColor = glm::vec3(0.9f, 0.9f, 0.0f);
glm::vec3 sphereColor = glm::vec3(0.9f, 0.2f, 0.7f);

float sunLightRadius = 50.0f;
float sunLightMaxHeight = 120.0f;
Framework::Timer sunLightTimer(Framework::Timer::TT_LOOP, 30.0f);

float carRadius = 6.71f;
float cubePlusCarRadius = 7.0711f + carRadius;
float cylinderPlusCarRadius = 3.0f + carRadius;
float spherePlusCarRadius = 4.5f + carRadius;

glm::vec4 calculateSunPosition() {
	float currentLoopTime = sunLightTimer.GetAlpha();

	float sunX = cosf(currentLoopTime * M_PI * 2.0f) * sunLightRadius;
	float factor;
	switch ((int) (currentLoopTime * 4) % 4) {
	case 0:
		factor = -currentLoopTime;
		break;
	case 1:
	case 2:
		factor = currentLoopTime - 0.5f;
		break;
	case 3:
		factor = 1.0f - currentLoopTime;
		break;
	}
	float sunY = sunLightMaxHeight * factor;
	float sunZ = sinf(currentLoopTime * M_PI * 2.0f) * sunLightRadius;

	return glm::vec4(sunX, sunY, sunZ, 1.0f);
}

void calculateCarLightPosition() {
	float angleInRadians = Framework::DegToRad(carAngleInDegrees);
	carLightPosition = glm::vec3(carPosition.x + 6.0f * sinf(angleInRadians),
			carPosition.y, carPosition.z + 6.0f * cosf(angleInRadians));
}

bool newCarPositionIsFine(float positionX, float positionZ) {
	float cubeCarDistance = sqrtf(
			powf(positionX - cubePosition.x, 2.0f)
					+ powf(positionZ - cubePosition.z, 2.0f));
	float cylinderCarDistance = sqrtf(
			powf(positionX - cylinderPosition.x, 2.0f)
					+ powf(positionZ - cylinderPosition.z, 2.0f));
	float sphereCarDistance = sqrtf(
			powf(positionX - spherePosition.x, 2.0f)
					+ powf(positionZ - spherePosition.z, 2.0f));
	return cubeCarDistance - cubePlusCarRadius > 0.0f
			&& cylinderCarDistance - cylinderPlusCarRadius > 0.0f
			&& sphereCarDistance - spherePlusCarRadius > 0.0f
			&& positionX <= 50.0f - carRadius && positionZ <= 50.0f - carRadius
			&& positionX >= carRadius - 50.0f && positionZ >= carRadius - 50.0f;
}

void calculateCarPosition(float direction) {
	float angleInRadians = Framework::DegToRad(carAngleInDegrees);
	float newCarPositionZ = carPosition.z + direction * 0.5f * cosf(angleInRadians);
	float newCarPositionX = carPosition.x + direction * 0.5f * sinf(angleInRadians);
	if (newCarPositionIsFine(newCarPositionX, newCarPositionZ)) {
		carPosition.z = newCarPositionZ;
		carPosition.x = newCarPositionX;
	}
}

SimpleProgramData initializeSimpleProgram(std::string vertexShader,
		std::string fragmentShader) {
	std::vector<GLuint> shaderList;

	shaderList.push_back(Framework::LoadShader(GL_VERTEX_SHADER, vertexShader));
	shaderList.push_back(
			Framework::LoadShader(GL_FRAGMENT_SHADER, fragmentShader));

	SimpleProgramData programData;
	programData.theProgram = Framework::CreateProgram(shaderList);
	programData.modelToCameraMatrixUniform = glGetUniformLocation(
			programData.theProgram, "modelToCameraMatrix");
	programData.objectColorUniform = glGetUniformLocation(
			programData.theProgram, "objectColor");

	GLuint projectionBlock = glGetUniformBlockIndex(programData.theProgram,
			"Projection");
	glUniformBlockBinding(programData.theProgram, projectionBlock,
			projectionBlockIndex);

	return programData;
}

ProgramData initializeProgram(std::string vertexShader,
		std::string fragmentShader) {
	std::vector<GLuint> shaderList;

	shaderList.push_back(Framework::LoadShader(GL_VERTEX_SHADER, vertexShader));
	shaderList.push_back(
			Framework::LoadShader(GL_FRAGMENT_SHADER, fragmentShader));

	ProgramData programData;
	programData.theProgram = Framework::CreateProgram(shaderList);
	programData.modelToCameraMatrixUniform = glGetUniformLocation(
			programData.theProgram, "modelToCameraMatrix");
	programData.objectColorUniform = glGetUniformLocation(
			programData.theProgram, "objectColor");

	programData.sunLightPositionInModelSpaceUniform = glGetUniformLocation(
			programData.theProgram, "sunLightPositionInModelSpace");
	programData.sunLightIntensityUniform = glGetUniformLocation(
			programData.theProgram, "sunLightIntensity");
	programData.sunAmbientIntensityUniform = glGetUniformLocation(
			programData.theProgram, "sunAmbientIntensity");

	programData.carLightPositionInModelSpaceUniform = glGetUniformLocation(
			programData.theProgram, "carLightPositionInModelSpace");
	programData.carLightIntensityUniform = glGetUniformLocation(
			programData.theProgram, "carLightIntensity");
	programData.carAmbientIntensityUniform = glGetUniformLocation(
			programData.theProgram, "carAmbientIntensity");

	GLuint projectionBlock = glGetUniformBlockIndex(programData.theProgram,
			"Projection");
	glUniformBlockBinding(programData.theProgram, projectionBlock,
			projectionBlockIndex);

	return programData;
}

void initializeProgramsAndMeshes() {
	program = initializeProgram("VertexShader.vert", "FragmentShader.frag");
	lightProgram = initializeSimpleProgram("LightVertexShader.vert",
			"LightFragmentShader.frag");

	try {
		planeMesh = new Framework::Mesh("Plane.xml");
		sunMesh = new Framework::Mesh("Sphere.xml");
		carBodyMesh = new Framework::Mesh("Car.xml");
		carLightMesh = new Framework::Mesh("Sphere.xml");
		cubeMesh = new Framework::Mesh("Cube.xml");
		cylinderMesh = new Framework::Mesh("Cylinder.xml");
		sphereMesh = new Framework::Mesh("BigSphere.xml");
	} catch (std::exception &e) {
		printf("%s\n", e.what());
		throw;
	}
}

namespace {
void MouseMotion(int x, int y) {
	Framework::ForwardMouseMotion(viewPole, x, y);
	glutPostRedisplay();
}

void MouseButton(int button, int state, int x, int y) {
	Framework::ForwardMouseButton(viewPole, button, state, x, y);
	glutPostRedisplay();
}

void MouseWheel(int wheel, int direction, int x, int y) {
	Framework::ForwardMouseWheel(viewPole, wheel, direction, x, y);
	glutPostRedisplay();
}
}

void init() {
	initializeProgramsAndMeshes();

	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutMouseWheelFunc(MouseWheel);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);

	glGenBuffers(1, &projectionUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, projectionUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionBlock), NULL,
			GL_DYNAMIC_DRAW);

	glBindBufferRange(GL_UNIFORM_BUFFER, projectionBlockIndex,
			projectionUniformBuffer, 0, sizeof(ProjectionBlock));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void renderMesh(const Framework::Mesh* mesh,
		const glutil::MatrixStack& modelMatrix,
		const glm::vec4& sunLightPositionInCameraSpace,
		const glm::vec4& carLightPositionInCameraSpace,
		const glm::vec3& color) {

	glm::mat4 invertedModelMatrix = glm::inverse(modelMatrix.Top());
	glm::vec4 sunLightPositionInModelSpace = invertedModelMatrix
			* sunLightPositionInCameraSpace;
	glm::vec4 carLightPositionInModelSpace = invertedModelMatrix
			* carLightPositionInCameraSpace;

	glUseProgram(program.theProgram);
	glUniformMatrix4fv(program.modelToCameraMatrixUniform, 1, GL_FALSE,
			glm::value_ptr(modelMatrix.Top()));
	glUniform3fv(program.sunLightPositionInModelSpaceUniform, 1,
			glm::value_ptr(sunLightPositionInModelSpace));
	glUniform3fv(program.carLightPositionInModelSpaceUniform, 1,
			glm::value_ptr(carLightPositionInModelSpace));
	glUniform4fv(program.objectColorUniform, 1,
			glm::value_ptr(glm::vec4(color, 1.0f)));
	mesh->Render();
	glUseProgram(0);
}

void renderLightMesh(const Framework::Mesh *mesh,
		const glutil::MatrixStack& modelMatrix, const glm::vec3& color) {
	glUseProgram(lightProgram.theProgram);
	glUniformMatrix4fv(lightProgram.modelToCameraMatrixUniform, 1, GL_FALSE,
			glm::value_ptr(modelMatrix.Top()));
	glUniform4fv(lightProgram.objectColorUniform, 1,
			glm::value_ptr(glm::vec4(color, 1.0f)));
	mesh->Render();
	glUseProgram(0);
}

void display() {
	sunLightTimer.Update();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (carBodyMesh && planeMesh && carLightMesh && cubeMesh && cylinderMesh
			&& sphereMesh) {
		glutil::MatrixStack modelMatrix;
		modelMatrix.SetMatrix(viewPole.CalcMatrix());

		glm::vec4 sunPosition = calculateSunPosition();
		glm::vec4 sunLightPositionInCameraSpace = modelMatrix.Top()
				* sunPosition;
		glm::vec4 carLightPositionInCameraSpace = modelMatrix.Top()
				* glm::vec4(carLightPosition, 1.0f);

		glUseProgram(program.theProgram);
		glUniform4fv(program.sunLightIntensityUniform, 1,
				glm::value_ptr(glm::vec4(sunLightIntensity, 1.0f)));
		glUniform4fv(program.sunAmbientIntensityUniform, 1,
				glm::value_ptr(glm::vec4(sunAmbientIntensity, 1.0f)));
		glUniform4fv(program.carLightIntensityUniform, 1,
				glm::value_ptr(glm::vec4(carLightIntensity, 1.0f)));
		glUniform4fv(program.carAmbientIntensityUniform, 1,
				glm::value_ptr(glm::vec4(carAmbientIntensity, 1.0f)));
		glUseProgram(0);

		{
			glutil::PushStack push(modelMatrix);
			renderMesh(planeMesh, modelMatrix, sunLightPositionInCameraSpace,
					carLightPositionInCameraSpace, planeColor);
		}

		{
			glutil::PushStack push(modelMatrix);
			modelMatrix.Translate(glm::vec3(sunPosition));
			renderLightMesh(sunMesh, modelMatrix, sunLightColor);
		}

		{
			glutil::PushStack push(modelMatrix);
			modelMatrix.ApplyMatrix(carObjectPole.CalcMatrix());
			modelMatrix.Translate(carPosition);
			modelMatrix.RotateY(carAngleInDegrees);
			renderMesh(carBodyMesh, modelMatrix, sunLightPositionInCameraSpace,
					carLightPositionInCameraSpace, carBodyColor);
		}

		{
			glutil::PushStack push(modelMatrix);
			modelMatrix.Translate(carLightPosition);
			modelMatrix.Scale(0.5f, 0.5f, 0.5f);
			renderLightMesh(carLightMesh, modelMatrix, carLightColor);
		}

		{
			glutil::PushStack push(modelMatrix);
			modelMatrix.Translate(cubePosition);
			renderMesh(cubeMesh, modelMatrix, sunLightPositionInCameraSpace,
					carLightPositionInCameraSpace, cubeColor);
		}

		{
			glutil::PushStack push(modelMatrix);
			modelMatrix.Translate(cylinderPosition);
			renderMesh(cylinderMesh, modelMatrix, sunLightPositionInCameraSpace,
					carLightPositionInCameraSpace, cylinderColor);
		}

		{
			glutil::PushStack push(modelMatrix);
			modelMatrix.Translate(spherePosition);
			renderMesh(sphereMesh, modelMatrix, sunLightPositionInCameraSpace,
					carLightPositionInCameraSpace, sphereColor);
		}
	}

	glutPostRedisplay();
	glutSwapBuffers();
}

void reshape(int width, int height) {
	glutil::MatrixStack perspectiveMatrix;
	perspectiveMatrix.Perspective(45.0f, (width / (float) height), zNear, zFar);

	ProjectionBlock projectionData;
	projectionData.cameraToClipMatrix = perspectiveMatrix.Top();

	glBindBuffer(GL_UNIFORM_BUFFER, projectionUniformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ProjectionBlock),
			&projectionData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		delete planeMesh;
		delete carBodyMesh;
		delete carLightMesh;
		delete cubeMesh;
		delete cylinderMesh;
		delete sphereMesh;
		glutLeaveMainLoop();
		return;
	case 'w':
		calculateCarPosition(1.0f);
		break;
	case 's':
		calculateCarPosition(-1.0f);
		break;
	case 'a':
		carAngleInDegrees += 5.0f;
		break;
	case 'd':
		carAngleInDegrees -= 5.0f;
		break;
	}
	calculateCarLightPosition();
	glutPostRedisplay();
}

unsigned int defaults(unsigned int displayMode, int &width, int &height) {
	return displayMode;
}

