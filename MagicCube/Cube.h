#pragma once

class Cube
{
public:
    Cube();
    ~Cube();

public:
    void init(QVector3D& orig);
    void getVboData(QVector<float>& data);
    QVector3D getOrig();
    void setXform(const QMatrix4x4& model);

private:
    friend class CubeImpl;
    CubeImpl *d;
};