#include <cmath>
#include <cstdio>

#include <string>
#include <vector>
#include <stack>

#include <glload/gl_3_3.h>
#include <glutil/glutil.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/freeglut.h>

#include "framework/framework.h"
#include "framework/Mesh.h"
#include "framework/MousePole.h"
#include "framework/Timer.h"

#include "app.h"

float zNear = 1.0f;
float zFar = 1000.0f;

ProgramData program;
SimpleProgramData lightProgram;

Framework::Mesh *planeMesh = NULL;
Framework::Mesh *sunMesh = NULL;
Framework::Mesh *ufoBodyMesh = NULL;
Framework::Mesh *ufoLightMesh = NULL;
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

glm::vec3 ufoPosition(0.0f, 0.5f, 0.0f);
glm::vec3 ufoLightPosition(0.0f, 0.5f, 6.0f);
glm::vec3 cubePosition = glm::vec3(9.0f, 0.0f, 19.0f);
glm::vec3 cylinderPosition = glm::vec3(23.0f, -0.5f, -26.0f);
glm::vec3 spherePosition = glm::vec3(-20.0f, 1.5f, -12.0f);
float ufoAngleInDegrees = 0.0f;

glutil::ObjectData ufoObjectData = {
    ufoPosition,
    glm::fquat(1.0f, 0.0f, 0.0f, 0.0f)
};

glutil::ObjectPole ufoObjectPole = glutil::ObjectPole(ufoObjectData, 1.0f,
        glutil::MB_RIGHT_BTN, &viewPole);

const int projectionBlockIndex = 2;
const int unprojectionBlockIndex = 1;
GLuint projectionUniformBuffer = 0;
GLuint unprojectionUniformBuffer = 0;

glm::vec3 sunLightIntensity = glm::vec3(5.0f, 5.0f, 5.0f);
glm::vec3 sunAmbientIntensity = glm::vec3(0.3f, 0.3f, 0.3f);
glm::vec3 ufoLightIntensity = glm::vec3(8.0f, 8.0f, 8.0f);
glm::vec3 ufoAmbientIntensity = glm::vec3(0.2f, 0.2f, 0.2f);

glm::vec3 planeColor = glm::vec3(0.2f, 0.8f, 0.3f);
glm::vec3 sunLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 ufoBodyColor = glm::vec3(0.9f, 0.1f, 0.3f);
glm::vec3 ufoLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 cubeColor = glm::vec3(0.2f, 0.2f, 1.0f);
glm::vec3 cylinderColor = glm::vec3(0.9f, 0.9f, 0.0f);
glm::vec3 sphereColor = glm::vec3(0.9f, 0.2f, 0.7f);

float sunLightRadius = 50.0f;
float sunLightMaxHeight = 120.0f;
Framework::Timer sunLightTimer(Framework::Timer::TT_LOOP, 30.0f);

float ufoRadius = 6.71f;
float cubePlusUfoRadius = 7.0711f + ufoRadius;
float cylinderPlusUfoRadius = 3.0f + ufoRadius;
float spherePlusUfoRadius = 4.5f + ufoRadius;

static float g_fSunAttenuation = 0.001f;
static float g_fUfoAttenuation = 0.05f;

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

static void calculateUfoLightPosition() {
    float angleInRadians = Framework::DegToRad(ufoAngleInDegrees);
    ufoLightPosition = glm::vec3(ufoPosition.x + 6.0f * sinf(angleInRadians),
            ufoPosition.y, ufoPosition.z + 6.0f * cosf(angleInRadians));
}

static bool newUfoPositionIsFine(glm::vec3 pos) {
    float cubeUfoDistance = sqrtf(
            powf(pos.x - cubePosition.x, 2.0f)
            + powf(pos.y - cubePosition.y, 2.0f)
            + powf(pos.z - cubePosition.z, 2.0f));
    float cylinderUfoDistance = sqrtf(
            powf(pos.x - cylinderPosition.x, 2.0f)
            + powf(pos.y - cylinderPosition.y, 2.0f)
            + powf(pos.z - cylinderPosition.z, 2.0f));
    float sphereUfoDistance = sqrtf(
            powf(pos.x - spherePosition.x, 2.0f)
            + powf(pos.y - spherePosition.y, 2.0f)
            + powf(pos.z - spherePosition.z, 2.0f));
    return cubeUfoDistance - cubePlusUfoRadius > 0.0f
        && cylinderUfoDistance - cylinderPlusUfoRadius > 0.0f
        && sphereUfoDistance - spherePlusUfoRadius > 0.0f
        && pos.x <= 50.0f - ufoRadius
        && pos.x >= ufoRadius - 50.0f
        && pos.y <= 50.0f - ufoRadius
        && pos.y >= 0.0f
        && pos.z <= 50.0f - ufoRadius
        && pos.z >= ufoRadius - 50.0f;
}

