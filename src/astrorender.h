#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include "GL/glew.h" // Must always be first include
#include <QOpenGLWidget>
#include "glm/glm.hpp"
#include "utils/scene.h"
#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class AstroRender : public QOpenGLWidget
{
public:
    AstroRender(Scene scene, QWidget *parent = nullptr);
    ~AstroRender();
    void finish();

public slots:
    void tick(QTimerEvent* event);

protected:
    void initializeGL()                  override; // Called once at the start of the program
    void paintGL()                       override; // Called every frame in a loop
    void resizeGL(int width, int height) override; // Called when window size changes
    // void wheelEvent(QWheelEvent *e)      override; // Used for camera movement
    // void rebuildMatrices();                        // Used for camera movement
    void createHDRFramebuffer(int w, int h);
    void renderSceneHDR();
    void toneMapToScreen();
    void uploadLights(GLuint shader);
    void uploadShapes(GLuint shader);

    //Camera movement
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    GLuint loadTexture(const QString &filePath);



private:
    GLuint m_phong_shader, m_tonemap_shader;
    GLuint m_quadVAO, m_quadVBO;
    GLuint m_hdrFBO, m_hdrColorTex, m_hdrDepthRBO;
    GLuint m_defaultFBO;
    GLuint m_shapeTextures[8] = {0}; // match MAX_SHAPES
    GLint shapeTexUnitArray[8] = {0};



    //The scene being rendered
    Scene m_scene;

    QPoint m_prevMousePos;
    float  m_angleX;
    float  m_angleY;
    float  m_zoom;

    // GLuint m_phong_shader, m_tonemap_shader;
    int m_screen_width, m_screen_height;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;
};
