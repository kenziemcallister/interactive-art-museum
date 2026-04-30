#include "MuseumWidget.h"

#include <vector>

// Each vertex has 3D position and a color
// This is where we will add texture coordinates later for more complex details
struct Vertex
{
    QVector3D position;
    QVector3D color;
};

MuseumWidget::MuseumWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    m_vbo(QOpenGLBuffer::VertexBuffer)
{
}

MuseumWidget::~MuseumWidget()
{
    makeCurrent();

    m_vbo.destroy();
    m_vao.destroy();
    m_program.removeAllShaders();

    doneCurrent();
}

void MuseumWidget::initializeGL()
{
    // This has to be called before using OpenGL functions
    initializeOpenGLFunctions();

    // Makes depth work correctly so walls/floor overlap properly in 3D
    glEnable(GL_DEPTH_TEST);

    // Background color of the window
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);

    // Vertex shader:
    // Takes each vertex position and transforms it by the MVP matrix
    const char *vertexShaderSource = R"(
        #version 330 core

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;

        uniform mat4 mvp;

        out vec3 vertexColor;

        void main()
        {
            vertexColor = color;
            gl_Position = mvp * vec4(position, 1.0);
        }
    )";

    // Fragment shader:
    // Controls the final color of each pixel
    const char *fragmentShaderSource = R"(
        #version 330 core

        in vec3 vertexColor;
        out vec4 fragColor;

        void main()
        {
            fragColor = vec4(vertexColor, 1.0);
        }
    )";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program.link();

    setupRoomGeometry();
}

void MuseumWidget::resizeGL(int w, int h)
{
    // Prevent division by zero if the window is minimized
    if (h == 0)
    {
        h = 1;
    }

    glViewport(0, 0, w, h);

    // Reset and recreate the perspective projection
    m_projection.setToIdentity();

    float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

    // 60 degrees field of view
    // aspect ratio from window size
    // near plane
    // far plane
    m_projection.perspective(60.0f, aspectRatio, 0.1f, 100.0f);
}

void MuseumWidget::paintGL()
{
    // Clear screen and depth buffer before drawing the room
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera/view matrix
    // For now the camera is fixed
    // WASD will change cameraPosition in later steps
    QMatrix4x4 view;

    QVector3D cameraPosition(0.0f, 1.6f, 4.0f);   // eye position
    QVector3D cameraTarget(0.0f, 1.4f, -3.0f);    // what we are looking at
    QVector3D cameraUp(0.0f, 1.0f, 0.0f);         // which way is up

    view.lookAt(cameraPosition, cameraTarget, cameraUp);

    // Model matrix
    // The room is not moved, rotated, or scaled yet
    QMatrix4x4 model;
    model.setToIdentity();

    // MVP -> Projection * View * Model
    QMatrix4x4 mvp = m_projection * view * model;

    m_program.bind();

    m_program.setUniformValue("mvp", mvp);

    m_vao.bind();

    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);

    m_vao.release();

    m_program.release();
}

void MuseumWidget::setupRoomGeometry()
{
    // Room dimensions:
    // x = left/right
    // y = up/down
    // z = depth

    // Camera starts in front of the room looking toward negative z
    const float left = -4.0f;
    const float right = 4.0f;
    const float floorY = 0.0f;
    const float ceilingY = 3.0f;
    const float frontZ = 1.0f;
    const float backZ = -8.0f;

    // Simple colors.
    QVector3D floorColor(0.45f, 0.38f, 0.30f);
    QVector3D wallColor(0.78f, 0.74f, 0.66f);
    QVector3D sideWallColor(0.68f, 0.65f, 0.58f);
    QVector3D ceilingColor(0.86f, 0.84f, 0.78f);

    std::vector<Vertex> vertices;

    // Helper lambda to add one rectangle as two triangles
    auto addRectangle = [&vertices](QVector3D a, QVector3D b, QVector3D c, QVector3D d, QVector3D color)
    {
        // Triangle 1
        vertices.push_back({ a, color });
        vertices.push_back({ b, color });
        vertices.push_back({ c, color });

        // Triangle 2
        vertices.push_back({ a, color });
        vertices.push_back({ c, color });
        vertices.push_back({ d, color });
    };

    // Floor
    addRectangle(
        QVector3D(left,  floorY, frontZ),
        QVector3D(right, floorY, frontZ),
        QVector3D(right, floorY, backZ),
        QVector3D(left,  floorY, backZ),
        floorColor
        );

    // Ceiling
    addRectangle(
        QVector3D(left,  ceilingY, frontZ),
        QVector3D(left,  ceilingY, backZ),
        QVector3D(right, ceilingY, backZ),
        QVector3D(right, ceilingY, frontZ),
        ceilingColor
        );

    // Back wall
    addRectangle(
        QVector3D(left,  floorY,   backZ),
        QVector3D(right, floorY,   backZ),
        QVector3D(right, ceilingY, backZ),
        QVector3D(left,  ceilingY, backZ),
        wallColor
        );

    // Left wall
    addRectangle(
        QVector3D(left, floorY,   frontZ),
        QVector3D(left, floorY,   backZ),
        QVector3D(left, ceilingY, backZ),
        QVector3D(left, ceilingY, frontZ),
        sideWallColor
        );

    // Right wall
    addRectangle(
        QVector3D(right, floorY,   backZ),
        QVector3D(right, floorY,   frontZ),
        QVector3D(right, ceilingY, frontZ),
        QVector3D(right, ceilingY, backZ),
        sideWallColor
        );

    m_vertexCount = static_cast<int>(vertices.size());

    // Creating and binding VAO
    m_vao.create();
    m_vao.bind();

    // Creating and filling VBO
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(Vertex)));

    // Attribute 0 = position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void *>(offsetof(Vertex, position))
        );

    // Attribute 1 = color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void *>(offsetof(Vertex, color))
        );

    m_vbo.release();
    m_vao.release();
}
