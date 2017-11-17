#include "stdafx.h"
#include "Cube.h"

float cubeSize = 1.0f;

QVector2D rectVertex[4] = 
{
    QVector2D(-0.5f, -0.5f),
    QVector2D(0.5f, -0.5f),
    QVector2D(0.5f, 0.5f),
    QVector2D(-0.5f, 0.5f),
};

QVector3D faceColor[6] =
{
    QVector3D(1.0f, 0.0f, 0.0f),
    QVector3D(0.0f, 1.0f, 0.0f),
    QVector3D(0.0f, 0.0f, 1.0f),
    QVector3D(0.5f, 0.5f, 0.0f),
    QVector3D(0.0f, 0.5f, 0.5f),
    QVector3D(0.5f, 0.0f, 0.5f)
};

class CubeImpl
{
public:
    CubeImpl()
    {

    }
    ~CubeImpl()
    {

    }

public:
    QVector3D _orig;
    QMatrix4x4 _face[6];
    QVector3D _color[6];
};

Cube::Cube()
{
    d = new CubeImpl();
}

Cube::~Cube()
{
    delete d;
}

void Cube::init(QVector3D& orig)
{
    d->_orig = orig;

    QMatrix4x4 m;
    m.translate(orig);

    for (int i = 0; i < 6; i++)
    {
        d->_face[i] = getPlane(i);
        d->_face[i] = m * d->_face[i];
        d->_face[i].translate(0.0f, 0.0f, cubeSize * 0.5f);
        d->_color[i] = faceColor[i];
    }
}

QVector3D Cube::getOrig()
{
    return d->_orig;
}

void Cube::setXform(const QMatrix4x4& Xform)
{
    for (auto& face : d->_face)
    {
        face = Xform * face;
    }

    d->_orig = (Xform * QVector4D(d->_orig, 1.0f)).toVector3D();
}

void Cube::getVboData(QVector<float>& data)
{
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            QVector3D vertex = (d->_face[i] * QVector4D(rectVertex[j], 0.0f, 1.0f)).toVector3D();
            QVector3D normal = (d->_face[i].column(2)).toVector3D();
            data.push_back(vertex.x());
            data.push_back(vertex.y());
            data.push_back(vertex.z());
            data.push_back(normal.x());
            data.push_back(normal.y());
            data.push_back(normal.z());
            data.push_back(d->_color[i].x());
            data.push_back(d->_color[i].y());
            data.push_back(d->_color[i].z());
        }
    }
}
