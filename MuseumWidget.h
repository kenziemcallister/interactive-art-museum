#ifndef MUSEUMWIDGET_H
#define MUSEUMWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>

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

private:
    // Creates the vertices for our simple room
    void setupRoomGeometry();

private:
    QOpenGLShaderProgram m_program;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;

    int m_vertexCount = 0;

    // This projection matrix controls the perspective
    QMatrix4x4 m_projection;
};

#endif // MUSEUMWIDGET_H
