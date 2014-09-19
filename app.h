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
    GLuint ufoLightIntensityUniform;
    GLuint ufoAmbientIntensityUniform;

    GLuint sunLightPositionInModelSpaceUniform;
    GLuint sunLightIntensityUniform;
    GLuint sunAmbientIntensityUniform;
};

struct ProjectionBlock {
    glm::mat4 cameraToClipMatrix;
};

#endif

