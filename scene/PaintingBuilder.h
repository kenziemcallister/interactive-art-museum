#ifndef PAINTINGBUILDER_H
#define PAINTINGBUILDER_H

#include <vector>
#include <QVector3D>
#include <QString>

#include "../rendering/Vertex.h"
#include "ClickableArtwork.h"

class PaintingBuilder
{
public:
    static void addPaintings(
        std::vector<Vertex>& vertices,
        std::vector<ClickableArtwork>& clickableArtworks
        );

private:
    static void addPaintingOnLeftWall(
        std::vector<Vertex>& vertices,
        std::vector<ClickableArtwork>& clickableArtworks,
        float centerZ,
        float centerY,
        float x,
        float width,
        float height,
        float textureIndex,
        const QString& title,
        const QString& artist,
        const QString& description
        );

    static void addPaintingOnRightWall(
        std::vector<Vertex>& vertices,
        std::vector<ClickableArtwork>& clickableArtworks,
        float centerZ,
        float centerY,
        float x,
        float width,
        float height,
        float textureIndex,
        const QString& title,
        const QString& artist,
        const QString& description
        );

    static void addPaintingOnFrontWall(
        std::vector<Vertex>& vertices,
        std::vector<ClickableArtwork>& clickableArtworks,
        float centerX,
        float centerY,
        float z,
        float width,
        float height,
        float textureIndex,
        const QString& title,
        const QString& artist,
        const QString& description
        );

    static void addBackWallFeaturePainting(
        std::vector<Vertex>& vertices,
        std::vector<ClickableArtwork>& clickableArtworks,
        float backZ
        );
};

#endif
