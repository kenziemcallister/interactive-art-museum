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
#include <QMessageBox>
#include <QVector4D>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <limits>

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
        //lighting
        layout(location = 5) in vec3 normal;

        uniform mat4 mvp;
        //lighting
        uniform mat4 model;
        uniform mat3 normalMatrix;

        out vec3 vertexColor;
        out vec2 fragTexCoord;
        out float fragUseTexture;
        out float fragTextureIndex;
        //lighting
        out vec3 FragPos;
        out vec3 Normal;

        void main()
        {
            vertexColor = color;
            fragTexCoord = texCoord;
            fragUseTexture = useTexture;
            fragTextureIndex = textureIndex;

            // world-space fragment position
            FragPos = vec3(model * vec4(position, 1.0));
            // transform normal by normalMatrix (model's inverse-transpose)
            Normal = normalize(normalMatrix * normal);

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
        //lighting
        in vec3 FragPos;
        in vec3 Normal;

        // increased size to cover paintings & room textures & sculpture
        uniform sampler2D paintingTextures[13];
        //uniform sampler2D sculptureTexture;

        //lighting uniforms
        uniform vec3 lightPos;
        uniform vec3 lightPos2;
        uniform vec3 lightColor;
        uniform vec3 viewPos;

        out vec4 fragColor;

        void main()
        {
            // --- lighting parameters ---
            float ambientStrength = 0.3;
            float specularStrength = 0.2;
            float shininess = 20.0;

            // Ambient
            vec3 ambient = ambientStrength * lightColor;

            // Diffuse (Lambert)
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            //float diff = max(dot(norm, lightDir), 0.0);
            float diff = max(abs(dot(norm, lightDir)), 0.0);
            vec3 diffuse = diff * lightColor * 0.5;

            // Specular (Blinn-Phong)
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 halfway = normalize(lightDir + viewDir);
            float spec = pow(max(dot(norm, halfway), 0.0), shininess);
            vec3 specular = specularStrength * spec * lightColor;

            // Second light for room 3
            vec3 lightDir2 = normalize(lightPos2 - FragPos);
            float diff2 = max(abs(dot(norm, lightDir2)), 0.0);
            vec3 diffuse2 = diff2 * lightColor * 0.5;
            vec3 lighting = ambient + diffuse + specular + diffuse2;

            // --- Base color from texture or vertex color ---
            vec4 baseColor;

            if (fragUseTexture > 0.5)
            {
                int index = int(fragTextureIndex);
                // clamp index to valid range definitively
                index = clamp(index, 0, 12);
                baseColor = texture(paintingTextures[index], fragTexCoord);
            }
            else
            {
                baseColor = vec4(vertexColor, 1.0);
            }

            // apply lighting to RBG & preserve alpha
            vec3 lit = lighting * baseColor.rgb;
            fragColor = vec4(lit, baseColor.a);
        }
    )";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program.link();

    if (!m_program.link()) {
        qDebug() << "Shader program link failed:" << m_program.log();
    } else {
        qDebug() << "Shader program linked successfully.";
    }

    //loading image textures below
    loadPaintingTextures();
    // load sculptures!!
    //loadSculptureTexture();
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

    // Model matrix (identity here; change if you transform objects)
    //QMatrix4x4 model;
    //model.setToIdentity();
    m_program.setUniformValue("model", model);

    // Normal matrix (3x3) — use model.normalMatrix()
    QMatrix3x3 normalMat = model.normalMatrix();
    m_program.setUniformValue("normalMatrix", normalMat);

    // Light and view uniforms
    // Place the light somewhere above the room; tweak as needed
    m_program.setUniformValue("lightPos", QVector3D(0.0f, 3.5f, -10.0f));
    m_program.setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));
    m_program.setUniformValue("viewPos", m_cameraPosition);

    // sculpture light
    m_program.setUniformValue("lightPos2", QVector3D(12.0f, 3.5f, -19.0f));


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
    m_clickableArtworks.clear();

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
        12.0f  // slot 10, next available texture slot after floor-7, wall-8, ceiling-9
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

    // attribute 5 = normal for lighting!!
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, normal)));

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
    setFocus(); //clicking inside open gl widget gives keyboard focus

    QVector3D rayOrigin = m_cameraPosition;
    QVector3D rayDirection = createRayFromMouseClick(event->pos());

    float closestDistance = std::numeric_limits<float>::max();
    const ClickableArtwork* closestArtwork = nullptr;

    for (const ClickableArtwork& artwork : m_clickableArtworks)
    {
        float distance = 0.0f;

        if (rayIntersectsArtwork(rayOrigin, rayDirection, artwork, distance))
        {
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestArtwork = &artwork;
            }
        }
    }

    if (closestArtwork != nullptr)
    {
        showArtworkPopup(*closestArtwork);
        return;
    }

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
                                          ":/artwork/Monet.jpg",
                                          ":/artwork/GreatWave.jpg",
                                          ":/artwork/Crown.jpg"};

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
        //m_sculptureTexture = nullptr; // avoid double-delete in destructor
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
        m_sculptureTexture = nullptr;
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

