#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <camera/camera.h>
#include <glm/glm.hpp>
#include <utils/sceneparser.h>

#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include <unordered_map>

#define MOVEMENT_SPEED  5.0f
#define Y_ROTATION_SENS 0.5f
#define X_ROTATION_SENS 0.5f

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish(); // Called on program exit
    void sceneChanged();
    void CameraSettingsChanged();
    void SceneSettingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent *event); // Called once per tick of m_timer

protected:
    void initializeGL() override; // Called once at the start of the program
    void paintGL() override; // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override; // Called when window size changes

    void createHDRFramebuffer(int w, int h);
    void renderSceneHDR();
    void toneMapToScreen();

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                  // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer; // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                   // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                 // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap; // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;
    int m_screen_width;
    int m_screen_height;

    // GL state variables
    GLuint m_phong_shader, m_tonemap_shader;
    GLuint m_quadVAO, m_quadVBO;
    GLuint m_hdrFBO, m_hdrColorTex, m_hdrDepthRBO;
    GLuint m_defaultFBO;

    // Data for rendering
    RenderData m_render;

    // Scene camera
    Camera *m_cam;
    int m_deltaX;
    int m_deltaY;
};
