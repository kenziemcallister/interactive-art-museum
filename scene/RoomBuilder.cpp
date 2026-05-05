#include "../scene/RoomBuilder.h"
#include "../rendering/Vertex.h"

#include "../geometry/GeometryBuilder.h"

#include <QVector3D>

void RoomBuilder::addMuseumRooms(std::vector<Vertex>& vertices)
{
    // Museum layout:
    // x = left/right
    // y = up/down
    // z = depth
    //
    // Room 1 goes from frontZ to middleZ.
    // Room 2 goes from middleZ to backZ.
    //
    // The shared wall at middleZ has a doorway cut into it.

    const float left = -6.0f;
    const float right = 6.0f;

    const float floorY = 0.0f;
    const float ceilingY = 4.5f;

    const float frontZ = 2.0f;
    const float middleZ = -10.0f;
    const float backZ = -22.0f;

    // Doorway dimensions on the wall between Room 1 and Room 2.
    const float doorLeft = -1.5f;
    const float doorRight = 1.5f;
    const float doorTop = 3.0f;

    // Simple colors for now.
    QVector3D floorColor(0.45f, 0.38f, 0.30f);
    QVector3D wallColor(0.78f, 0.74f, 0.66f);
    QVector3D sideWallColor(0.68f, 0.65f, 0.58f);
    QVector3D ceilingColor(0.86f, 0.84f, 0.78f);

    // Helper function:
    // Adds a full rectangular room shell.
    //
    // This does NOT add front/back walls by default because we want control
    // over doorways. It adds:
    // floor, ceiling, left wall, right wall.
    auto addRoomShell = [&](float zFront, float zBack)
    {
        // Floor
        GeometryBuilder::addColoredRectangle(
            vertices,
            QVector3D(left,  floorY, zFront),
            QVector3D(right, floorY, zFront),
            QVector3D(right, floorY, zBack),
            QVector3D(left,  floorY, zBack),
            floorColor
            );

        // Ceiling
        GeometryBuilder::addColoredRectangle(
            vertices,
            QVector3D(left,  ceilingY, zFront),
            QVector3D(left,  ceilingY, zBack),
            QVector3D(right, ceilingY, zBack),
            QVector3D(right, ceilingY, zFront),
            ceilingColor
            );

        // Left wall
        GeometryBuilder::addColoredRectangle(
            vertices,
            QVector3D(left, floorY,   zFront),
            QVector3D(left, floorY,   zBack),
            QVector3D(left, ceilingY, zBack),
            QVector3D(left, ceilingY, zFront),
            sideWallColor
            );

        // Right wall
        GeometryBuilder::addColoredRectangle(
            vertices,
            QVector3D(right, floorY,   zBack),
            QVector3D(right, floorY,   zFront),
            QVector3D(right, ceilingY, zFront),
            QVector3D(right, ceilingY, zBack),
            sideWallColor
            );
    };

    // Helper function:
    // Adds a solid wall at a constant z value.
    auto addSolidWallAtZ = [&](float z)
    {
        GeometryBuilder::addColoredRectangle(
            vertices,
            QVector3D(left,  floorY,   z),
            QVector3D(right, floorY,   z),
            QVector3D(right, ceilingY, z),
            QVector3D(left,  ceilingY, z),
            wallColor
            );
    };

    // Helper function:
    // Adds a wall at a constant z value, but leaves a rectangular doorway open.
    auto addWallWithDoorAtZ = [&](float z)
    {
        // Left piece of wall beside doorway
        GeometryBuilder::addColoredRectangle(
            vertices,
            QVector3D(left,     floorY,   z),
            QVector3D(doorLeft, floorY,   z),
            QVector3D(doorLeft, ceilingY, z),
            QVector3D(left,     ceilingY, z),
            wallColor
            );

        // Right piece of wall beside doorway
        GeometryBuilder::addColoredRectangle(
            vertices,
            QVector3D(doorRight, floorY,   z),
            QVector3D(right,     floorY,   z),
            QVector3D(right,     ceilingY, z),
            QVector3D(doorRight, ceilingY, z),
            wallColor
            );

        // Top piece of wall above doorway
        GeometryBuilder::addColoredRectangle(
            vertices,
            QVector3D(doorLeft,  doorTop,  z),
            QVector3D(doorRight, doorTop,  z),
            QVector3D(doorRight, ceilingY, z),
            QVector3D(doorLeft,  ceilingY, z),
            wallColor
            );
    };

    // -------------------------
    // Build the museum layout
    // -------------------------

    // Room 1 shell
    addRoomShell(frontZ, middleZ);

    // Room 2 shell
    addRoomShell(middleZ, backZ);

    // Front wall of Room 1.
    // This is behind the player at the start.
    addSolidWallAtZ(frontZ);

    // Shared wall between Room 1 and Room 2, with doorway.
    addWallWithDoorAtZ(middleZ);

    // Back wall of Room 2.
    addSolidWallAtZ(backZ);
}