//Gives the ray direction. Ray origin is the camera
QVector3D MuseumWidget::createRayFromMouseClick(const QPoint& mousePosition)
{
    // Convert mouse position from screen pixels into normalized device coordinates.
    // x goes from -1 to 1.
    // y goes from -1 to 1, but screen y is flipped.
    float x = (2.0f * mousePosition.x()) / width() - 1.0f;
    float y = 1.0f - (2.0f * mousePosition.y()) / height();

    // Start ray in clip space.
    QVector4D rayClip(x, y, -1.0f, 1.0f);

    // Move ray from clip space into eye/camera space.
    QMatrix4x4 inverseProjection = m_projection.inverted();
    QVector4D rayEye = inverseProjection * rayClip;

    // We want a direction, not a point, so z = -1 and w = 0.
    rayEye = QVector4D(rayEye.x(), rayEye.y(), -1.0f, 0.0f);

    // Rebuild the same view matrix used in paintGL().
    QMatrix4x4 view;
    view.lookAt(m_cameraPosition, m_cameraPosition + m_cameraFront, m_cameraUp);

    // Move ray from camera space into world space.
    QMatrix4x4 inverseView = view.inverted();
    QVector4D rayWorld4 = inverseView * rayEye;

    QVector3D rayWorld(rayWorld4.x(), rayWorld4.y(), rayWorld4.z());
    return rayWorld.normalized();
}

static bool rayIntersectsTriangle(
    const QVector3D& rayOrigin,
    const QVector3D& rayDirection,
    const QVector3D& v0,
    const QVector3D& v1,
    const QVector3D& v2,
    float& distance
    )
{
    // Moller-Trumbore ray-triangle intersection.
    const float EPSILON = 0.000001f;

    QVector3D edge1 = v1 - v0;
    QVector3D edge2 = v2 - v0;

    QVector3D h = QVector3D::crossProduct(rayDirection, edge2);
    float a = QVector3D::dotProduct(edge1, h);

    if (a > -EPSILON && a < EPSILON)
    {
        return false;
    }

    float f = 1.0f / a;
    QVector3D s = rayOrigin - v0;
    float u = f * QVector3D::dotProduct(s, h);

    if (u < 0.0f || u > 1.0f)
    {
        return false;
    }

    QVector3D q = QVector3D::crossProduct(s, edge1);
    float v = f * QVector3D::dotProduct(rayDirection, q);

    if (v < 0.0f || u + v > 1.0f)
    {
        return false;
    }

    float t = f * QVector3D::dotProduct(edge2, q);

    if (t > EPSILON)
    {
        distance = t;
        return true;
    }

    return false;
}

bool MuseumWidget::rayIntersectsArtwork(
    const QVector3D& rayOrigin,
    const QVector3D& rayDirection,
    const ClickableArtwork& artwork,
    float& distance
    )
{
    float distance1 = 0.0f;
    float distance2 = 0.0f;

    bool hitFirstTriangle = rayIntersectsTriangle(
        rayOrigin,
        rayDirection,
        artwork.a,
        artwork.b,
        artwork.c,
        distance1
        );

    bool hitSecondTriangle = rayIntersectsTriangle(
        rayOrigin,
        rayDirection,
        artwork.a,
        artwork.c,
        artwork.d,
        distance2
        );

    if (hitFirstTriangle && hitSecondTriangle)
    {
        distance = qMin(distance1, distance2);
        return true;
    }

    if (hitFirstTriangle)
    {
        distance = distance1;
        return true;
    }

    if (hitSecondTriangle)
    {
        distance = distance2;
        return true;
    }

    return false;
}

void MuseumWidget::showArtworkPopup(const ClickableArtwork& artwork)
{
    QDialog dialog(this);
    dialog.setWindowTitle(artwork.title);
    dialog.setModal(true);
    dialog.resize(460, 320);

    // Main layout for the popup
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(28, 26, 28, 22);
    mainLayout->setSpacing(12);

    // Title label
    QLabel* titleLabel = new QLabel(artwork.title);
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 23px;"
        "font-weight: bold;"
        "letter-spacing: 0.5px;"
        "color: #28464a;"
        );

    // Artist label
    QLabel* artistLabel = new QLabel("Artist: " + artwork.artist);
    artistLabel->setWordWrap(true);
    artistLabel->setAlignment(Qt::AlignCenter);
    artistLabel->setStyleSheet(
        "font-size: 15px;"
        "font-style: italic;"
        "color: #6e5a73;"
        );

    // Divider line
    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Plain);
    divider->setStyleSheet(
        "background-color: #8aa3a1;"
        "max-height: 1px;"
        );

    // Description label
    QLabel* descriptionLabel = new QLabel(artwork.description);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    descriptionLabel->setStyleSheet(
        "font-size: 14px;"
        "color: #303a3b;"
        "line-height: 140%;"
        );

    // Hint label
    QLabel* hintLabel = new QLabel("Click Close to return to the museum.");
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet(
        "font-size: 11px;"
        "font-style: italic;"
        "color: #6f7f80;"
        );

    // Close button
    QPushButton* closeButton = new QPushButton("Close");
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFixedWidth(100);
    closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #6e5a73;"
        "   color: #edf5f2;"
        "   border: 1px solid #4d3f52;"
        "   border-radius: 7px;"
        "   padding: 8px 16px;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #7f6a86;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #514057;"
        "}"
        );

    // Put the close button in the center
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(artistLabel);
    mainLayout->addWidget(divider);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(hintLabel);
    mainLayout->addLayout(buttonLayout);

    // Museum plaque styling, matched to the teal/purple floor palette
    dialog.setStyleSheet(
        "QDialog {"
        "   background-color: #d8e3df;"
        "   border: 3px solid #6e5a73;"
        "   border-radius: 14px;"
        "}"
        );

    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}
