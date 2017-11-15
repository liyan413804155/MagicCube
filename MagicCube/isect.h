#pragma once

bool isectLine2Box
(
const QVector3D& linePnt,
const QVector3D& LineVec,
const QVector3D& boxMin,
const QVector3D& boxMax,
int& nearFaces,
QVector3D& nearPnts
);