#ifndef MUSEUMWIDGET_H
#define MUSEUMWIDGET_H

#include <QCursor>
#include <QKeyEvent> //used for using keyboard keys when moving the camera
#include <QMatrix4x4>
#include <QMouseEvent> //used for using the mouse to look around
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture> //for adding texture objects
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector3D>
#include "rendering/Vertex.h"
#include <vector>
#include "scene/ClickableArtwork.h"

// This widget is where our 3D museum scene is drawn
// It inherits from QOpenGLWidget so Qt can place it inside the MainWindow
// Inherits from QOpenGLFunctions_3_3_Core so we can call OpenGL functions safely
class MuseumWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit MuseumWidget(QWidget *parent = nullptr);
    ~MuseumWidget();

protected:
    void initializeGL() override;

    // Called whenever the window is resized
    void resizeGL(int w, int h) override;

    // Called whenever the scene needs to be redrawn
    void paintGL() override;

    //this function helps us respond when the user presses W,A,S, or D
    void keyPressEvent(QKeyEvent *event) override;

    //this helps us use the mouse to look around the rooms
    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

private:
    // Creates the vertices for our simple room
    void setupRoomGeometry();
    void uploadGeometryToGPU(const std::vector<Vertex> &vertices);
    void loadPaintingTextures();
    void loadSculptureTexture();

    std::vector<QOpenGLTexture*> m_paintingTextures;
    std::vector<ClickableArtwork> m_clickableArtworks;

    void showArtworkPopup(const ClickableArtwork& artwork);

    QVector3D createRayFromMouseClick(const QPoint& mousePosition);
    bool rayIntersectsArtwork(
        const QVector3D& rayOrigin,
        const QVector3D& rayDirection,
        const ClickableArtwork& artwork,
        float& distance
        );

private:
    QOpenGLShaderProgram m_program;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;

    int m_vertexCount = 0;

    // This projection matrix controls the perspective
    QMatrix4x4 m_projection;

    //camera variables below:
    QVector3D m_cameraPosition;
    QVector3D m_cameraFront;
    QVector3D m_cameraUp;

    float m_cameraSpeed = 0.25f;

    //mouse look variables:
    float m_yaw = -90.0f; //-90 lines up with camera looking straight ahead (down at the -z axis)
    float m_pitch = 0.0f;

    float m_mouseSensitivity = 0.15f;

    bool m_firstMouse = true;
    QPoint m_lastMousePosition;

    bool m_mouseLocked = true;
    bool m_ignoreNextMouseMove = false;

    // texture variable
    QOpenGLTexture* m_sculptureTexture = nullptr;
};

#endif // MUSEUMWIDGET_H
