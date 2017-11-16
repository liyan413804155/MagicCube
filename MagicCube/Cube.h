#pragma once

extern float cubeSize;

class Cube : public QOpenGLFunctions
{
public:
    Cube(QVector3D& orig);
    ~Cube();

public:
    void init();
    void drawFace(QOpenGLShaderProgram &faceShader);
    void drawEdge(QOpenGLShaderProgram &edgeShader);

public:
    bool checkPick(const QVector3D& linePnt, const QVector3D& lineVec, QVector<int>& isectDirs, QVector<QVector3D>& isectPnts);
    QVector3D getOrig();
    QMatrix4x4 getFace(int dir);

    void setModel(const QMatrix4x4& model);
    void setXform(const QMatrix4x4& Xform);

private:
    friend class CubeImpl;
    CubeImpl *d;
};