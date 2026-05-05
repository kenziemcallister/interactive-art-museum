#ifndef VERTEX_H
#define VERTEX_H

#include <QVector2D>
#include <QVector3D>

// One vertex in our 3D scene.
// This stores position, color, texture coordinates,
// and whether this vertex should use a texture.
struct Vertex
{
    QVector3D position;
    QVector3D color;
    QVector2D texCoord;

    // 0.0 = use plain color
    // 1.0 = use texture
    float useTexture;
};

#endif
