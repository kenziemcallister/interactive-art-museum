/*
 * Controls the paintings and frames
 * Where paintings are, which wall they're on, how big, which texture index
 * Frame + artwork rectangles
 * This is the file used to add/remove paintings from the wall
*/

#include "PaintingBuilder.h"

#include "../geometry/GeometryBuilder.h"

#include <QVector3D>

void PaintingBuilder::addPaintings(
    std::vector<Vertex>& vertices,
    std::vector<ClickableArtwork>& clickableArtworks
    )
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
    // Each painting also stores a clickable rectangle for raycasting.

    // Room 1 left wall paintings
    addPaintingOnLeftWall(
        vertices,
        clickableArtworks,
        -3.0f,
        2.2f,
        left,
        2.0f,
        1.3f,
        1.0f, // texture index
        "Bedroom in Arles",
        "Vincent van Gogh",
        "Vincent van Gogh’s Bedroom in Arles presents the artist’s simple room through bold color, flattened perspective, and expressive brushwork. The quiet interior becomes more than a living space; it reflects Van Gogh’s desire for rest, comfort, and emotional stability."
        );

    addPaintingOnLeftWall(
        vertices,
        clickableArtworks,
        -7.0f,
        2.2f,
        left,
        2.0f,
        1.3f,
        2.0f, // texture index
        "Georgia O'Keeffe Painting",
        "Georgia O'Keeffe",
        "Georgia O’Keeffe is known for transforming natural forms into powerful, abstract compositions. Through enlarged shapes, smooth color transitions, and close-up detail, her work invites viewers to slow down and notice beauty in forms that might otherwise be overlooked."
        );

    // Room 1 right wall paintings
    addPaintingOnRightWall(
        vertices,
        clickableArtworks,
        -3.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        3.0f, // texture index
        "The Starry Night",
        "Vincent van Gogh",
        "The Starry Night captures the night sky as a swirling, energetic landscape of movement and emotion. Van Gogh’s expressive brushstrokes and vivid contrasts turn the quiet village below into a dramatic scene shaped by memory, imagination, and feeling."
        );

    addPaintingOnRightWall( //(portrait)
        vertices,
        clickableArtworks,
        -7.0f,
        2.5f,  // slightly higher center y
        right,
        1.3f,  // portrait width
        2.0f,  // portrait height
        4.0f,  // texture index
        "Frida Kahlo",
        "Frida Kahlo",
        "Frida Kahlo’s portraits often combine personal identity, symbolism, and emotional honesty. Her work uses direct gaze, vivid color, and meaningful details to explore themes of pain, resilience, culture, and self-expression."
        );

    // Room 2 left wall paintings (portrait)
    addPaintingOnLeftWall(
        vertices,
        clickableArtworks,
        -13.0f,
        2.5f,
        left,
        1.3f,
        2.0f,
        5.0f, // texture index
        "Mona Lisa",
        "Leonardo da Vinci",
        "Leonardo da Vinci’s Mona Lisa is celebrated for its subtle expression, delicate modeling, and mysterious atmosphere. The sitter’s calm presence and faint smile have made the portrait one of the most studied and recognizable works in art history."
        );

    // Room 2 right wall paintings
    addPaintingOnRightWall(
        vertices,
        clickableArtworks,
        -13.0f,
        2.2f,
        right,
        2.0f,
        1.3f,
        6.0f, // texture index
        "Monet Painting",
        "Claude Monet",
        "Claude Monet’s impressionist paintings focus on light, color, and atmosphere rather than sharp detail. With loose brushwork and shifting tones, Monet captures a fleeting moment in nature and encourages viewers to experience the scene through sensation and mood."
        );

    addBackWallFeaturePainting(vertices, clickableArtworks, backZ);
}

void PaintingBuilder::addPaintingOnLeftWall(
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

    QVector3D artworkA(x + offset * 2.0f, centerY - halfH, centerZ + halfW);
    QVector3D artworkB(x + offset * 2.0f, centerY - halfH, centerZ - halfW);
    QVector3D artworkC(x + offset * 2.0f, centerY + halfH, centerZ - halfW);
    QVector3D artworkD(x + offset * 2.0f, centerY + halfH, centerZ + halfW);

    // Artwork rectangle
    GeometryBuilder::addTexturedRectangle(
        vertices,
        artworkA,
        artworkB,
        artworkC,
        artworkD,
        textureIndex
        );

    // Save the same rectangle corners so raycasting can detect clicks on this painting.
    clickableArtworks.push_back({
        title,
        artist,
        description,
        artworkA,
        artworkB,
        artworkC,
        artworkD
    });
}

void PaintingBuilder::addPaintingOnRightWall(
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

    QVector3D artworkA(x - offset * 2.0f, centerY - halfH, centerZ - halfW);
    QVector3D artworkB(x - offset * 2.0f, centerY - halfH, centerZ + halfW);
    QVector3D artworkC(x - offset * 2.0f, centerY + halfH, centerZ + halfW);
    QVector3D artworkD(x - offset * 2.0f, centerY + halfH, centerZ - halfW);

    // Artwork rectangle
    GeometryBuilder::addTexturedRectangle(
        vertices,
        artworkA,
        artworkB,
        artworkC,
        artworkD,
        textureIndex
        );

    // Save the same rectangle corners so raycasting can detect clicks on this painting.
    clickableArtworks.push_back({
        title,
        artist,
        description,
        artworkA,
        artworkB,
        artworkC,
        artworkD
    });
}

void PaintingBuilder::addBackWallFeaturePainting(
    std::vector<Vertex>& vertices,
    std::vector<ClickableArtwork>& clickableArtworks,
    float backZ
    )
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

    QVector3D artworkA(paintingCenterX - halfW, paintingCenterY - halfH, paintingZ + wallOffset * 2.0f);
    QVector3D artworkB(paintingCenterX + halfW, paintingCenterY - halfH, paintingZ + wallOffset * 2.0f);
    QVector3D artworkC(paintingCenterX + halfW, paintingCenterY + halfH, paintingZ + wallOffset * 2.0f);
    QVector3D artworkD(paintingCenterX - halfW, paintingCenterY + halfH, paintingZ + wallOffset * 2.0f);

    // Textured artwork
    GeometryBuilder::addTexturedRectangle(
        vertices,
        artworkA,
        artworkB,
        artworkC,
        artworkD,
        0.0f // texture index for Nighthawk / feature painting
        );

    // Save the same rectangle corners so raycasting can detect clicks on this painting.
    clickableArtworks.push_back({
        "Nighthawks",
        "Edward Hopper",
        "Edward Hopper’s Nighthawks portrays a quiet late-night diner illuminated against the darkness of the surrounding city. Through its sharp light, still figures, and sense of separation, the painting captures themes of urban isolation, silence, and modern life.",
        artworkA,
        artworkB,
        artworkC,
        artworkD
    });
}
