#include "stdafx.h"
#include "Cube.h"
#include "isect.h"

QVector3D g_normal[6] = 
{
    QVector3D(0.0f, 0.0f, 1.0f),
    QVector3D(0.0f, 0.0f, -1.0f),
    QVector3D(0.0f, -1.0f, 0.0f),
    QVector3D(0.0f, 1.0f, 0.0f),
    QVector3D(1.0f, 0.0f, 0.0f),
    QVector3D(-1.0f, 0.0f, 0.0f)
};

QVector3D g_color[6] =
{
    QVector3D(1.0f, 0.0f, 0.0f),
    QVector3D(0.0f, 1.0f, 0.0f),
    QVector3D(0.0f, 0.0f, 1.0f),
    QVector3D(0.5f, 0.5f, 0.0f),
    QVector3D(0.0f, 0.5f, 0.5f),
    QVector3D(0.5f, 0.0f, 0.5f)
};



struct Face
{
    QVector3D* _vertex[4];
    QVector3D* _normal;
    QVector3D  _color;
};

class CubeImpl
{
public:
    CubeImpl(QVector3D& orig)
    {
        /* all vertexes */
        for (int i = 0; i < 8; i++)
        {
            _vertex[i] = QVector3D(orig.x() + (i & 1 ? 0.5f : -0.5f),
                orig.y() + (i & 2 ? 0.5f : -0.5f),
                orig.z() + (i & 4 ? 0.5f : -0.5f));
        }

        memcpy(_normal, g_normal, sizeof(_normal));

        faceBindVertex(D_FRONT, 0, 1, 3, 2);
        faceBindVertex(D_BACK, 4, 6, 7, 5);
        faceBindVertex(D_LEFT, 0, 2, 6, 4);
        faceBindVertex(D_RIGHT, 1, 5, 7, 3);
        faceBindVertex(D_UP, 2, 3, 7, 6);
        faceBindVertex(D_DOWN, 0, 4, 5, 1);

    }
    ~CubeImpl()
    {

    }

public:
    void faceBindVertex(int iFace, int iVertex0, int iVertex1, int iVertex2, int iVertex3)
    {
        _face[iFace]._vertex[0] = &_vertex[iVertex0];
        _face[iFace]._vertex[1] = &_vertex[iVertex1];
        _face[iFace]._vertex[2] = &_vertex[iVertex2];
        _face[iFace]._vertex[3] = &_vertex[iVertex3];
        _face[iFace]._normal = &_normal[iFace];
        _face[iFace]._color = g_color[iFace];
    }

    void genFaceVBO()
    {
        QVector<float> data;
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                data.push_back(_face[i]._vertex[j]->x());
                data.push_back(_face[i]._vertex[j]->y());
                data.push_back(_face[i]._vertex[j]->z());
                data.push_back(_face[i]._normal->x());
                data.push_back(_face[i]._normal->y());
                data.push_back(_face[i]._normal->z());
                data.push_back(_face[i]._color.x());
                data.push_back(_face[i]._color.y());
                data.push_back(_face[i]._color.z());
            }
        }

        _faceVBO.create();
        _faceVBO.bind();
        _faceVBO.allocate(&data[0], data.size() * sizeof(float));
    }

public:
    Face _face[6];

    QVector3D _vertex[8];
    QVector3D _normal[6];

    QOpenGLBuffer _faceVBO;
};

Cube::Cube(QVector3D& orig)
{
    d = new CubeImpl(orig);
}

Cube::~Cube()
{
    delete d;
}

void Cube::init()
{
    initializeOpenGLFunctions();
}

void Cube::drawFace(QOpenGLShaderProgram &faceShader)
{
    if (!d->_faceVBO.isCreated())
    {
        d->genFaceVBO();
    }

    d->_faceVBO.bind();

    int vertexLocation = faceShader.attributeLocation("vertex");
    faceShader.enableAttributeArray(vertexLocation);
    faceShader.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, 9 * sizeof(float));

    int normalLocation = faceShader.attributeLocation("normal");
    faceShader.enableAttributeArray(normalLocation);
    faceShader.setAttributeBuffer(normalLocation, GL_FLOAT, 3 * sizeof(float), 3, 9 * sizeof(float));

    int colorLocation = faceShader.attributeLocation("color");
    faceShader.enableAttributeArray(colorLocation);
    faceShader.setAttributeBuffer(colorLocation, GL_FLOAT, 6 * sizeof(float), 3, 9 * sizeof(float));

    glDrawArrays(GL_QUADS, 0, 4 * 6);

    d->_faceVBO.release();
}

void Cube::drawEdge(QOpenGLShaderProgram &edgeShader)
{
    if (!d->_faceVBO.isCreated())
    {
        d->genFaceVBO();
    }

    d->_faceVBO.bind();

    int vertexLocation = edgeShader.attributeLocation("vertex");
    edgeShader.enableAttributeArray(vertexLocation);
    edgeShader.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, 9 * sizeof(float));

    glDrawArrays(GL_QUADS, 0, 4 * 6);

    d->_faceVBO.release();
}

bool Cube::checkPick(const QVector3D& linePnt, const QVector3D& lineVec, int& nearFaces, QVector3D& nearPnts)
{
    QVector3D orig;
    for (auto v : d->_vertex)
    {
        orig += v;
    }
    orig /= 8.0f;

    QVector3D boxMin(orig.x() - 0.5f, orig.y() - 0.5f, orig.z() - 0.5f);
    QVector3D boxMax(orig.x() + 0.5f, orig.y() + 0.5f, orig.z() + 0.5f);

    return isectLine2Box(linePnt, lineVec, boxMin, boxMax, nearFaces, nearPnts);
}
