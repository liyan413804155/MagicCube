#include "Cube.h"

extern const int modelLevel;

class Model : public QObject, public QOpenGLFunctions
{
    Q_OBJECT
public:
    Model();
    ~Model();

public:
    void init();
    void reinit();

public:
    bool pick(const QMatrix4x4& projView, const QVector2D& locPnt);

public:
    bool dragBegin(const QMatrix4x4& projView, const QVector2D& locPnt);
    void dragging(const QMatrix4x4& projView, const QVector2D& locPnt);
    void dragEnd(const QMatrix4x4& projView, const QVector2D& locPnt);

public:
    void draw(const QMatrix4x4& projView);

signals:
    void sendCmd(QSharedPointer<Cmd> pCmd);

private:
    DECL_PRI(Model)
};