/*
This is the main openGL widget. Owns the OpenGL context and displays the widget
"Director of the scene"
In charge of:
    - openGL setup
    - shader setup
    - texture loading
    - camera position/movement
    - mouse movement
    - keyboard interaction
    - calling the builders (room, painting, etc)
    - sends final vertex to the GPU
    - draws the scene
*/

#include "MuseumWidget.h"

#include "rendering/Vertex.h"
#include "scene/PaintingBuilder.h"
#include "scene/RoomBuilder.h"
#include "scene/SculptureBuilder.h"
#include "geometry/GeometryBuilder.h"

#include <QDebug>
#include <QImage>
#include <QtGlobal>
#include <QtMath>
#include <cstddef>
#include <vector>
#include <QFile>

MuseumWidget::MuseumWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_vbo(QOpenGLBuffer::VertexBuffer)
    , m_sculptureTexture(nullptr)
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

    for (QOpenGLTexture *texture : m_paintingTextures) {
        delete texture;
    }

    delete m_sculptureTexture;

    m_paintingTextures.clear();

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
        layout(location = 2) in vec2 texCoord;
        layout(location = 3) in float useTexture;
        layout(location = 4) in float textureIndex;

        uniform mat4 mvp;

        out vec3 vertexColor;
        out vec2 fragTexCoord;
        out float fragUseTexture;
        out float fragTextureIndex;

        void main()
        {
            vertexColor = color;
            fragTexCoord = texCoord;
            fragUseTexture = useTexture;
            fragTextureIndex = textureIndex;

            gl_Position = mvp * vec4(position, 1.0);
        }
    )";

    // Fragment shader:
    // Controls the final color of each pixel
    const char *fragmentShaderSource = R"(
        #version 330 core

        in vec3 vertexColor;
        in vec2 fragTexCoord;
        in float fragUseTexture;
        in float fragTextureIndex;

        // increased size to cover paintings & room textures & sculpture
        uniform sampler2D paintingTextures[12];
        //uniform sampler2D sculptureTexture;

        out vec4 fragColor;

        void main()
        {
            if (fragUseTexture > 0.5)
            {
                int index = int(fragTextureIndex);

                if (index == 0)
                    fragColor = texture(paintingTextures[0], fragTexCoord);
                else if (index == 1)
                    fragColor = texture(paintingTextures[1], fragTexCoord);
                else if (index == 2)
                    fragColor = texture(paintingTextures[2], fragTexCoord);
                else if (index == 3)
                    fragColor = texture(paintingTextures[3], fragTexCoord);
                else if (index == 4)
                    fragColor = texture(paintingTextures[4], fragTexCoord);
                else if (index == 5)
                    fragColor = texture(paintingTextures[5], fragTexCoord);
                else if (index == 6)
                    fragColor = texture(paintingTextures[6], fragTexCoord);
                else if (index == 7)
                    fragColor = texture(paintingTextures[7], fragTexCoord);
                else if (index == 8)
                    fragColor = texture(paintingTextures[8], fragTexCoord);
                else if (index == 9)
                    fragColor = texture(paintingTextures[9], fragTexCoord);
                else if (index == 10)
                    fragColor = texture(paintingTextures[10], fragTexCoord);
                else
                    fragColor = texture(paintingTextures[11], fragTexCoord);
            }
            else
            {
                fragColor = vec4(vertexColor, 1.0);
            }
        }
    )";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program.link();

    //loading image textures below
    loadPaintingTextures();
    // load sculptures!!
    loadSculptureTexture();
    setupRoomGeometry();
}

