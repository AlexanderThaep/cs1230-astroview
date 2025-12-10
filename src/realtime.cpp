#include "realtime.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include "settings.h"
#include <iostream>

#include "shaderloader.h"
#include "utils/shapeloader.h"

inline static float aspectRatio(int width, int height)
{
    return (float) width / (float) height;
}

// ================== Rendering the Scene!

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width() / 2, size().height() / 2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W] = false;
    m_keyMap[Qt::Key_A] = false;
    m_keyMap[Qt::Key_S] = false;
    m_keyMap[Qt::Key_D] = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space] = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish()
{
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    clearShapes(m_render);
    glDeleteProgram(m_phong_shader);
    glDeleteProgram(m_tonemap_shader);

    // Delete fullscreen quad
    glDeleteBuffers(1, &m_quadVBO);
    glDeleteVertexArrays(1, &m_quadVAO);

    // Delete HDR framebuffer resources
    glDeleteTextures(1, &m_hdrColorTex);
    glDeleteRenderbuffers(1, &m_hdrDepthRBO);
    glDeleteFramebuffers(1, &m_hdrFBO);

    this->doneCurrent();
}

void Realtime::initializeGL()
{
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000 / 60);
    m_elapsedTimer.start();
    m_defaultFBO = 2;

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    if (!settings.sceneFilePath.empty()) sceneChanged();

    // Allows OpenGL to draw objects appropriately on top of one another
    glDisable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    glClearColor(0, 0, 0, 1);

    // Compile shaders
    m_phong_shader = ShaderLoader::createShaderProgram(":/resources/shaders/phong.vert",
                                                       ":/resources/shaders/phong.frag");
    m_tonemap_shader = ShaderLoader::createShaderProgram(":/resources/shaders/tonemap.vert",
                                                         ":/resources/shaders/tonemap.frag");

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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);

    // uv attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Setup sizes and HDR FBO
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    createHDRFramebuffer(m_screen_width, m_screen_height);

    // Set tonemap shader sampler once
    glUseProgram(m_tonemap_shader);
    GLint loc = glGetUniformLocation(m_tonemap_shader, "uHDRTexture");
    if (loc >= 0) glUniform1i(loc, 0);
    glUseProgram(0);
}

// HDR Framebuffer Creation
void Realtime::createHDRFramebuffer(int w, int h)
{
    // delete old resources if they exist
    if (m_hdrColorTex) { glDeleteTextures(1, &m_hdrColorTex); m_hdrColorTex = 0; }
    if (m_hdrDepthRBO) { glDeleteRenderbuffers(1, &m_hdrDepthRBO); m_hdrDepthRBO = 0; }
    if (m_hdrFBO)      { glDeleteFramebuffers(1, &m_hdrFBO); m_hdrFBO = 0; }

    m_screen_width = w;
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

void Realtime::renderSceneHDR()
{
    // Bind HDR FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use phong shader
    glUseProgram(m_phong_shader);

    // Upload shape information
    ShaderLoader::passShaderValues(m_phong_shader, m_render);
    ShaderLoader::passLightValues(m_phong_shader, m_render);
    ShaderLoader::passCameraValues(m_phong_shader, *m_cam);

    //Draw scene
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::toneMapToScreen()
{
    // Bind default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
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

void Realtime::paintGL()
{
    // Students: anything requiring OpenGL calls every frame should be done here
    if (!m_cam) return;

    renderSceneHDR();
    toneMapToScreen();
}

void Realtime::resizeGL(int w, int h)
{
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    m_screen_width = w * m_devicePixelRatio;
    m_screen_height = h * m_devicePixelRatio;

    // Students: anything requiring OpenGL calls when the program starts should be done here

    if (m_cam) m_cam->update(m_render.cameraData, aspectRatio(w, h));

    createHDRFramebuffer(m_screen_width, m_screen_height);
}

void Realtime::sceneChanged()
{
    this->makeCurrent();
    clearShapes(m_render);

    RenderData data;
    bool success = SceneParser::parse(settings.sceneFilePath, data);

    if (!success) {
        std::cerr << "Error loading scene: \"" << settings.sceneFilePath << "\"" << std::endl;
        return;
    }

    std::cout << "Loaded scenefile: \"" << settings.sceneFilePath << "\"." << std::endl;

    m_render = data;
    updateShapes(m_render);

    if (!m_cam) m_cam = new Camera(m_render.cameraData);
    m_cam->update(m_render.cameraData, aspectRatio(size().width(), size().height()));
    m_cam->updateView(m_render.cameraData);

    update(); // asks for a PaintGL() call to occur
}

void Realtime::CameraSettingsChanged()
{
    if (m_cam) m_cam->update(m_render.cameraData, aspectRatio(size().width(), size().height()));

    this->makeCurrent();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::SceneSettingsChanged()
{
    this->makeCurrent();

    update(); // asks for a PaintGL() call to occur
}

// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event)
{
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event)
{
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        m_deltaX = posX - m_prev_mouse_pos.x;
        m_deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);
    }
}

void Realtime::timerEvent(QTimerEvent *event)
{
    int elapsedms = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    if (m_cam) {
        if (m_keyMap[Qt::Key_W] && !m_keyMap[Qt::Key_S]) m_cam->moveMedial(m_render.cameraData, MOVEMENT_SPEED * deltaTime);
        if (m_keyMap[Qt::Key_S] && !m_keyMap[Qt::Key_W]) m_cam->moveMedial(m_render.cameraData, -MOVEMENT_SPEED * deltaTime);
        if (m_keyMap[Qt::Key_A] && !m_keyMap[Qt::Key_D]) m_cam->moveLateral(m_render.cameraData, -MOVEMENT_SPEED * deltaTime);
        if (m_keyMap[Qt::Key_D] && !m_keyMap[Qt::Key_A]) m_cam->moveLateral(m_render.cameraData, MOVEMENT_SPEED * deltaTime);

        if (m_keyMap[Qt::Key_Control] && !m_keyMap[Qt::Key_Space]) m_cam->moveVertical(m_render.cameraData, -MOVEMENT_SPEED * deltaTime);
        if (m_keyMap[Qt::Key_Space] && !m_keyMap[Qt::Key_Control]) m_cam->moveVertical(m_render.cameraData, MOVEMENT_SPEED * deltaTime);

        // Use deltaX and deltaY here to rotate
        if (m_mouseDown) {
            m_cam->rotate(m_render.cameraData, X_ROTATION_SENS * -m_deltaX, Y_ROTATION_SENS * -m_deltaY);
        }
    }

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath)
{
    // Make sure we have the right context and everything has been drawn
    this->makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.flipped(Qt::Vertical); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &fbo);
}
