#include "PaintingBuilder.h"

#include "../geometry/GeometryBuilder.h"

#include <QVector3D>

void PaintingBuilder::addPaintings(std::vector<Vertex>& vertices)
{
    // Museum wall constants.
    // These should match the room dimensions in RoomBuilder.cpp.
    const float left = -6.0f;
    const float right = 6.0f;
    const float backZ = -22.0f;

    // -------------------------
    // Add placeholder paintings
    // -------------------------
    //
    // These are colored placeholders for now.
    // Later, we can replace the artColor with real National Gallery of Art image textures.

    // Room 1 left wall paintings
    addPaintingOnLeftWall(
        vertices,
        -3.0f,      // center z
        2.2f,       // center y
        left,       // wall x position
        2.0f,       // painting width
        1.3f,       // painting height
        QVector3D(0.15f, 0.30f, 0.75f) // blue artwork
        );

    addPaintingOnLeftWall(
        vertices,
        -7.0f,
        2.2f,
        left,
        2.0f,
        1.3f,
        QVector3D(0.70f, 0.25f, 0.15f) // red artwork
        );

    // Room 1 right wall paintings
    addPaintingOnRightWall(
        vertices,
        -3.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        QVector3D(0.20f, 0.60f, 0.35f) // green artwork
        );

    addPaintingOnRightWall(
        vertices,
        -7.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        QVector3D(0.65f, 0.45f, 0.15f) // gold artwork
        );

    // Room 2 left wall paintings
    addPaintingOnLeftWall(
        vertices,
        -13.0f,
        2.2f,
        left,
        2.0f,
        1.3f,
        QVector3D(0.55f, 0.20f, 0.65f) // purple artwork
        );

    // Room 2 right wall paintings
    addPaintingOnRightWall(
        vertices,
        -13.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        QVector3D(0.10f, 0.55f, 0.65f) // teal artwork
        );

    // Large feature painting on the back wall of Room 2.
    // This one uses an actual image texture.
    addBackWallFeaturePainting(vertices, backZ);
}

void PaintingBuilder::addPaintingOnLeftWall(
    std::vector<Vertex>& vertices,
    float centerZ,
    float centerY,
    float x,
    float width,
    float height,
    const QVector3D& artColor
    )
{
    // Adds a framed painting on the left wall where x stays constant.
    // The painting stretches along z and y.

    QVector3D frameColor(0.05f, 0.03f, 0.02f);

    float halfW = width / 2.0f;
    float halfH = height / 2.0f;

    float offset = 0.04f;
    float framePadding = 0.15f;

    // Frame rectangle
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(x + offset, centerY - halfH - framePadding, centerZ + halfW + framePadding),
        QVector3D(x + offset, centerY - halfH - framePadding, centerZ - halfW - framePadding),
        QVector3D(x + offset, centerY + halfH + framePadding, centerZ - halfW - framePadding),
        QVector3D(x + offset, centerY + halfH + framePadding, centerZ + halfW + framePadding),
        frameColor
        );

    // Artwork rectangle
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(x + offset * 2.0f, centerY - halfH, centerZ + halfW),
        QVector3D(x + offset * 2.0f, centerY - halfH, centerZ - halfW),
        QVector3D(x + offset * 2.0f, centerY + halfH, centerZ - halfW),
        QVector3D(x + offset * 2.0f, centerY + halfH, centerZ + halfW),
        artColor
        );
}

void PaintingBuilder::addPaintingOnRightWall(
    std::vector<Vertex>& vertices,
    float centerZ,
    float centerY,
    float x,
    float width,
    float height,
    const QVector3D& artColor
    )
{
    // Adds a framed painting on the right wall where x stays constant.
    // The painting stretches along z and y.

    QVector3D frameColor(0.05f, 0.03f, 0.02f);

    float halfW = width / 2.0f;
    float halfH = height / 2.0f;

    float offset = 0.04f;
    float framePadding = 0.15f;

    // Frame rectangle
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(x - offset, centerY - halfH - framePadding, centerZ - halfW - framePadding),
        QVector3D(x - offset, centerY - halfH - framePadding, centerZ + halfW + framePadding),
        QVector3D(x - offset, centerY + halfH + framePadding, centerZ + halfW + framePadding),
        QVector3D(x - offset, centerY + halfH + framePadding, centerZ - halfW - framePadding),
        frameColor
        );

    // Artwork rectangle
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(x - offset * 2.0f, centerY - halfH, centerZ - halfW),
        QVector3D(x - offset * 2.0f, centerY - halfH, centerZ + halfW),
        QVector3D(x - offset * 2.0f, centerY + halfH, centerZ + halfW),
        QVector3D(x - offset * 2.0f, centerY + halfH, centerZ - halfW),
        artColor
        );
}

void PaintingBuilder::addBackWallFeaturePainting(std::vector<Vertex>& vertices, float backZ)
{
    // Large feature painting on the back wall of Room 2.
    // This one uses an actual image texture.

    QVector3D frameColor(0.05f, 0.03f, 0.02f);

    float paintingCenterX = 0.0f;
    float paintingCenterY = 2.3f;
    float paintingZ = backZ;

    float paintingWidth = 3.0f;
    float paintingHeight = 1.7f;

    float halfW = paintingWidth / 2.0f;
    float halfH = paintingHeight / 2.0f;

    float framePadding = 0.18f;
    float wallOffset = 0.04f;

    // Frame
    GeometryBuilder::addColoredRectangle(
        vertices,
        QVector3D(paintingCenterX - halfW - framePadding, paintingCenterY - halfH - framePadding, paintingZ + wallOffset),
        QVector3D(paintingCenterX + halfW + framePadding, paintingCenterY - halfH - framePadding, paintingZ + wallOffset),
        QVector3D(paintingCenterX + halfW + framePadding, paintingCenterY + halfH + framePadding, paintingZ + wallOffset),
        QVector3D(paintingCenterX - halfW - framePadding, paintingCenterY + halfH + framePadding, paintingZ + wallOffset),
        frameColor
        );

    // Textured artwork
    GeometryBuilder::addTexturedRectangle(
        vertices,
        QVector3D(paintingCenterX - halfW, paintingCenterY - halfH, paintingZ + wallOffset * 2.0f),
        QVector3D(paintingCenterX + halfW, paintingCenterY - halfH, paintingZ + wallOffset * 2.0f),
        QVector3D(paintingCenterX + halfW, paintingCenterY + halfH, paintingZ + wallOffset * 2.0f),
        QVector3D(paintingCenterX - halfW, paintingCenterY + halfH, paintingZ + wallOffset * 2.0f)
        );
}
