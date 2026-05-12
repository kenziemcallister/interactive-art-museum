/*
 * The reusable shape helper - in charge of turning simple shapes into vertices
 * Helps form rectangles for the paintings, walls, etc
 *
*/

#include "GeometryBuilder.h"

#include <QVector2D>

QVector3D GeometryBuilder::computeFaceNormal(const QVector3D &a, const QVector3D &b, const QVector3D &c) {
    return QVector3D::crossProduct(b - a, c - a).normalized();
}

//creating rectangles in the scene
void GeometryBuilder::addColoredRectangle(
    std::vector<Vertex>& vertices,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c,
    const QVector3D& d,
    const QVector3D& color
    )
{
    QVector2D noTexCoord(0.0f, 0.0f);
    float useTexture = 0.0f;
    float textureIndex = 0.0f;

    // compute face normal once and reuse for all vertices (flat shading)
    QVector3D normal = computeFaceNormal(a, b, c);

    // Triangle 1
    vertices.push_back({ a, color, noTexCoord, useTexture, textureIndex, normal });
    vertices.push_back({ b, color, noTexCoord, useTexture, textureIndex, normal });
    vertices.push_back({ c, color, noTexCoord, useTexture, textureIndex, normal });

    // Triangle 2
    vertices.push_back({ a, color, noTexCoord, useTexture, textureIndex, normal });
    vertices.push_back({ c, color, noTexCoord, useTexture, textureIndex, normal });
    vertices.push_back({ d, color, noTexCoord, useTexture, textureIndex, normal });
}

//creating textured rectangles (adding images, textures, etc)
void GeometryBuilder::addTexturedRectangle(
    std::vector<Vertex>& vertices,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c,
    const QVector3D& d,
    float textureIndex
    )
{
    QVector3D whiteColor(1.0f, 1.0f, 1.0f);
    float useTexture = 1.0f;

    QVector2D texA(0.0f, 0.0f);
    QVector2D texB(1.0f, 0.0f);
    QVector2D texC(1.0f, 1.0f);
    QVector2D texD(0.0f, 1.0f);

    // compute face normal once
    QVector3D normal = computeFaceNormal(a, b, c);

    // Triangle 1
    vertices.push_back({ a, whiteColor, texA, useTexture, textureIndex, normal });
    vertices.push_back({ b, whiteColor, texB, useTexture, textureIndex, normal });
    vertices.push_back({ c, whiteColor, texC, useTexture, textureIndex, normal });

    // Triangle 2
    vertices.push_back({ a, whiteColor, texA, useTexture, textureIndex, normal });
    vertices.push_back({ c, whiteColor, texC, useTexture, textureIndex, normal });
    vertices.push_back({ d, whiteColor, texD, useTexture, textureIndex, normal });
}

//helper for the wall texture to not repeat so much
void GeometryBuilder::addTexturedRectangleTiled(
    std::vector<Vertex>& vertices,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c,
    const QVector3D& d,
    float textureIndex,
    float tileX,
    float tileY
    )
{
    QVector3D whiteColor(1.0f, 1.0f, 1.0f);
    float useTexture = 1.0f;

    // Larger tileX/tileY = more repeating, smaller texture pattern.
    // Smaller tileX/tileY = less repeating, bigger texture pattern.
    QVector2D texA(0.0f, 0.0f);
    QVector2D texB(tileX, 0.0f);
    QVector2D texC(tileX, tileY);
    QVector2D texD(0.0f, tileY);

    // compute face normal once
    QVector3D normal = computeFaceNormal(a, b, c);

    // Triangle 1
    vertices.push_back({ a, whiteColor, texA, useTexture, textureIndex, normal });
    vertices.push_back({ b, whiteColor, texB, useTexture, textureIndex, normal });
    vertices.push_back({ c, whiteColor, texC, useTexture, textureIndex, normal });

    // Triangle 2
    vertices.push_back({ a, whiteColor, texA, useTexture, textureIndex, normal });
    vertices.push_back({ c, whiteColor, texC, useTexture, textureIndex, normal });
    vertices.push_back({ d, whiteColor, texD, useTexture, textureIndex, normal });
}

void GeometryBuilder::addStool(
    std::vector<Vertex>& vertices,
    const QVector3D& center,
    float width,
    float depth,
    float height,
    const QVector3D& color,
    float offset
    )
{
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float topY = center.y();
    float bottomY = center.y() - height;

    // Top face (slightly offset upward to avoid z-fighting with floor)
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(center.x() - halfW, topY, center.z() + halfD),
        QVector3D(center.x() + halfW, topY, center.z() + halfD),
        QVector3D(center.x() + halfW, topY, center.z() - halfD),
        QVector3D(center.x() - halfW, topY, center.z() - halfD),
        color
        );

    // Bottom face (underside)
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(center.x() - halfW, bottomY, center.z() - halfD),
        QVector3D(center.x() + halfW, bottomY, center.z() - halfD),
        QVector3D(center.x() + halfW, bottomY, center.z() + halfD),
        QVector3D(center.x() - halfW, bottomY, center.z() + halfD),
        color
        );

    // Front face (toward -Z)
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(center.x() - halfW, bottomY, center.z() - halfD - offset),
        QVector3D(center.x() + halfW, bottomY, center.z() - halfD - offset),
        QVector3D(center.x() + halfW, topY,    center.z() - halfD - offset),
        QVector3D(center.x() - halfW, topY,    center.z() - halfD - offset),
        color
        );

    // Back face (toward +Z)
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(center.x() + halfW, bottomY, center.z() + halfD + offset),
        QVector3D(center.x() - halfW, bottomY, center.z() + halfD + offset),
        QVector3D(center.x() - halfW, topY,    center.z() + halfD + offset),
        QVector3D(center.x() + halfW, topY,    center.z() + halfD + offset),
        color
        );

    // Left face (toward -X)
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(center.x() - halfW, bottomY, center.z() + halfD),
        QVector3D(center.x() - halfW, bottomY, center.z() - halfD),
        QVector3D(center.x() - halfW, topY,    center.z() - halfD),
        QVector3D(center.x() - halfW, topY,    center.z() + halfD),
        color
        );

    // Right face (toward +X)
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(center.x() + halfW, bottomY, center.z() - halfD),
        QVector3D(center.x() + halfW, bottomY, center.z() + halfD),
        QVector3D(center.x() + halfW, topY,    center.z() + halfD),
        QVector3D(center.x() + halfW, topY,    center.z() - halfD),
        color
        );
}
