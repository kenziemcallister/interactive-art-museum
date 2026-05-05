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
        const QVector3D& d
        );
};

#endif
