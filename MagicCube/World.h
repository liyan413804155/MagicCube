#pragma once

struct ViewInfo
{
    QRect  _viewport;
    QMatrix4x4 _xform;
};

class World : public QObject, QOpenGLFunctions
{
    Q_OBJECT
public:
    World();
    ~World();

public:
    void init();
    void reinit();

public:
    void dragBegin(const ViewInfo& view, const QPoint& pnt, Qt::MouseButton btn);
    void dragging(const ViewInfo& view, const QPoint& pnt, Qt::MouseButton btn);
    void dragEnd(const ViewInfo& view, const QPoint& pnt, Qt::MouseButton btn);

public:
    void zoom(int iZoomIn);

public:
    void paint(const ViewInfo& view);

public:
    void setView(const QMatrix4x4& view, const QString reason);

signals:
    void sendCmd(QSharedPointer<Cmd> pCmd);

private:
    friend class WorldImpl;
    WorldImpl * d;
};