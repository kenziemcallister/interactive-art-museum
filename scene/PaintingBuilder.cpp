/*
 * Controls the paintings and frames
 * Where paintings are, which wall they're on, how big, which texture index
 * Frame + artwork rectangles
 * This is the file used to add/remove paintings from the wall
*/

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
    // Add paintings
    // -------------------------
    //
    // These used to be colored placeholders.
    // Now each painting uses a texture index that matches the texture loaded in MuseumWidget.

    // Room 1 left wall paintings
    addPaintingOnLeftWall(
        vertices,
        -3.0f,
        2.2f,
        left,
        2.0f,
        1.3f,
        1.0f // texture index
        );

    addPaintingOnLeftWall(
        vertices,
        -7.0f,
        2.2f,
        left,
        2.0f,
        1.3f,
        2.0f // texture index
        );

    // Room 1 right wall paintings
    addPaintingOnRightWall(
        vertices,
        -3.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        3.0f // texture index
        );

    addPaintingOnRightWall( //(portrait)
        vertices,
        -7.0f,
        2.5f,  // slightly higher center y
        right,
        1.3f,  // portrait width
        2.0f,  // portrait height
        4.0f
        );

    // Room 2 left wall paintings (portrait)
    addPaintingOnLeftWall(
        vertices,
        -13.0f,
        2.5f,
        left,
        1.3f,
        2.0f,
        5.0f // texture index
        );

    // Room 2 right wall paintings
    addPaintingOnRightWall(
        vertices,
        -13.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        6.0f // texture index
        );

    addBackWallFeaturePainting(vertices, backZ);
}

void PaintingBuilder::addPaintingOnLeftWall(
    std::vector<Vertex>& vertices,
    float centerZ,
    float centerY,
    float x,
    float width,
    float height,
    float textureIndex
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
    GeometryBuilder::addTexturedRectangle(
        vertices,
        QVector3D(x + offset * 2.0f, centerY - halfH, centerZ + halfW),
        QVector3D(x + offset * 2.0f, centerY - halfH, centerZ - halfW),
        QVector3D(x + offset * 2.0f, centerY + halfH, centerZ - halfW),
        QVector3D(x + offset * 2.0f, centerY + halfH, centerZ + halfW),
        textureIndex
        );
}

void PaintingBuilder::addPaintingOnRightWall(
    std::vector<Vertex>& vertices,
    float centerZ,
    float centerY,
    float x,
    float width,
    float height,
    float textureIndex
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
    GeometryBuilder::addTexturedRectangle(
        vertices,
        QVector3D(x - offset * 2.0f, centerY - halfH, centerZ - halfW),
        QVector3D(x - offset * 2.0f, centerY - halfH, centerZ + halfW),
        QVector3D(x - offset * 2.0f, centerY + halfH, centerZ + halfW),
        QVector3D(x - offset * 2.0f, centerY + halfH, centerZ - halfW),
        textureIndex
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
        QVector3D(paintingCenterX - halfW, paintingCenterY + halfH, paintingZ + wallOffset * 2.0f),
        0.0f // texture index for Nighthawk / feature painting
        );
}
