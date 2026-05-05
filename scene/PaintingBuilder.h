#ifndef PAINTINGBUILDER_H
#define PAINTINGBUILDER_H

#include <vector>
#include <QVector3D>
#include "../rendering/Vertex.h"

class PaintingBuilder
{
public:
    static void addPaintings(std::vector<Vertex>& vertices);

private:
    static void addPaintingOnLeftWall(
        std::vector<Vertex>& vertices,
        float centerZ,
        float centerY,
        float x,
        float width,
        float height,
        float textureIndex
        );

    static void addPaintingOnRightWall(
        std::vector<Vertex>& vertices,
        float centerZ,
        float centerY,
        float x,
        float width,
        float height,
        float textureIndex
        );

    static void addBackWallFeaturePainting(
        std::vector<Vertex>& vertices,
        float backZ
        );
};

#endif
