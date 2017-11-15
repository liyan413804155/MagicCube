#pragma once

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
    bool checkPick(const QVector3D& line_pnt, const QVector3D& line_vec, int& nearFaces, QVector3D& nearPnts);

private:
    friend class CubeImpl;
    CubeImpl *d;
};