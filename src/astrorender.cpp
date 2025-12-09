#include "astrorender.h"

#include <QCoreApplication>
#include "src/utils/shaderloader.h"

#include <QMouseEvent>
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"


AstroRender::AstroRender(Scene scene, QWidget *parent)
    : QOpenGLWidget(parent),
    m_angleX(6),
    m_angleY(0),
    m_zoom(2.0f),
    m_hdrFBO(0),
    m_hdrColorTex(0),
    m_hdrDepthRBO(0),
    m_quadVAO(0),
    m_quadVBO(0),
    m_phong_shader(0),
    m_tonemap_shader(0),
    m_scene(scene)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;
}

AstroRender::~AstroRender()
{
    makeCurrent();
    finish();
    doneCurrent();
}

void AstroRender::finish()
{
    killTimer(m_timer);

    // Delete GL programs
    if (m_phong_shader) glDeleteProgram(m_phong_shader);
    if (m_tonemap_shader) glDeleteProgram(m_tonemap_shader);

    // Delete fullscreen quad
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);

    // Delete HDR framebuffer resources
    if (m_hdrColorTex) glDeleteTextures(1, &m_hdrColorTex);
    if (m_hdrDepthRBO) glDeleteRenderbuffers(1, &m_hdrDepthRBO);
    if (m_hdrFBO) glDeleteFramebuffers(1, &m_hdrFBO);
}

void AstroRender::initializeGL()
{
    m_devicePixelRatio = this->devicePixelRatio();
    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();
    m_defaultFBO = 2;

    // initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
        fprintf(stderr, "Error while initializing GLEW: %s\n", glewGetErrorString(err));
    fprintf(stdout, "Successfully initialized GLEW %s\n", glewGetString(GLEW_VERSION));

    // GL default state
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST); // raymarch doesn't need standard depth test
    glEnable(GL_CULL_FACE);

    // Compile shaders
    m_phong_shader   = ShaderLoader::createShaderProgram(":/resources/shaders/phong.vert",   ":/resources/shaders/phong.frag");
    m_tonemap_shader = ShaderLoader::createShaderProgram(":/resources/shaders/tonemap.vert",   ":/resources/shaders/tonemap.frag");

    // Fullscreen quad: position XY, uv
    float quadVertices[] = {
        // pos      // uv
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // pos attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // uv attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Setup sizes and HDR FBO
    m_screen_width  = width()  * m_devicePixelRatio;
    m_screen_height = height() * m_devicePixelRatio;
    createHDRFramebuffer(m_screen_width, m_screen_height);

    // Set tonemap shader sampler once
    glUseProgram(m_tonemap_shader);
    GLint loc = glGetUniformLocation(m_tonemap_shader, "uHDRTexture");
    if (loc >= 0) glUniform1i(loc, 0);
    glUseProgram(0);
}

