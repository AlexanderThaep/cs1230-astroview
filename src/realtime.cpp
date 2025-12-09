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
    glDeleteProgram(m_shader);

    this->doneCurrent();
}

void Realtime::initializeGL()
{
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000 / 60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    glClearColor(0, 0, 0, 1);

    m_shader = ShaderLoader::createShaderProgram("resources/shaders/default.vert", "resources/shaders/default.frag");
}

void Realtime::paintGL()
{
    // Students: anything requiring OpenGL calls every frame should be done here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shader);

    GLuint loc;

    for (RenderShapeData &o : m_render.shapes) {
        glBindVertexArray(o.vao);

        ShaderLoader::passShaderValues(m_shader, o, m_render, m_cam);
        ShaderLoader::passLightValues(m_shader, m_render);

        glDrawArrays(GL_TRIANGLES, 0, o.vertexCount / 3);

        glBindVertexArray(0);
    }

    glBindVertexArray(0);

    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h)
{
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    if (m_cam) m_cam->update(m_render.cameraData, aspectRatio(w, h));
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

    m_render = data;
    updateShapes(m_render, settings.extraCredit1);

    if (!m_cam) m_cam = new Camera(m_render.cameraData);
    m_cam->update(m_render.cameraData, aspectRatio(size().width(), size().height()));
    m_cam->updateView(m_render.cameraData);

    update(); // asks for a PaintGL() call to occur
}

void Realtime::CameraSettingsChanged()
{
    if (m_cam) m_cam->update(m_render.cameraData, aspectRatio(size().width(), size().height()));

    this->makeCurrent();
    updateShapes(m_render, settings.extraCredit1);

    update(); // asks for a PaintGL() call to occur
}

void Realtime::SceneSettingsChanged()
{
    this->makeCurrent();
    updateShapes(m_render, settings.extraCredit1);

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
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        if (m_cam && m_mouseDown) {
            m_cam->rotate(m_render.cameraData, X_ROTATION_SENS * -deltaX, Y_ROTATION_SENS * -deltaY);
        }

        update(); // asks for a PaintGL() call to occur
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
    }

    if (settings.extraCredit1
        && (m_keyMap[Qt::Key_W]
        || m_keyMap[Qt::Key_D]
        || m_keyMap[Qt::Key_S]
        || m_keyMap[Qt::Key_A]
        || m_keyMap[Qt::Key_Space]
        || m_keyMap[Qt::Key_Control])) {

        this->makeCurrent();
        updateShapes(m_render, true);
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

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

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
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
