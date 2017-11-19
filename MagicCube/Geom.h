#pragma once

#define ZERO (1.0e-6f)

template <>
class std::less<QVector3D>
{
public:
    inline bool operator()(const QVector3D& a, const QVector3D& b) const
    {
        if (a.x() < b.x() - ZERO)
            return false;
        else if (a.x() > b.x() + ZERO)
            return true;
        else if (a.y() < b.y() - ZERO)
            return false;
        else if (a.y() > b.y() + ZERO)
            return true;
        else if (a.z() < b.z() - ZERO)
            return false;
        else if (a.z() > b.z() + ZERO)
            return true;
        return false;
    }
};

enum DIR
{
    D_FRONT = 0,
    D_BACK,
    D_LEFT,
    D_RIGHT,
    D_TOP,
    D_BOTTOM,
    D_SIDE,
};

bool isectLine2Plane(const QVector3D& linePnt, const QVector3D& lineVec, const QMatrix4x4& plane, QVector3D &isectPnt);

bool isectLine2Face(const QVector3D& linePnt, const QVector3D& lineVec, const QMatrix4x4& face, const QVector2D& faceMin, const QVector2D& faceMax, QVector3D &isectPnt);

QMatrix4x4 getPlane(int dir);

void getRay(const QMatrix4x4& projView, const QVector2D& locPnt, QVector3D& wldPnt, QVector3D& wldVec);