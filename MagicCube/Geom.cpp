#include "stdafx.h"

bool isectLine2Face
(
    const QVector3D& linePnt,
    const QVector3D& lineVec,
    const QMatrix4x4& face,
    const QVector2D& faceMin,
    const QVector2D& faceMax,
    QVector3D &isectPnt
)
{
    QMatrix4x4 faceInv = face.inverted();

    QVector3D zLinePnt = (faceInv * QVector4D(linePnt, 1.0f)).toVector3D();
    QVector3D zLineVec = (faceInv * QVector4D(lineVec, 0.0f)).toVector3D();
    QVector3D zIsecPnt;

    if (qAbs(zLinePnt.z()) < ZERO)
    {
        zIsecPnt = zLinePnt;
    }
    else
    {
        zIsecPnt = zLinePnt - zLinePnt.z() / zLineVec.z() * zLineVec;
    }

    if (zIsecPnt.x() < faceMin.x() || zIsecPnt.x() > faceMax.x())
        return false;

    if (zIsecPnt.y() < faceMin.y() || zIsecPnt.y() > faceMax.y())
        return false;

    isectPnt = (face * QVector4D(zIsecPnt, 1.0f)).toVector3D();

    return true;
}

QMatrix4x4 getPlane(int dir)
{
    static QVector<QMatrix4x4> planes(7);
    static bool bInit = false;
    if (bInit)
    {
        return planes[dir];
    }

    QMatrix4x4 m;

    planes[D_FRONT] = m;

    m.rotate(180.0f, 1.0f, 0.0f, 0.0f);
    planes[D_BACK] = m;

    m.setToIdentity();
    m.rotate(90.0f, 0.0f, 1.0f, 0.0f);
    planes[D_LEFT] = m;

    m.setToIdentity();
    m.rotate(90.0f, 0.0f, -1.0f, 0.0f);
    planes[D_RIGHT] = m;

    m.setToIdentity();
    m.rotate(90.0f, 1.0f, 0.0f, 0.0f);
    planes[D_TOP] = m;

    m.setToIdentity();
    m.rotate(90.0f, -1.0f, 0.0f, 0.0f);
    planes[D_BOTTOM] = m;

    m.setToIdentity();
    m.rotate(45.0f, 1.0f, 1.0f, 1.0f);
    planes[D_SIDE] = m;

    bInit = true;

    return planes[dir];
}
