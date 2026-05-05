#include "GeometryBuilder.h"

#include <QVector2D>

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

    // Triangle 1
    vertices.push_back({ a, color, noTexCoord, useTexture });
    vertices.push_back({ b, color, noTexCoord, useTexture });
    vertices.push_back({ c, color, noTexCoord, useTexture });

    // Triangle 2
    vertices.push_back({ a, color, noTexCoord, useTexture });
    vertices.push_back({ c, color, noTexCoord, useTexture });
    vertices.push_back({ d, color, noTexCoord, useTexture });
}

void GeometryBuilder::addTexturedRectangle(
    std::vector<Vertex>& vertices,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c,
    const QVector3D& d
    )
{
    QVector3D whiteColor(1.0f, 1.0f, 1.0f);
    float useTexture = 1.0f;

    QVector2D texA(0.0f, 0.0f);
    QVector2D texB(1.0f, 0.0f);
    QVector2D texC(1.0f, 1.0f);
    QVector2D texD(0.0f, 1.0f);

    // Triangle 1
    vertices.push_back({ a, whiteColor, texA, useTexture });
    vertices.push_back({ b, whiteColor, texB, useTexture });
    vertices.push_back({ c, whiteColor, texC, useTexture });

    // Triangle 2
    vertices.push_back({ a, whiteColor, texA, useTexture });
    vertices.push_back({ c, whiteColor, texC, useTexture });
    vertices.push_back({ d, whiteColor, texD, useTexture });
}
