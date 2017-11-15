#pragma once

#include <QtCore/QRect>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtCore/QVector>
#include <QtGui/QMatrix4x4>
#include <QtGui/QMouseEvent>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QVector3D>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QToolBar>

#include <functional>

typedef std::function<void(void)> Transaction;

#define ZERO (1.0e-6f)

enum DIR
{
    D_FRONT = 0,
    D_BACK,
    D_LEFT,
    D_RIGHT,
    D_UP,
    D_DOWN,
};
