#pragma once

#include "Cmd.h"

class CentralWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
public:
    CentralWidget(QWidget *parent = Q_NULLPTR);
    ~CentralWidget();

protected:
    virtual void initializeGL()override;
    virtual void resizeGL(int w, int h)override;
    virtual void paintGL()override;

    virtual void mousePressEvent(QMouseEvent *e)override;
    virtual void mouseMoveEvent(QMouseEvent *e)override;
    virtual void mouseReleaseEvent(QMouseEvent *e)override;
    virtual void wheelEvent(QWheelEvent *e)override;

public:
    void setAlignView(const QMatrix4x4& alignView, const QString& dir);

signals:
    void sendCmd(QSharedPointer<Cmd> pCmd);

private:
    friend class CentralWidgetImpl;
    CentralWidgetImpl *d;
};