static void calculateUfoPosition(float direction, float height_change) {
    float angleInRadians = Framework::DegToRad(ufoAngleInDegrees);
    glm::vec3 pos;
    pos.z = ufoPosition.z + direction * 0.5f * cosf(angleInRadians);
    pos.x = ufoPosition.x + direction * 0.5f * sinf(angleInRadians);
    pos.y = ufoPosition.y + height_change;
    if (newUfoPositionIsFine(pos)) {
        ufoPosition = pos;
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
    programData.cameraSpaceSunPosition = glGetUniformLocation(
            programData.theProgram, "cameraSpaceSunPosition");
    programData.sunLightIntensityUniform = glGetUniformLocation(
            programData.theProgram, "sunLightIntensity");
    programData.sunAmbientIntensityUniform = glGetUniformLocation(
            programData.theProgram, "sunAmbientIntensity");

    programData.ufoLightPositionInModelSpaceUniform = glGetUniformLocation(
            programData.theProgram, "ufoLightPositionInModelSpace");
    programData.cameraSpaceUfoPosition = glGetUniformLocation(
            programData.theProgram, "cameraSpaceUfoPosition");
    programData.ufoLightIntensityUniform = glGetUniformLocation(
            programData.theProgram, "ufoLightIntensity");
    programData.ufoAmbientIntensityUniform = glGetUniformLocation(
            programData.theProgram, "ufoAmbientIntensity");

    programData.sunAttenuationUnif = glGetUniformLocation(programData.theProgram, "sunAttenuation");
    programData.ufoAttenuationUnif = glGetUniformLocation(programData.theProgram, "ufoAttenuation");

    GLuint projectionBlock = glGetUniformBlockIndex(programData.theProgram,
            "Projection");
    glUniformBlockBinding(programData.theProgram, projectionBlock,
            projectionBlockIndex);
    GLuint unprojectionBlock = glGetUniformBlockIndex(programData.theProgram,
            "UnProjection");
    glUniformBlockBinding(programData.theProgram, unprojectionBlock,
            unprojectionBlockIndex);

    return programData;
}

void initializeProgramsAndMeshes() {
    program = initializeProgram("VertexShader.vert", "FragmentShader.frag");
    lightProgram = initializeSimpleProgram("LightVertexShader.vert",
            "LightFragmentShader.frag");

    try {
        planeMesh = new Framework::Mesh("Plane.xml");
        sunMesh = new Framework::Mesh("Sphere.xml");
        ufoBodyMesh = new Framework::Mesh("Ship.xml");
        ufoLightMesh = new Framework::Mesh("Sphere.xml");
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
    glGenBuffers(1, &unprojectionUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, projectionUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionBlock), NULL,
            GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, projectionBlockIndex,
            projectionUniformBuffer, 0, sizeof(ProjectionBlock));

    glBindBuffer(GL_UNIFORM_BUFFER, unprojectionUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UnProjectionBlock), NULL,
            GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, unprojectionBlockIndex,
            unprojectionUniformBuffer, 0, sizeof(UnProjectionBlock));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void renderMesh(const Framework::Mesh* mesh,
        const glutil::MatrixStack& modelMatrix,
        const glm::vec4& sunLightPositionInCameraSpace,
        const glm::vec4& ufoLightPositionInCameraSpace,
        const glm::vec3& color) {

    glm::mat4 invertedModelMatrix = glm::inverse(modelMatrix.Top());
    glm::vec4 sunLightPositionInModelSpace = invertedModelMatrix
        * sunLightPositionInCameraSpace;
    glm::vec4 ufoLightPositionInModelSpace = invertedModelMatrix
        * ufoLightPositionInCameraSpace;

    glUseProgram(program.theProgram);
    glUniformMatrix4fv(program.modelToCameraMatrixUniform, 1, GL_FALSE,
            glm::value_ptr(modelMatrix.Top()));
    glUniform3fv(program.sunLightPositionInModelSpaceUniform, 1,
            glm::value_ptr(sunLightPositionInModelSpace));
    glUniform3fv(program.ufoLightPositionInModelSpaceUniform, 1,
            glm::value_ptr(ufoLightPositionInModelSpace));
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

    if (ufoBodyMesh && planeMesh && ufoLightMesh && cubeMesh && cylinderMesh
            && sphereMesh) {
        glutil::MatrixStack modelMatrix;
        modelMatrix.SetMatrix(viewPole.CalcMatrix());

        glm::vec4 sunPosition = calculateSunPosition();
        const glm::vec4 &sunLightPositionInCameraSpace = modelMatrix.Top()
            * sunPosition;
        const glm::vec4 &ufoLightPositionInCameraSpace = modelMatrix.Top()
            * glm::vec4(ufoLightPosition, 1.0f);
        printf("%s\n", glm::to_string(ufoLightPositionInCameraSpace).c_str());

        glUseProgram(program.theProgram);
        glUniform4fv(program.sunLightIntensityUniform, 1,
                glm::value_ptr(glm::vec4(sunLightIntensity, 1.0f)));
        glUniform3fv(program.cameraSpaceSunPosition, 1,
                glm::value_ptr(sunLightPositionInCameraSpace));
        glUniform4fv(program.sunAmbientIntensityUniform, 1,
                glm::value_ptr(glm::vec4(sunAmbientIntensity, 1.0f)));

        glUniform4fv(program.ufoLightIntensityUniform, 1,
                glm::value_ptr(glm::vec4(ufoLightIntensity, 1.0f)));
        glUniform3fv(program.cameraSpaceUfoPosition, 1,
                glm::value_ptr(ufoLightPositionInCameraSpace));
        glUniform4fv(program.ufoAmbientIntensityUniform, 1,
                glm::value_ptr(glm::vec4(ufoAmbientIntensity, 1.0f)));
        glUniform1f(program.sunAttenuationUnif, g_fSunAttenuation);
        glUniform1f(program.ufoAttenuationUnif, g_fUfoAttenuation);
        glUseProgram(0);

        {
            glutil::PushStack push(modelMatrix);
            renderMesh(planeMesh, modelMatrix, sunLightPositionInCameraSpace,
                    ufoLightPositionInCameraSpace, planeColor);
        }

        {
            glutil::PushStack push(modelMatrix);
            modelMatrix.Translate(glm::vec3(sunPosition));
            renderLightMesh(sunMesh, modelMatrix, sunLightColor);
        }

        {
            glutil::PushStack push(modelMatrix);
            modelMatrix.ApplyMatrix(ufoObjectPole.CalcMatrix());
            modelMatrix.Translate(ufoPosition);
            modelMatrix.RotateY(ufoAngleInDegrees);
            renderMesh(ufoBodyMesh, modelMatrix, sunLightPositionInCameraSpace,
                    ufoLightPositionInCameraSpace, ufoBodyColor);
        }

        {
            glutil::PushStack push(modelMatrix);
            modelMatrix.Translate(ufoLightPosition);
            modelMatrix.Scale(0.5f, 0.5f, 0.5f);
            renderLightMesh(ufoLightMesh, modelMatrix, ufoLightColor);
        }

        {
            glutil::PushStack push(modelMatrix);
            modelMatrix.Translate(cubePosition);
            renderMesh(cubeMesh, modelMatrix, sunLightPositionInCameraSpace,
                    ufoLightPositionInCameraSpace, cubeColor);
        }

        {
            glutil::PushStack push(modelMatrix);
            modelMatrix.Translate(cylinderPosition);
            renderMesh(cylinderMesh, modelMatrix, sunLightPositionInCameraSpace,
                    ufoLightPositionInCameraSpace, cylinderColor);
        }

        {
            glutil::PushStack push(modelMatrix);
            modelMatrix.Translate(spherePosition);
            renderMesh(sphereMesh, modelMatrix, sunLightPositionInCameraSpace,
                    ufoLightPositionInCameraSpace, sphereColor);
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

    UnProjectionBlock unprojData;
    unprojData.clipToCameraMatrix = glm::inverse(perspectiveMatrix.Top());
    unprojData.windowSize = glm::ivec2(width, height);

    glBindBuffer(GL_UNIFORM_BUFFER, projectionUniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ProjectionBlock),
            &projectionData);
    glBindBuffer(GL_UNIFORM_BUFFER, unprojectionUniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UnProjectionBlock),
            &unprojData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            delete planeMesh;
            delete ufoBodyMesh;
            delete ufoLightMesh;
            delete cubeMesh;
            delete cylinderMesh;
            delete sphereMesh;
            glutLeaveMainLoop();
            return;
        case 'w':
            calculateUfoPosition(1.0f, 0.0f);
            break;
        case 's':
            calculateUfoPosition(-1.0f, 0.0f);
            break;
        case 'e':
            calculateUfoPosition(0, 1.0f);
            break;
        case 'q':
            calculateUfoPosition(0, -1.0f);
            break;
        case 'a':
            ufoAngleInDegrees += 5.0f;
            break;
        case 'd':
            ufoAngleInDegrees -= 5.0f;
            break;
    }
    calculateUfoLightPosition();
    glutPostRedisplay();
}

unsigned int defaults(unsigned int displayMode, int &width, int &height) {
    return displayMode;
}

