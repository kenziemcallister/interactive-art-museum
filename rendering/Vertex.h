/*
 This file defines what one vertex contains
 Vertex == one point of a triangle. All geomtry is made of triangles
 So every wall, floor, frame, and painting is built from the struct below
*/

#ifndef VERTEX_H
#define VERTEX_H

#include <QVector2D>
#include <QVector3D>

// One vertex in our 3D scene.
// This stores position, color, texture coordinates,
// whether this vertex should use a texture,
// and which texture it should use.
struct Vertex
{
    QVector3D position; //where point is in 3D space
    QVector3D color; //fallback color for the vertex
    QVector2D texCoord; //what part of the image maps to this point

    // 0.0 = use plain color
    // 1.0 = use texture
    float useTexture;

    // Which texture slot to use.
    // Example:
    // 0 = Nighthawk
    // 1 = second painting
    // 2 = third painting
    float textureIndex;
};

#endif
