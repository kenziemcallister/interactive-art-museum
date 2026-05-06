#ifndef SCULPTUREBUILDER_H
#define SCULPTUREBUILDER_H

#include <vector>
#include <QString>
#include <QVector3D>
#include "../rendering/Vertex.h"

class SculptureBuilder
{
public:
    // takes giant list of vertices, file path, where to put it, & how big it should be
    static void addSculpture(std::vector<Vertex>& vertices,
                             const QString& objResourcePath,
                             const QString& mtlResourcePath,
                             QVector3D position,
                             float scale,
                             float textureIndex);
};

#endif // SCULPTUREBUILDER_H
