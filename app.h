#ifndef _APP_H_
#define _APP_H_

static void calculateUfoLightPosition();
static bool newUfoPositionIsFine(glm::vec3 pos);
static void calculateUfoPosition(float direction, float height_change);

struct SimpleProgramData {
    GLuint theProgram;
    GLuint objectColorUniform;
    GLuint modelToCameraMatrixUniform;
};

struct ProgramData: SimpleProgramData {
    GLuint ufoLightPositionInModelSpaceUniform;
    GLuint cameraSpaceUfoPosition;
    GLuint ufoLightIntensityUniform;
    GLuint ufoAmbientIntensityUniform;
    GLuint ufoAttenuationUnif;

    GLuint sunLightPositionInModelSpaceUniform;
    GLuint cameraSpaceSunPosition;
    GLuint sunLightIntensityUniform;
    GLuint sunAmbientIntensityUniform;
    GLuint sunAttenuationUnif;
};

struct ProjectionBlock {
    glm::mat4 cameraToClipMatrix;
};

struct UnProjectionBlock {
    glm::mat4 clipToCameraMatrix;
    glm::ivec2 windowSize;
};

#endif

