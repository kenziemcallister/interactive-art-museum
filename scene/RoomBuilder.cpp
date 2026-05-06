/*
 * Responsible for building the physical museum space.
 * Creates:
 *  - room size
 *  - floor
 *  - ceiling
 *  - left wall
 *  - right wall
 *  - front wall
 *  - back wall
 *  - middle wall
 *  - doorway opening
 *  - basic room colors
*/

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
    //
    // Room 3 branches off the right wall of Room 2.

    //room constants below (this wide, ths high, doorway location, etc)
    const float left = -6.0f;
    const float right = 6.0f;

    const float floorY = 0.0f;
    const float ceilingY = 4.5f;

    const float frontZ = 2.0f;
    const float middleZ = -10.0f;
    const float backZ = -22.0f;

    //this room starts at the right wall of Room 2 and extends farther right
    const float room3Left = right;
    const float room3Right = 18.0f;

    const float room3FrontZ = -16.0f;
    const float room3BackZ = -22.0f;

    //doorway dimensions on the wall between Room 1 and Room 2
    const float doorLeft = -1.5f;
    const float doorRight = 1.5f;
    const float doorTop = 3.0f;

    // Doorway dimensions on the right wall of Room 2 into Room 3.
    // Since this door is on a wall where x stays constant,
    // the opening is controlled by z values instead of x values.
    const float sideDoorFrontZ = -17.0f;
    const float sideDoorBackZ = -20.0f;

    // Simple colors
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
        GeometryBuilder::addTexturedRectangle(
            vertices,
            QVector3D(left,  floorY, zFront),
            QVector3D(right, floorY, zFront),
            QVector3D(right, floorY, zBack),
            QVector3D(left,  floorY, zBack),
            8.0f
            );

        // Ceiling
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(left,  ceilingY, zFront),
            QVector3D(left,  ceilingY, zBack),
            QVector3D(right, ceilingY, zBack),
            QVector3D(right, ceilingY, zFront),
            9.0f,
            1.5f,
            1.5f
            );

        // Left wall
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(left, floorY,   zFront),
            QVector3D(left, floorY,   zBack),
            QVector3D(left, ceilingY, zBack),
            QVector3D(left, ceilingY, zFront),
            7.0f,
            0.5f,
            0.5f
            );

        // Right wall
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(right, floorY,   zBack),
            QVector3D(right, floorY,   zFront),
            QVector3D(right, ceilingY, zFront),
            QVector3D(right, ceilingY, zBack),
            7.0f,
            0.5f,
            0.5f
            );
    };

    // Helper function:
    // Adds a solid wall at a constant z value.
    auto addSolidWallAtZ = [&](float z)
    {
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(left,  floorY,   z),
            QVector3D(right, floorY,   z),
            QVector3D(right, ceilingY, z),
            QVector3D(left,  ceilingY, z),
            7.0f,
            0.5f,
            0.5f
            );
    };

    // Helper function:
    // Adds a wall at a constant z value, but leaves a rectangular doorway open.
    auto addWallWithDoorAtZ = [&](float z)
    {
        // Left piece of wall beside doorway
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(left,     floorY,   z),
            QVector3D(doorLeft, floorY,   z),
            QVector3D(doorLeft, ceilingY, z),
            QVector3D(left,     ceilingY, z),
            7.0f,
            0.5f,
            0.5f
            );

        // Right piece of wall beside doorway
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(doorRight, floorY,   z),
            QVector3D(right,     floorY,   z),
            QVector3D(right,     ceilingY, z),
            QVector3D(doorRight, ceilingY, z),
            7.0f,
            0.5f,
            0.5f
            );

        // Top piece of wall above doorway
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(doorLeft,  doorTop,  z),
            QVector3D(doorRight, doorTop,  z),
            QVector3D(doorRight, ceilingY, z),
            QVector3D(doorLeft,  ceilingY, z),
            7.0f,
            0.5f,
            0.5f
            );
    };

    // Helper function:
    // Adds a wall at a constant x value, but leaves a rectangular doorway open.
    //
    // This is used for the doorway from Room 2 into Room 3.
    // Since this wall is on the side, the doorway opening uses z positions.
    auto addWallWithDoorAtX = [&](float x,
                                  float zFront,
                                  float zBack,
                                  float doorFrontZ,
                                  float doorBackZ)
    {
        // Front piece of wall before doorway
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(x, floorY,   zFront),
            QVector3D(x, floorY,   doorFrontZ),
            QVector3D(x, ceilingY, doorFrontZ),
            QVector3D(x, ceilingY, zFront),
            7.0f,
            0.5f,
            0.5f
            );

        // Back piece of wall after doorway
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(x, floorY,   doorBackZ),
            QVector3D(x, floorY,   zBack),
            QVector3D(x, ceilingY, zBack),
            QVector3D(x, ceilingY, doorBackZ),
            7.0f,
            0.5f,
            0.5f
            );

        // Top piece of wall above doorway
        GeometryBuilder::addTexturedRectangleTiled(
            vertices,
            QVector3D(x, doorTop,  doorFrontZ),
            QVector3D(x, doorTop,  doorBackZ),
            QVector3D(x, ceilingY, doorBackZ),
            QVector3D(x, ceilingY, doorFrontZ),
            7.0f,
            0.5f,
            0.5f
            );
    };

    // -------------------------
    // Build the museum layout
    // -------------------------

    // Room 1 shell
    addRoomShell(frontZ, middleZ);

    // Room 2 is built manually instead of using addRoomShell.
    // This is because Room 2 needs a doorway on its right wall into Room 3.

    // Room 2 floor
    GeometryBuilder::addTexturedRectangle(
        vertices,
        QVector3D(left,  floorY, middleZ),
        QVector3D(right, floorY, middleZ),
        QVector3D(right, floorY, backZ),
        QVector3D(left,  floorY, backZ),
        8.0f
        );

    // Room 2 ceiling
    GeometryBuilder::addTexturedRectangleTiled(
        vertices,
        QVector3D(left,  ceilingY, middleZ),
        QVector3D(left,  ceilingY, backZ),
        QVector3D(right, ceilingY, backZ),
        QVector3D(right, ceilingY, middleZ),
        9.0f,
        1.5f,
        1.5f
        );

    // Room 2 left wall
    GeometryBuilder::addTexturedRectangleTiled(
        vertices,
        QVector3D(left, floorY,   middleZ),
        QVector3D(left, floorY,   backZ),
        QVector3D(left, ceilingY, backZ),
        QVector3D(left, ceilingY, middleZ),
        7.0f,
        0.5f,
        0.5f
        );

    // Room 2 right wall with doorway into Room 3
    addWallWithDoorAtX(
        right,
        middleZ,
        backZ,
        sideDoorFrontZ,
        sideDoorBackZ
        );

    // Room 3 branches off the right side of Room 2.

    // Room 3 floor
    GeometryBuilder::addTexturedRectangleTiled(
        vertices,
        QVector3D(room3Left,  floorY, room3FrontZ),
        QVector3D(room3Right, floorY, room3FrontZ),
        QVector3D(room3Right, floorY, room3BackZ),
        QVector3D(room3Left,  floorY, room3BackZ),
        8.0f,
        1.0,
        0.5
        );

    // Room 3 ceiling
    GeometryBuilder::addTexturedRectangleTiled(
        vertices,
        QVector3D(room3Left,  ceilingY, room3FrontZ),
        QVector3D(room3Left,  ceilingY, room3BackZ),
        QVector3D(room3Right, ceilingY, room3BackZ),
        QVector3D(room3Right, ceilingY, room3FrontZ),
        9.0f,
        1.5,
        1.5
        );

    // Room 3 far right wall
    GeometryBuilder::addTexturedRectangleTiled(
        vertices,
        QVector3D(room3Right, floorY,   room3BackZ),
        QVector3D(room3Right, floorY,   room3FrontZ),
        QVector3D(room3Right, ceilingY, room3FrontZ),
        QVector3D(room3Right, ceilingY, room3BackZ),
        7.0f,
        0.5f,
        0.5f
        );

    // Room 3 front wall
    GeometryBuilder::addTexturedRectangleTiled(
        vertices,
        QVector3D(room3Left,  floorY,   room3FrontZ),
        QVector3D(room3Right, floorY,   room3FrontZ),
        QVector3D(room3Right, ceilingY, room3FrontZ),
        QVector3D(room3Left,  ceilingY, room3FrontZ),
        7.0f,
        0.5f,
        0.5f
        );

    // Room 3 back wall
    GeometryBuilder::addTexturedRectangleTiled(
        vertices,
        QVector3D(room3Right, floorY,   room3BackZ),
        QVector3D(room3Left,  floorY,   room3BackZ),
        QVector3D(room3Left,  ceilingY, room3BackZ),
        QVector3D(room3Right, ceilingY, room3BackZ),
        7.0f,
        0.5f,
        0.5f
        );

    // Front wall of Room 1.
    // This is behind the player at the start.
    addSolidWallAtZ(frontZ);

    // Shared wall between Room 1 and Room 2, with doorway.
    addWallWithDoorAtZ(middleZ);

    // Back wall of Room 2.
    // This only covers Room 2 from left to right.
    // Room 3 has its own back wall above.
    addSolidWallAtZ(backZ);
}