// HDR Framebuffer Creation
void AstroRender::createHDRFramebuffer(int w, int h)
{
    // delete old resources if they exist
    if (m_hdrColorTex) { glDeleteTextures(1, &m_hdrColorTex); m_hdrColorTex = 0; }
    if (m_hdrDepthRBO) { glDeleteRenderbuffers(1, &m_hdrDepthRBO); m_hdrDepthRBO = 0; }
    if (m_hdrFBO)      { glDeleteFramebuffers(1, &m_hdrFBO); m_hdrFBO = 0; }

    m_screen_width  = w;
    m_screen_height = h;

    // Create float color texture (HDR)
    glGenTextures(1, &m_hdrColorTex);
    glBindTexture(GL_TEXTURE_2D, m_hdrColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_screen_width, m_screen_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Clamp to edge to avoid wrap seams
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create depth renderbuffer (optional - but good practice)
    glGenRenderbuffers(1, &m_hdrDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_hdrDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_screen_width, m_screen_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Create framebuffer and attach
    glGenFramebuffers(1, &m_hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_hdrColorTex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_hdrDepthRBO);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR: HDR Framebuffer not complete (status 0x%x)\n", status);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AstroRender::paintGL()
{
    // 1) Render scene into the HDR framebuffer using phong shader
    renderSceneHDR();

    // 2) Tone-map the HDR texture to the default framebuffer
    toneMapToScreen();
}

void AstroRender::uploadLights(GLuint shader) {

    // std::cout << "Will upload " << m_scene.renderData.lights.size() << " lights " << std::endl;

    // Upload global properties
    float ka = m_scene.renderData.globalData.ka;
    float kd = m_scene.renderData.globalData.kd;
    float ks = m_scene.renderData.globalData.ks;
    glUniform1f(glGetUniformLocation(shader, "ka"), ka);
    glUniform1f(glGetUniformLocation(shader, "kd"), kd);
    glUniform1f(glGetUniformLocation(shader, "ks"), ks);

    // Upload lights
    int numLights = std::fmin(m_scene.renderData.lights.size(), 8);
    glUniform1i(glGetUniformLocation(m_phong_shader, "numLights"), (GLint) numLights);


    for (int i = 0; i < numLights; ++i) {
        auto& light = m_scene.renderData.lights[i];
        std::string prefix = "lights[" + std::to_string(i) + "].";
        glUniform1i(glGetUniformLocation(shader, (prefix + "type").c_str()), (int) light.type);
        glUniform3f(glGetUniformLocation(shader, (prefix + "color").c_str()), light.color.r, light.color.g, light.color.b);
        glUniform3f(glGetUniformLocation(shader, (prefix + "function").c_str()), light.function.x, light.function.y, light.function.z);
        glUniform4f(glGetUniformLocation(shader, (prefix + "pos").c_str()), light.pos.x, light.pos.y, light.pos.z, light.pos.w);
        glUniform4f(glGetUniformLocation(shader, (prefix + "dir").c_str()), light.dir.x, light.dir.y, light.dir.z, light.dir.w);
        glUniform1f(glGetUniformLocation(shader, (prefix + "penumbra").c_str()), light.penumbra);
        glUniform1f(glGetUniformLocation(shader, (prefix + "angle").c_str()), light.angle);
    }
}

void AstroRender::uploadShapes(GLuint shader)
{
    int numShapes = std::min((int)m_scene.renderData.shapes.size(), 8);
    // std::cout << "Will upload " << numShapes << " shapes " << std::endl;

    glUniform1i(glGetUniformLocation(shader, "numShapes"), numShapes);

    for (int i = 0; i < numShapes; i++) {
        const auto& shape = m_scene.renderData.shapes[i];
        std::string base = "shapes[" + std::to_string(i) + "].";

        // std::cout << "primtive type was " << (int) shape.primitive.type << std::endl;

        // primitive type
        glUniform1i(glGetUniformLocation(shader, (base + "primitive").c_str()),(GLuint) shape.primitive.type);

        // inverse CTM
        glUniformMatrix4fv(glGetUniformLocation(shader, (base + "invCTM").c_str()),1, GL_FALSE, &glm::inverse(shape.ctm)[0][0]);

        // material properties: ambient/diffuse/specular and shininess
        glUniform3f(glGetUniformLocation(shader, (base + "ambient").c_str()),shape.primitive.material.cAmbient.r, shape.primitive.material.cAmbient.g, shape.primitive.material.cAmbient.b);
        glUniform3f(glGetUniformLocation(shader, (base + "diffuse").c_str()),shape.primitive.material.cDiffuse.r, shape.primitive.material.cDiffuse.g, shape.primitive.material.cDiffuse.b);
        glUniform3f(glGetUniformLocation(shader, (base + "specular").c_str()),shape.primitive.material.cSpecular.r, shape.primitive.material.cSpecular.g, shape.primitive.material.cSpecular.b);
        glUniform1f(glGetUniformLocation(shader, (base + "shininess").c_str()),shape.primitive.material.shininess);
    }
}


void AstroRender::renderSceneHDR()
{
    // Bind HDR FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use phong shader
    glUseProgram(m_phong_shader);

    //Upload all shape information
    uploadShapes(m_phong_shader);

    //Uplaod lights
    uploadLights(m_phong_shader);

    //Upload all other informatio: projection + view matrices, and camera pos
    GLuint locView = glGetUniformLocation(m_phong_shader, "uView"); //view matrix
    glUniformMatrix4fv(locView, 1, GL_FALSE, &m_scene.camera.getViewMatrix()[0][0]);
    GLuint locProj = glGetUniformLocation(m_phong_shader, "uProj"); //projection matrix
    glUniformMatrix4fv(locProj, 1, GL_FALSE, &m_scene.camera.getProjectionMatrix()[0][0]);
    glm::vec3 camPos = m_scene.camera.cameraData.pos; //camera pos
    glUniform3f(glGetUniformLocation(m_phong_shader, "uCameraPos"), camPos.x, camPos.y, camPos.z);

    //Draw scene
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);


}

void AstroRender::toneMapToScreen()
{
    // Bind default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, width() * m_devicePixelRatio, height() * m_devicePixelRatio);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_tonemap_shader);

    // Bind HDR texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_hdrColorTex);


    // draw fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void AstroRender::resizeGL(int w, int h)
{
    // update device pixel scaled size
    m_devicePixelRatio = this->devicePixelRatio();
    m_screen_width  = w * m_devicePixelRatio;
    m_screen_height = h * m_devicePixelRatio;

    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    m_scene.camera.viewPlaneWidth = w;
    m_scene.camera.viewPlaneHeight = h;

    // Recreate HDR FBO at new size
    createHDRFramebuffer(m_screen_width, m_screen_height);
}

// ================== Camera Movement!

void AstroRender::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void AstroRender::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void AstroRender::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void AstroRender::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

glm::mat4 rodriguesMatrix(const glm::vec3 &axis, float angle) {
    glm::vec3 k = glm::normalize(axis);
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0f - c;

    float kx = k.x, ky = k.y, kz = k.z;

    glm::mat4 R(1.0f); // identity 4x4
    R[0][0] = t*kx*kx + c;     R[1][0] = t*kx*ky - s*kz;  R[2][0] = t*kx*kz + s*ky;
    R[0][1] = t*kx*ky + s*kz;  R[1][1] = t*ky*ky + c;     R[2][1] = t*ky*kz - s*kx;
    R[0][2] = t*kx*kz - s*ky;  R[1][2] = t*ky*kz + s*kx;  R[2][2] = t*kz*kz + c;
    return R;
}


void AstroRender::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        auto& cam = m_scene.camera;

        glm::vec3 forward = glm::normalize(cam.cameraData.look);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(cam.cameraData.up)));

        float sensitivity = 0.005f;
        float yaw   = -deltaX * sensitivity;
        float pitch = -deltaY * sensitivity;

        // Turns camera left to right
        glm::mat4 yawMat = rodriguesMatrix(worldUp, yaw);
        glm::vec4 f_yaw = yawMat * glm::vec4(forward, 0.0f);
        forward = glm::normalize(glm::vec3(f_yaw));

        // Rotates camera up and down
        right = glm::normalize(glm::cross(forward, worldUp));
        glm::mat4 pitchMat = rodriguesMatrix(right, pitch);
        glm::vec4 f_pitch = pitchMat * glm::vec4(forward, 0.0f);
        forward = glm::normalize(glm::vec3(f_pitch));

        // Update camera
        cam.cameraData.look = glm::vec4(forward, 1.0);
        cam.cameraData.up   = glm::vec4(glm::normalize(glm::cross(right, forward)), 1.0);
    }

    update(); // asks for a PaintGL() call to occur

}

void AstroRender::timerEvent(QTimerEvent *event) {
    float speed = 5.0f; // units per second
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    auto& cam = m_scene.camera;

    glm::vec3 look = glm::normalize(cam.cameraData.look);
    glm::vec3 right   = glm::normalize(glm::cross(look, glm::vec3(cam.cameraData.up)));
    glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);

    if (m_keyMap[Qt::Key_W]) cam.cameraData.pos += glm::vec4(look * speed * deltaTime, 1.0);
    if (m_keyMap[Qt::Key_S]) cam.cameraData.pos -= glm::vec4(look * speed * deltaTime, 1.0);
    if (m_keyMap[Qt::Key_A]) cam.cameraData.pos -= glm::vec4(right * speed * deltaTime, 1.0);
    if (m_keyMap[Qt::Key_D]) cam.cameraData.pos += glm::vec4(right * speed * deltaTime, 1.0);
    if (m_keyMap[Qt::Key_Space]) cam.cameraData.pos += glm::vec4(up * speed * deltaTime, 1.0);
    if (m_keyMap[Qt::Key_Control]) cam.cameraData.pos -= glm::vec4(up * speed * deltaTime, 1.0);



    update(); // asks for a PaintGL() call to occur
}