void MuseumWidget::resizeGL(int w, int h)
{
    // Prevent division by zero if the window is minimized
    if (h == 0) {
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

    // Bind every texture in m_paintingTextures to its corresponding unit
    for (int i = 0; i < static_cast<int>(m_paintingTextures.size()); ++i) {
        if (m_paintingTextures[i]) {
            m_paintingTextures[i]->bind(i);
        }
    }

    // Build an int array of texture unit indices and pass to shader
    std::vector<int> textureUnits(m_paintingTextures.size());
    for (int i = 0; i < static_cast<int>(textureUnits.size()); ++i) {
        textureUnits[i] = i;
    }

    // setUniformValueArray expects a raw pointer and count
    m_program.setUniformValueArray("paintingTextures", textureUnits.data(), static_cast<int>(textureUnits.size()));

    // Bind each painting texture to a texture unit.
    //for (int i = 0; i < static_cast<int>(m_paintingTextures.size()); i++) {
    //    if (m_paintingTextures[i]) {
    //        m_paintingTextures[i]->bind(i);
    //    }
    //}

    // Bind sculpture texture to slot 7.
    //if (m_sculptureTexture) {
    //    m_sculptureTexture->bind(7);
    //}

    // Tell the shader which texture units to use.
    //int textureUnits[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    //m_program.setUniformValueArray("paintingTextures", textureUnits, 8);

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

    std::vector<Vertex> vertices;

    // Build the walls, floor, ceiling, and doorway.
    RoomBuilder::addMuseumRooms(vertices);

    // Add all painting frames and artwork rectangles.
    PaintingBuilder::addPaintings(vertices, m_clickableArtworks);

    // wooden stool under the sculpture
    GeometryBuilder::addStool(
        vertices,
        QVector3D(12.0f, 0.2f, -19.0f), // center: x, topY, z
        2.0f,   // width (X)
        2.0f,   // depth (Z)
        2.0f,   // height (Y)
        QVector3D(0.45f, 0.30f, 0.18f), // wood color
        0.0f //0.02f   // small offset to reduce Z-fighting
        );

    // add sculptures!!!
    SculptureBuilder::addSculpture(
        vertices,
        ":/sculptures/oAM.obj",
        ":/sculptures/oAM.mtl",
        QVector3D(12.0f, 1.5f, -19.0f), // (x, y, z) -> y to move vertically
        5.0f,
        10.0f  // slot 10, next available texture slot after floor-7, wall-8, ceiling-9
        );

    // Send geometry to OpenGL.
    uploadGeometryToGPU(vertices);
}

void MuseumWidget::uploadGeometryToGPU(const std::vector<Vertex> &vertices)
{
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
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, position)));

    // Attribute 1 = color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, color)));

    // Attribute 2 = texture coordinate
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, texCoord)));

    // Attribute 3 = whether this vertex uses texture
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3,
                          1,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, useTexture)));

    // Attribute 4 = which texture this vertex uses
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4,
                          1,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, textureIndex)));

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

    if (event->key() == Qt::Key_W) {
        //moving forward
        m_cameraPosition += forward * m_cameraSpeed;
    } else if (event->key() == Qt::Key_S) {
        //moving backward
        m_cameraPosition -= forward * m_cameraSpeed;
    } else if (event->key() == Qt::Key_A) {
        //moving left
        m_cameraPosition -= right * m_cameraSpeed;
    } else if (event->key() == Qt::Key_D) {
        //moving right
        m_cameraPosition += right * m_cameraSpeed;
    } else if (event->key() == Qt::Key_Escape) {
        //makes cursor visible again to click on artwork if we press esc on keyboard
        m_mouseLocked = false;
        setCursor(Qt::ArrowCursor);
    }

    //need to keep the movement between the walls:
    m_cameraPosition.setX(qBound(-5.7f, m_cameraPosition.x(), 17.7f));
    m_cameraPosition.setY(1.8f);
    m_cameraPosition.setZ(qBound(-21.7f, m_cameraPosition.z(), 1.7f));

    //redraw scene after moving
    update();
}

