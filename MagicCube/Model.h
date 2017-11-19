

extern const int modelLevel;

class Model : public QObject, public QOpenGLFunctions
{
    Q_OBJECT
public:
    Model();
    ~Model();

public:
    void init(bool bFirst);

public:
    bool pick(const QMatrix4x4& projView, const QVector3D& wldPnt,const QVector3D& wldVec);

public:
    bool dragBegin(const QMatrix4x4& projView, const QVector3D& wldPnt, const QVector3D& wldVec);
    void dragging(const QMatrix4x4& projView, const QVector3D& wldPnt, const QVector3D& wldVec);
    void dragEnd(const QMatrix4x4& projView, const QVector3D& wldPnt, const QVector3D& wldVec);

public:
    void draw(const QMatrix4x4& projView);

signals:
    void sendCmd(QSharedPointer<Cmd> pCmd);
    void setCoord(const QVector3D& coord);

private:
    DECL_PRI(Model)
};