#pragma once

class Cube
{
public:
    Cube();
    ~Cube();

public:
    void init(QVector3D& orig);

public:
    void getVboData(QVector<float>& data);
    QVector3D getOrig();

public:
    void setXform(const QMatrix4x4& model);

private:
    DECL_PRI(Cube);
};