void MuseumWidget::mouseMoveEvent(QMouseEvent *event)
{
    //ff mouse look is turned off, do not rotate the camera
    if (!m_mouseLocked) {
        return;
    }

    // When we manually move the cursor back to the center,
    // Qt may fire another mouseMoveEvent.
    // We ignore that one so the camera does not jitter.
    if (m_ignoreNextMouseMove) {
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
    if (m_pitch > 89.0f) {
        m_pitch = 89.0f;
    }

    if (m_pitch < -89.0f) {
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

void MuseumWidget::mousePressEvent(QMouseEvent *event)
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

void MuseumWidget::loadPaintingTextures()
{
    // All painting images that are compiled into Qt resources.
    std::vector<QString> paintingPaths = {":/artwork/Nighthawk.jpg",
                                          ":/artwork/Bedroom.jpg",
                                          ":/artwork/GeorgiaOkeeffe.jpg",
                                          ":/artwork/StarryNight.jpg",
                                          ":/artwork/FridaKahlo.jpg",
                                          ":/artwork/MonaLisa.jpg",
                                          ":/artwork/Monet.jpg"};

    for (const QString &path : paintingPaths) {
        QImage paintingImage(path);

        if (paintingImage.isNull()) {
            qDebug() << "Failed to load painting texture:" << path;
            m_paintingTextures.push_back(nullptr);
            continue;
        }

        qDebug() << "Painting texture loaded successfully:" << path << paintingImage.width() << "x"
                 << paintingImage.height();

        // Convert the image to a format OpenGL likes.
        paintingImage = paintingImage.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);

        QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture->create();
        texture->bind();

        texture->setData(paintingImage);

        // Use simple filters first.
        // Do NOT use mipmaps yet.
        texture->setMinificationFilter(QOpenGLTexture::Linear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);

        texture->setWrapMode(QOpenGLTexture::ClampToEdge);

        texture->release();

        m_paintingTextures.push_back(texture);
    }

    // load room textures in this order:
    // index 7 = walls, index 8 = floor, index 9 = CeilingTexture
    std::vector<QString> roomPaths = {
        ":/textures/WallTexture.png",
        ":/textures/FloorTexture.jpg",
        ":/textures/CeilingTexture.jpg"
    };

    for (const QString &path : roomPaths) {
        QImage img(path);
        if (img.isNull()) {
            qDebug() << "Failed to load room texture:" << path;
            m_paintingTextures.push_back(nullptr);
            continue;
        }

        qDebug() << "Room texture loaded successfully:" << path << img.width() << "x" << img.height();

        img = img.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);

        QOpenGLTexture *tex = new QOpenGLTexture(QOpenGLTexture::Target2D);
        tex->create();
        tex->bind();
        tex->setData(img);
        tex->setMinificationFilter(QOpenGLTexture::Linear);
        tex->setMagnificationFilter(QOpenGLTexture::Linear);
        tex->setWrapMode(QOpenGLTexture::ClampToEdge);
        tex->release();

        m_paintingTextures.push_back(tex);
    }

    // use existing loader for sculpture texture then append its pointer so it becomes index 10
    loadSculptureTexture(); // this sets m_sculptureTexture
    if (m_sculptureTexture) {
        // Ensure  sculpture texture is flipped/format-correct if loadSculptureTexture didn't already do it
        // If loadSculptureTexture already set m_sculptureTexture correctly, this is just appending the pointer.
        m_paintingTextures.push_back(m_sculptureTexture);
    } else {
        m_paintingTextures.push_back(nullptr);
    }

    // print how many textures we have
    qDebug() << "Total textures in m_paintingTextures:" << m_paintingTextures.size();
}

void MuseumWidget::loadSculptureTexture()
{
    const QString path = ":/sculptures/oAM.jpg";

    QFile file(path);
    qDebug() << "Sculpture texture exists?" << file.exists() << path;

    QImage img(path);
    if (img.isNull()) {
        qDebug() << "Failed to load sculpture texture";
        return;
    }

    qDebug() << "Sculpture texture loaded successfully:" << path
             << img.width() << "x" << img.height();

    img = img.convertToFormat(QImage::Format_RGBA8888);

    m_sculptureTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_sculptureTexture->create();
    m_sculptureTexture->setData(img);
    m_sculptureTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_sculptureTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_sculptureTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    qDebug() << "Sculpture texture loaded successfully";
}
