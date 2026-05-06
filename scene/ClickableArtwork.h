#ifndef CLICKABLEARTWORK_H
#define CLICKABLEARTWORK_H

#include <QString>
#include <QVector3D>

// This stores both the artwork info and the 3D clickable rectangle.
// The rectangle uses the same 4 corner points as the visual painting/plaque.
struct ClickableArtwork
{
    QString title;
    QString artist;
    QString description;

    // 4 corners of the clickable area in 3D space.
    // These should match the painting or plaque rectangle.
    QVector3D a; // bottom-left
    QVector3D b; // bottom-right
    QVector3D c; // top-right
    QVector3D d; // top-left
};

#endif
