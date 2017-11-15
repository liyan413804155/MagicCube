#pragma once

#include "Cmd.h"

struct ViewInfo
{
    QRect  _viewport;
    QMatrix4x4 _xform;
};

class World : public QObject, QOpenGLFunctions
{
    Q_OBJECT

public:
    enum STAT
    {
        NOTHING,
        ROT_VIEW,
        ROT_MODE
    };
public:
    World();
    ~World();

public:
    void init();

    void dragBegin(const ViewInfo& view, const QPoint& pnt);
    void dragging(const ViewInfo& view, const QPoint& pnt);
    void dragEnd(const ViewInfo& view, const QPoint& pnt);

    void zoom(int iZoomIn);

    void paint(const ViewInfo& view);

public:
    void setView(const QMatrix4x4& view, const QString reason);
    void setStat(World::STAT s);

signals:
    void sendCmd(QSharedPointer<Cmd> pCmd);

private:
    friend class WorldImpl;
    WorldImpl * d;
};