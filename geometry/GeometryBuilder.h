#ifndef GEOMETRYBUILDER_H
#define GEOMETRYBUILDER_H

#include <vector>
#include <QVector3D>
#include "../rendering/Vertex.h"

class GeometryBuilder
{
public:
    static void addColoredRectangle(
        std::vector<Vertex>& vertices,
        const QVector3D& a,
        const QVector3D& b,
        const QVector3D& c,
        const QVector3D& d,
        const QVector3D& color
        );

    static void addTexturedRectangle(
        std::vector<Vertex>& vertices,
        const QVector3D& a,
        const QVector3D& b,
        const QVector3D& c,
        const QVector3D& d,
        float textureIndex
        );

    static void addTexturedRectangleTiled(
        std::vector<Vertex>& vertices,
        const QVector3D& a,
        const QVector3D& b,
        const QVector3D& c,
        const QVector3D& d,
        float textureIndex,
        float tileX,
        float tileY
        );

    // Adds a simple rectangular stool (solid box) centered at `center`.
    // width = size along X, depth = size along Z, height = size along Y.
    // color is the RGB color of all faces. offset avoids Z-fighting with floor.
    static void addStool(
        std::vector<Vertex>& vertices,
        const QVector3D& center,
        float width,
        float depth,
        float height,
        const QVector3D& color,
        float offset = 0.02f
        );

    // computes normal & uses it when pushing each vertex made in addColoredRectangle & addTexturedRectangle
    static QVector3D computeFaceNormal(const QVector3D &a, const QVector3D &b, const QVector3D &c);
};

#endif
