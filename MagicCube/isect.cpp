#include "stdafx.h"
#include "isect.h"

bool checkPntInBox
(
const QVector3D& pnt,
const QVector3D& boxMin,
const QVector3D& boxMax
)
{
    if ((pnt.x() < boxMax.x() + ZERO)
        && (pnt.x() > boxMin.x() - ZERO)
        && (pnt.y() < boxMax.y() + ZERO)
        && (pnt.y() > boxMin.y() - ZERO)
        && (pnt.z() < boxMax.z() + ZERO)
        && (pnt.z() > boxMin.z() - ZERO))
        return true;
    return false;
}

bool isectLine2Plane
(
const QVector3D& linePnt,
const QVector3D& LineVec,
const QVector3D& plnPnt,
const QVector3D& plnVec,
QVector3D &isectPnt
)
{
    float d = QVector3D::dotProduct(LineVec, plnVec);
    if (qAbs(d) < ZERO)
        return false;

    float param = QVector3D::dotProduct(plnPnt - linePnt, plnVec) / d;

    isectPnt = linePnt + param * LineVec;

    return true;
}

bool isectLine2Face
(
const QVector3D& linePnt,
const QVector3D& LineVec,
const QVector3D& facePnt,
const QVector3D& faceVec,
const QVector3D& faceMin,
const QVector3D& faceMax,
QVector3D &isectPnt
)
{
    if (!isectLine2Plane(linePnt, LineVec, facePnt, faceVec, isectPnt))
        return false;

    if (!checkPntInBox(isectPnt, faceMin, faceMax))
        return false;

    return true;
}


bool isectLine2Box
(
const QVector3D& linePnt,
const QVector3D& LineVec,
const QVector3D& boxMin,
const QVector3D& boxMax,
int& nearFaces,
QVector3D& nearPnts
)
{
    QVector<int> isectFaces;
    QVector<QVector3D> isectPnts;
    QVector3D isectPnt;

    if (isectLine2Face(linePnt, LineVec, boxMax, QVector3D(0.0f, 0.0f, 1.0f), QVector3D(boxMin.x(), boxMin.y(), boxMax.z()), boxMax, isectPnt))
    {
        isectFaces.push_back(D_FRONT);
        isectPnts.push_back(isectPnt);
    }

    if (isectLine2Face(linePnt, LineVec, boxMin, QVector3D(0.0f, 0.0f, 1.0f), boxMin, QVector3D(boxMax.x(), boxMax.y(), boxMin.z()), isectPnt))
    {
        isectFaces.push_back(D_BACK);
        isectPnts.push_back(isectPnt);
    }

    if (isectLine2Face(linePnt, LineVec, boxMin, QVector3D(1.0f, 0.0f, 0.0f), boxMin, QVector3D(boxMin.x(), boxMax.y(), boxMax.z()), isectPnt))
    {
        isectFaces.push_back(D_LEFT);
        isectPnts.push_back(isectPnt);
    }

    if (isectLine2Face(linePnt, LineVec, boxMax, QVector3D(1.0f, 0.0f, 0.0f), QVector3D(boxMax.x(), boxMin.y(), boxMin.z()), boxMax, isectPnt))
    {
        isectFaces.push_back(D_RIGHT);
        isectPnts.push_back(isectPnt);
    }

    if (isectLine2Face(linePnt, LineVec, boxMax, QVector3D(0.0f, 1.0f, 0.0f), QVector3D(boxMin.x(), boxMax.y(), boxMin.z()), boxMax, isectPnt))
    {
        isectFaces.push_back(D_UP);
        isectPnts.push_back(isectPnt);
    }

    if (isectLine2Face(linePnt, LineVec, boxMax, QVector3D(0.0f, 1.0f, 0.0f), boxMin, QVector3D(boxMax.x(), boxMin.y(), boxMax.z()), isectPnt))
    {
        isectFaces.push_back(D_UP);
        isectPnts.push_back(isectPnt);
    }

    Q_ASSERT(isectFaces.size() <= 2);

    if (isectFaces.size() == 1)
    {
        nearFaces = isectFaces[0];
        nearPnts = isectPnts[0];
    }
    else if (isectFaces.size() == 2)
    {
        if ((isectPnts[0].x() - isectPnts[1].x()) / LineVec.x() < 0.0f)
        {
            nearFaces = isectFaces[1];
            nearPnts = isectPnts[1];
        }
        else
        {
            nearFaces = isectFaces[0];
            nearPnts = isectPnts[0];
        }
    }

    return isectFaces.size() > 0;
}