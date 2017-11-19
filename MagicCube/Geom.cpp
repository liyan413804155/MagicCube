#include "stdafx.h"

bool isectLine2Plane
(
const QVector3D& linePnt,
const QVector3D& lineVec,
const QMatrix4x4& plane,
QVector3D &isectPnt
)
{
    QMatrix4x4 inv = plane.inverted();

    QVector3D zLinePnt = (inv * QVector4D(linePnt, 1.0f)).toVector3D();
    QVector3D zLineVec = (inv * QVector4D(lineVec, 0.0f)).toVector3D();
    QVector3D zIsecPnt;

    if (qAbs(zLinePnt.z()) < ZERO)
    {
        zIsecPnt = zLinePnt;
    }
    else if (qAbs(zLineVec.z()) < ZERO)
    {
        return false;
    }
    else
    {
        zIsecPnt = zLinePnt - zLinePnt.z() / zLineVec.z() * zLineVec;
    }

    isectPnt = (plane * QVector4D(zIsecPnt, 1.0f)).toVector3D();

    return true;
}

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
    QVector3D zIsecPnt;

    if (!isectLine2Plane(linePnt, lineVec, face, isectPnt))
        return false;

    zIsecPnt = (face.inverted() * QVector4D(isectPnt, 1.0f)).toVector3D();

    if (zIsecPnt.x() < faceMin.x() || zIsecPnt.x() > faceMax.x())
        return false;

    if (zIsecPnt.y() < faceMin.y() || zIsecPnt.y() > faceMax.y())
        return false;

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

void getRay
(
const QMatrix4x4& projView,
const QVector2D& locPnt,
QVector3D& wldPnt,
QVector3D& wldVec
)
{
    QMatrix4x4 inv = projView.inverted();
    wldPnt = (inv * QVector4D(locPnt, 0.0f, 1.0f)).toVector3D();  /* ray point */
    wldVec = inv.column(2).toVector3D();                          /* ray direct */
}