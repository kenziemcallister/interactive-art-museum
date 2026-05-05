/*
 * The reusable shape helper - in charge of turning simple shapes into vertices
 * Helps form rectangles for the paintings, walls, etc
 *
*/

#include "GeometryBuilder.h"

#include <QVector2D>

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

    // Triangle 1
    vertices.push_back({ a, color, noTexCoord, useTexture, textureIndex });
    vertices.push_back({ b, color, noTexCoord, useTexture, textureIndex });
    vertices.push_back({ c, color, noTexCoord, useTexture, textureIndex });

    // Triangle 2
    vertices.push_back({ a, color, noTexCoord, useTexture, textureIndex });
    vertices.push_back({ c, color, noTexCoord, useTexture, textureIndex });
    vertices.push_back({ d, color, noTexCoord, useTexture, textureIndex });
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

    // Triangle 1
    vertices.push_back({ a, whiteColor, texA, useTexture, textureIndex });
    vertices.push_back({ b, whiteColor, texB, useTexture, textureIndex });
    vertices.push_back({ c, whiteColor, texC, useTexture, textureIndex });

    // Triangle 2
    vertices.push_back({ a, whiteColor, texA, useTexture, textureIndex });
    vertices.push_back({ c, whiteColor, texC, useTexture, textureIndex });
    vertices.push_back({ d, whiteColor, texD, useTexture, textureIndex });
}
