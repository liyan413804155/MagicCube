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
    CubeImpl(QVector3D& orig)
    {
        _orig = orig;

        QMatrix4x4 m;
        m.translate(orig);

        for (int i = 0; i < 6; i++)
        {
            _face[i] = getPlane(i);
            _face[i] = m * _face[i];
            _face[i].translate(0.0f, 0.0f, cubeSize * 0.5f);

            _color[i] = faceColor[i];
        }
    }
    ~CubeImpl()
    {

    }

public:

    void genFaceVBO()
    {
        QVector<float> data;
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                QVector3D vertex = (_face[i] * QVector4D(rectVertex[j], 0.0f, 1.0f)).toVector3D();
                QVector3D normal = (_face[i].column(2)).toVector3D();
                data.push_back(vertex.x());
                data.push_back(vertex.y());
                data.push_back(vertex.z());
                data.push_back(normal.x());
                data.push_back(normal.y());
                data.push_back(normal.z());
                data.push_back(_color[i].x());
                data.push_back(_color[i].y());
                data.push_back(_color[i].z());
            }
        }

        _faceVBO.create();
        _faceVBO.bind();
        _faceVBO.allocate(&data[0], data.size() * sizeof(float));
    }

public:
    QVector3D _orig;
    QMatrix4x4 _face[6];
    QVector3D _color[6];

    QMatrix4x4 _model;

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

    faceShader.setUniformValue("model", d->_model);

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

    edgeShader.setUniformValue("model", d->_model);

    glDrawArrays(GL_QUADS, 0, 4 * 6);

    d->_faceVBO.release();
}

bool Cube::checkPick
(
    const QVector3D& linePnt,
    const QVector3D& lineVec,
    QVector<int>& isectDirs,
    QVector<QVector3D>& isectPnts
)
{
    isectDirs.clear();
    isectPnts.clear();

    for (int i = 0; i < 6; i++)
    {
        QVector3D isectPnt;
        if (isectLine2Face(linePnt, lineVec, d->_face[i], rectVertex[0], rectVertex[2], isectPnt))
        {
            isectDirs.push_back(i);
            isectPnts.push_back(isectPnt);
        }
    }

    return isectDirs.size() > 0;
}

QVector3D Cube::getOrig()
{
    return d->_orig;
}

QMatrix4x4 Cube::getFace(int dir)
{
    return d->_face[dir];
}

void Cube::setModel(const QMatrix4x4& model)
{
    d->_model = model;
}

void Cube::setXform(const QMatrix4x4& Xform)
{
    for (auto& face : d->_face)
    {
        face = Xform * face;
    }

    d->_orig = (Xform * QVector4D(d->_orig, 1.0f)).toVector3D();

    d->_faceVBO.destroy();
}
