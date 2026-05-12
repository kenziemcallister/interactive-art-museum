#include "SculptureBuilder.h"
#include <QDebug>
#include <QFile>

// This magic line tells C++ to compile the library right here, right now.
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void SculptureBuilder::addSculpture(std::vector<Vertex>& vertices,
                                    const QString& objResourcePath,
                                    const QString& mtlResourcePath,
                                    QVector3D position,
                                    float scale,
                                    float textureIndex)
{
    // Read the .obj file from Qt resources into a string
    QFile objFile(objResourcePath);
    if (!objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "SculptureBuilder: Could not open OBJ resource:" << objResourcePath;
        return;
    }
    std::string objText = objFile.readAll().toStdString();
    objFile.close();

    // Read the .mtl file from Qt resources into a string
    QFile mtlFile(mtlResourcePath);
    if (!mtlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "SculptureBuilder: Could not open MTL resource:" << mtlResourcePath;
        return;
    }
    std::string mtlText = mtlFile.readAll().toStdString();
    mtlFile.close();

    // Parse using strings instead of file paths
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromString(objText, mtlText, reader_config)) {
        if (!reader.Error().empty()) {
            qDebug() << "TinyObjReader ERROR:" << QString::fromStdString(reader.Error());
        }
        return;
    }

    if (!reader.Warning().empty()) {
        qDebug() << "TinyObjReader WARNING:" << QString::fromStdString(reader.Warning());
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                Vertex vertex;

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                vertex.position = QVector3D(vx, vy, vz) * scale + position;

                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vertex.texCoord = QVector2D(tx, 1.0f - ty);
                    vertex.useTexture = 1.0f;
                    vertex.textureIndex = textureIndex;
                } else {
                    vertex.texCoord = QVector2D(0.0f, 0.0f);
                    vertex.useTexture = 0.0f;
                    vertex.textureIndex = 0.0f;
                }

                // read normals from file!
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    vertex.normal = QVector3D(nx, ny, nz).normalized();
                } else {
                    vertex.normal = QVector3D(0.0f, 1.0f, 0.0f); // fallback
                }

                vertex.color = QVector3D(0.8f, 0.8f, 0.8f);
                vertices.push_back(vertex);
            }
            index_offset += fv;
        }
    }
}
