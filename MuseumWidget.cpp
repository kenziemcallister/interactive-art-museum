#include "MuseumWidget.h"

#include <vector>
#include <QtGlobal>
#include <QtMath>

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
    //lets the OpenGL widget to receive keyboard input
    setFocusPolicy(Qt::StrongFocus);

    //allows to receive mouse movement events even when no mouse button is pressed
    setMouseTracking(true);

    //hide the cursor while navigating for cleaner look
    setCursor(Qt::BlankCursor);

    //starting camera position
    //x = left and right
    //y = height
    //z = moving forward or backward
    m_cameraPosition = QVector3D(0.0f, 1.8f, 1.0f);

    //direction the camera looks at (-z means looking into the room)
    m_cameraFront = QVector3D(0.0f, 0.0f, -1.0f);

    //defining which direction is considered up:
    m_cameraUp = QVector3D(0.0f, 1.0f, 0.0f);
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
    m_projection.perspective(70.0f, aspectRatio, 0.1f, 100.0f);
}

void MuseumWidget::paintGL()
{
    // Clear screen and depth buffer before drawing the room
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera/view matrix
    QMatrix4x4 view;

    //the camera looks from the current position toward pos in front of it
    //m_cameraPosition = where we are standing,
    //m_cameraPosition + m_cameraFront is the point we are looking at
    view.lookAt(m_cameraPosition, m_cameraPosition + m_cameraFront, m_cameraUp);

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
    // Museum layout:
    // x = left/right
    // y = up/down
    // z = depth
    //
    // Room 1 goes from frontZ to middleZ.
    // Room 2 goes from middleZ to backZ.
    //
    // The shared wall at middleZ has a doorway cut into it.

    const float left = -6.0f;
    const float right = 6.0f;

    const float floorY = 0.0f;
    const float ceilingY = 4.5f;

    const float frontZ = 2.0f;
    const float middleZ = -10.0f;
    const float backZ = -22.0f;

    // Doorway dimensions on the wall between Room 1 and Room 2.
    const float doorLeft = -1.5f;
    const float doorRight = 1.5f;
    const float doorTop = 3.0f;

    // Simple colors for now.
    QVector3D floorColor(0.45f, 0.38f, 0.30f);
    QVector3D wallColor(0.78f, 0.74f, 0.66f);
    QVector3D sideWallColor(0.68f, 0.65f, 0.58f);
    QVector3D ceilingColor(0.86f, 0.84f, 0.78f);

    std::vector<Vertex> vertices;

    // Helper function:
    // Adds one rectangle made from two triangles.
    auto addRectangle = [&vertices](QVector3D a,
                                    QVector3D b,
                                    QVector3D c,
                                    QVector3D d,
                                    QVector3D color)
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

    // Helper function:
    // Adds a full rectangular room shell.
    //
    // This does NOT add front/back walls by default because we want control
    // over doorways. It adds:
    // floor, ceiling, left wall, right wall.
    auto addRoomShell = [&](float zFront, float zBack)
    {
        // Floor
        addRectangle(
            QVector3D(left,  floorY, zFront),
            QVector3D(right, floorY, zFront),
            QVector3D(right, floorY, zBack),
            QVector3D(left,  floorY, zBack),
            floorColor
            );

        // Ceiling
        addRectangle(
            QVector3D(left,  ceilingY, zFront),
            QVector3D(left,  ceilingY, zBack),
            QVector3D(right, ceilingY, zBack),
            QVector3D(right, ceilingY, zFront),
            ceilingColor
            );

        // Left wall
        addRectangle(
            QVector3D(left, floorY,   zFront),
            QVector3D(left, floorY,   zBack),
            QVector3D(left, ceilingY, zBack),
            QVector3D(left, ceilingY, zFront),
            sideWallColor
            );

        // Right wall
        addRectangle(
            QVector3D(right, floorY,   zBack),
            QVector3D(right, floorY,   zFront),
            QVector3D(right, ceilingY, zFront),
            QVector3D(right, ceilingY, zBack),
            sideWallColor
            );
    };

    // Helper function:
    // Adds a solid wall at a constant z value.
    auto addSolidWallAtZ = [&](float z)
    {
        addRectangle(
            QVector3D(left,  floorY,   z),
            QVector3D(right, floorY,   z),
            QVector3D(right, ceilingY, z),
            QVector3D(left,  ceilingY, z),
            wallColor
            );
    };

    // Helper function:
    // Adds a wall at a constant z value, but leaves a rectangular doorway open.
    auto addWallWithDoorAtZ = [&](float z)
    {
        // Left piece of wall beside doorway
        addRectangle(
            QVector3D(left,     floorY,   z),
            QVector3D(doorLeft, floorY,   z),
            QVector3D(doorLeft, ceilingY, z),
            QVector3D(left,     ceilingY, z),
            wallColor
            );

        // Right piece of wall beside doorway
        addRectangle(
            QVector3D(doorRight, floorY,   z),
            QVector3D(right,     floorY,   z),
            QVector3D(right,     ceilingY, z),
            QVector3D(doorRight, ceilingY, z),
            wallColor
            );

        // Top piece of wall above doorway
        addRectangle(
            QVector3D(doorLeft,  doorTop,  z),
            QVector3D(doorRight, doorTop,  z),
            QVector3D(doorRight, ceilingY, z),
            QVector3D(doorLeft,  ceilingY, z),
            wallColor
            );
    };

    // Adds a framed painting on a wall where z stays constant.
    // Good for back walls or front walls.
    auto addPaintingOnZWall = [&](float centerX,
                                  float centerY,
                                  float z,
                                  float width,
                                  float height,
                                  QVector3D artColor)
    {
        QVector3D frameColor(0.05f, 0.03f, 0.02f);

        float halfW = width / 2.0f;
        float halfH = height / 2.0f;

        // Slight offset so the painting is in front of the wall,
        // not exactly inside the wall.
        float offset = 0.04f;

        float framePadding = 0.15f;

        // Frame rectangle
        addRectangle(
            QVector3D(centerX - halfW - framePadding, centerY - halfH - framePadding, z + offset),
            QVector3D(centerX + halfW + framePadding, centerY - halfH - framePadding, z + offset),
            QVector3D(centerX + halfW + framePadding, centerY + halfH + framePadding, z + offset),
            QVector3D(centerX - halfW - framePadding, centerY + halfH + framePadding, z + offset),
            frameColor
            );

        // Artwork rectangle
        addRectangle(
            QVector3D(centerX - halfW, centerY - halfH, z + offset * 2.0f),
            QVector3D(centerX + halfW, centerY - halfH, z + offset * 2.0f),
            QVector3D(centerX + halfW, centerY + halfH, z + offset * 2.0f),
            QVector3D(centerX - halfW, centerY + halfH, z + offset * 2.0f),
            artColor
            );
    };


    // Adds a framed painting on the left wall where x stays constant.
    // The painting stretches along z and y.
    auto addPaintingOnLeftWall = [&](float centerZ,
                                     float centerY,
                                     float x,
                                     float width,
                                     float height,
                                     QVector3D artColor)
    {
        QVector3D frameColor(0.05f, 0.03f, 0.02f);

        float halfW = width / 2.0f;
        float halfH = height / 2.0f;

        float offset = 0.04f;
        float framePadding = 0.15f;

        // Frame rectangle
        addRectangle(
            QVector3D(x + offset, centerY - halfH - framePadding, centerZ + halfW + framePadding),
            QVector3D(x + offset, centerY - halfH - framePadding, centerZ - halfW - framePadding),
            QVector3D(x + offset, centerY + halfH + framePadding, centerZ - halfW - framePadding),
            QVector3D(x + offset, centerY + halfH + framePadding, centerZ + halfW + framePadding),
            frameColor
            );

        // Artwork rectangle
        addRectangle(
            QVector3D(x + offset * 2.0f, centerY - halfH, centerZ + halfW),
            QVector3D(x + offset * 2.0f, centerY - halfH, centerZ - halfW),
            QVector3D(x + offset * 2.0f, centerY + halfH, centerZ - halfW),
            QVector3D(x + offset * 2.0f, centerY + halfH, centerZ + halfW),
            artColor
            );
    };


    // Adds a framed painting on the right wall where x stays constant.
    // The painting stretches along z and y.
    auto addPaintingOnRightWall = [&](float centerZ,
                                      float centerY,
                                      float x,
                                      float width,
                                      float height,
                                      QVector3D artColor)
    {
        QVector3D frameColor(0.05f, 0.03f, 0.02f);

        float halfW = width / 2.0f;
        float halfH = height / 2.0f;

        float offset = 0.04f;
        float framePadding = 0.15f;

        // Frame rectangle
        addRectangle(
            QVector3D(x - offset, centerY - halfH - framePadding, centerZ - halfW - framePadding),
            QVector3D(x - offset, centerY - halfH - framePadding, centerZ + halfW + framePadding),
            QVector3D(x - offset, centerY + halfH + framePadding, centerZ + halfW + framePadding),
            QVector3D(x - offset, centerY + halfH + framePadding, centerZ - halfW - framePadding),
            frameColor
            );

        // Artwork rectangle
        addRectangle(
            QVector3D(x - offset * 2.0f, centerY - halfH, centerZ - halfW),
            QVector3D(x - offset * 2.0f, centerY - halfH, centerZ + halfW),
            QVector3D(x - offset * 2.0f, centerY + halfH, centerZ + halfW),
            QVector3D(x - offset * 2.0f, centerY + halfH, centerZ - halfW),
            artColor
            );
    };

    // -------------------------
    // Build the museum layout
    // -------------------------

    // Room 1 shell
    addRoomShell(frontZ, middleZ);

    // Room 2 shell
    addRoomShell(middleZ, backZ);

    // Front wall of Room 1.
    // This is behind the player at the start.
    addSolidWallAtZ(frontZ);

    // Shared wall between Room 1 and Room 2, with doorway.
    addWallWithDoorAtZ(middleZ);

    // Back wall of Room 2.
    addSolidWallAtZ(backZ);

    // -------------------------
    // Add placeholder paintings
    // -------------------------
    //
    // These are colored placeholders for now.
    // Later, we can replace the artColor with real National Gallery of Art image textures.

    // Room 1 left wall paintings
    addPaintingOnLeftWall(
        -3.0f,      // center z
        2.2f,       // center y
        left,       // wall x position
        2.0f,       // painting width
        1.3f,       // painting height
        QVector3D(0.15f, 0.30f, 0.75f) // blue artwork
        );

    addPaintingOnLeftWall(
        -7.0f,
        2.2f,
        left,
        2.0f,
        1.3f,
        QVector3D(0.70f, 0.25f, 0.15f) // red artwork
        );


    // Room 1 right wall paintings
    addPaintingOnRightWall(
        -3.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        QVector3D(0.20f, 0.60f, 0.35f) // green artwork
        );

    addPaintingOnRightWall(
        -7.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        QVector3D(0.65f, 0.45f, 0.15f) // gold artwork
        );


    // Room 2 left wall paintings
    addPaintingOnLeftWall(
        -13.0f,
        2.2f,
        left,
        2.0f,
        1.3f,
        QVector3D(0.55f, 0.20f, 0.65f) // purple artwork
        );


    // Room 2 right wall paintings
    addPaintingOnRightWall(
        -13.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        QVector3D(0.10f, 0.55f, 0.65f) // teal artwork
        );


    // Large feature painting on the back wall of Room 2
    addPaintingOnZWall(
        0.0f,       // center x
        2.3f,       // center y
        backZ,      // wall z position
        3.0f,       // width
        1.7f,       // height
        QVector3D(0.80f, 0.35f, 0.25f) // warm orange artwork
        );

    // -------------------------
    // Send geometry to OpenGL
    // -------------------------

    m_vertexCount = static_cast<int>(vertices.size());

    m_vao.create();
    m_vao.bind();

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

void MuseumWidget::keyPressEvent(QKeyEvent *event)
{
    //forward/backward movement uses the direction the camera is facing
    QVector3D forward = m_cameraFront;
    forward.setY(0.0f);
    forward.normalize();

    //left/right movement uses the cross product
    //this gives us the sideways direction relative to the camera
    QVector3D right = QVector3D::crossProduct(forward, m_cameraUp).normalized();

    if (event->key() == Qt::Key_W)
    {
        //moving forward
        m_cameraPosition += forward * m_cameraSpeed;
    }
    else if (event->key() == Qt::Key_S)
    {
        //moving backward
        m_cameraPosition -= forward * m_cameraSpeed;
    }
    else if (event->key() == Qt::Key_A)
    {
        //moving left
        m_cameraPosition -= right * m_cameraSpeed;
    }
    else if (event->key() == Qt::Key_D)
    {
        //moving right
        m_cameraPosition += right * m_cameraSpeed;
    }
    else if (event->key() == Qt::Key_Escape)
    {
        //makes cursor visible again to click on artwork if we press esc on keyboard
        m_mouseLocked = false;
        setCursor(Qt::ArrowCursor);
    }

    //need to keep the movement between the walls:
    m_cameraPosition.setX(qBound(-5.7f, m_cameraPosition.x(), 5.7f));
    m_cameraPosition.setY(1.8f);
    m_cameraPosition.setZ(qBound(-21.7f, m_cameraPosition.z(), 1.7f));

    //redraw scene after moving
    update();
}

void MuseumWidget::mouseMoveEvent(QMouseEvent *event)
{
    //ff mouse look is turned off, do not rotate the camera
    if (!m_mouseLocked)
    {
        return;
    }

    // When we manually move the cursor back to the center,
    // Qt may fire another mouseMoveEvent.
    // We ignore that one so the camera does not jitter.
    if (m_ignoreNextMouseMove)
    {
        m_ignoreNextMouseMove = false;
        return;
    }

    //center of the OpenGL widget
    QPoint center(width() / 2, height() / 2);

    //current mouse position inside the widget
    QPoint currentMousePosition = event->pos();

    //difference between where the mouse is and the center
    float xOffset = currentMousePosition.x() - center.x();

    // Y is reversed because screen y increases downward
    float yOffset = center.y() - currentMousePosition.y();

    //applying sensitivity so the camera does not spin too fast
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;

    //updating yaw and pitch
    m_yaw += xOffset;
    m_pitch += yOffset;

    //prevent camera from flipping upside down
    if (m_pitch > 89.0f)
    {
        m_pitch = 89.0f;
    }

    if (m_pitch < -89.0f)
    {
        m_pitch = -89.0f;
    }

    //converting yaw/pitch into a direction vector
    QVector3D direction;

    direction.setX(qCos(qDegreesToRadians(m_yaw)) * qCos(qDegreesToRadians(m_pitch)));
    direction.setY(qSin(qDegreesToRadians(m_pitch)));
    direction.setZ(qSin(qDegreesToRadians(m_yaw)) * qCos(qDegreesToRadians(m_pitch)));

    m_cameraFront = direction.normalized();

    //move the cursor back to the center of the widget
    //mapToGlobal converts the widget center to a screen position
    m_ignoreNextMouseMove = true;
    QCursor::setPos(mapToGlobal(center));

    update();
}


void MuseumWidget::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    setFocus(); //clicking inside open gl widget gives keyboard focus

    m_mouseLocked = true;
    setCursor(Qt::BlankCursor);

    //puts cursor in center so movement starts cleanly
    QPoint center(width() / 2, height() / 2);

    m_ignoreNextMouseMove = true;
    QCursor::setPos(mapToGlobal(center));
}
