#pragma once

#include <QtCore/QRect>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtGui/QMatrix4x4>
#include <QtGui/QMouseEvent>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QPainter>
#include <QtGui/QVector3D>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>

#include <functional>
#include <map>
#include <set>

#define DECL_PRI(classname) friend class classname##Impl;classname##Impl *d;

struct GLRect
{
    GLRect()
        : GLRect(0, 0, 0, 0)
    {

    }

    GLRect(int _x, int _y, int _w, int _h)
    {
        x = _x;
        y = _y;
        w = _w;
        h = _h;
    }

    GLRect(const QPoint& pnt, const QSize& size)
        : GLRect(pnt.x(), pnt.y(), size.width(), size.height())
    {

    }

    bool contains(const QPoint& pnt)
    {
        if (pnt.x() < x)
            return false;
        if (pnt.y() < y)
            return false;
        if (pnt.x() > x + w - 1)
            return false;
        if (pnt.y() > y + h - 1)
            return false;
        return true;
    }

    QPoint source()const
    {
        return QPoint(x, y);
    }

    QRect toRect(const QSize& size)const
    {
        return QRect(x, size.height() - y - h, w, h);
    }

    int x, y, w, h;
};

#include "Cmd.h"
#include "Geom.h"