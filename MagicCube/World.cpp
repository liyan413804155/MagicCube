#include "stdafx.h"

#include "World.h"
#include "Model.h"

using namespace std;


class WorldImpl : public QOpenGLFunctions
{

public:
    WorldImpl(World* self)
    {
        q = self;

        _model = new Model;
    }

    ~WorldImpl()
    {
        delete _model;
    }

    void init(bool bFirst)
    {
        if (bFirst)
        {
            initializeOpenGLFunctions();
        }

        _extent = modelLevel * 1.2f;

        _view.rotate(45.0f, 1.0f, 1.0f, 1.0f);

        _model->init(bFirst);
    }

    void paintForeground(const ViewInfo& viewInfo)
    {

    }

    void paintBackground(const ViewInfo& viewInfo)
    {
        /* [1] paint background
        */
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUseProgram(0);

        const float white = 1.0f;
        const float gray = 0.5f;
        const float gradient = (white + gray) / 2.0f;

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glBegin(GL_QUADS);
            glColor3f(white, white, white);
            glVertex2f(-1.0f, -1.0f);
            glColor3f(gradient, gradient, gradient);
            glVertex2f(1.0f, -1.0f);
            glColor3f(gray, gray, gray);
            glVertex2f(1.0f, 1.0f);        
            glColor3f(gradient, gradient, gradient);
            glVertex2f(-1.0f, 1.0f);
        glEnd();
    }

    QMatrix4x4 getProjMatrix(const ViewInfo& viewInfo)
    {
        /* [1] calculate projection matrix
         */
        const float ZDEPTHMAX = 200.0f;

        float r = ((float)(viewInfo._viewport.w)) / (viewInfo._viewport.h);
        float x = 0.0f, y = 0.0f;
        if (r > 1.0f)
        {
            x = _extent * r;
            y = _extent;
        }
        else
        {
            x = _extent;
            y = _extent / r;
        }

        QMatrix4x4 projection;
        projection.ortho(-x, x, -y, y, -ZDEPTHMAX, ZDEPTHMAX);

        return projection;
    }

    QMatrix4x4 getViewMatrix(const ViewInfo& viewInfo)
    {
        return viewInfo._xform * _view;
    }

    QMatrix4x4 getProjView(const ViewInfo& viewInfo)
    {
        return getProjMatrix(viewInfo) * getViewMatrix(viewInfo);
    }

    QVector2D dev2Loc(const ViewInfo& viewInfo, const QPoint& devPnt)
    {
        return QVector2D(((float)devPnt.x())/(viewInfo._viewport.w / 2.0f) - 1.0f,
            ((float)devPnt.y()) / (viewInfo._viewport.h / 2.0f) - 1.0f);
    }

    void dragBegin(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        _pntSave = pnt;
        _viewSave = _view;
    }

    void dragging(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        /* [1] length to angle
        */
        float r = 180.0f * (_extent / modelLevel) / min(viewInfo._viewport.w, viewInfo._viewport.w);
        float yAngle = (pnt.x() - _pntSave.x()) * r;
        float xAngle = -(pnt.y() - _pntSave.y()) * r;

        /* [2] projection view transform
        */
        _view = _viewSave;
        QMatrix4x4 projView = getProjView(viewInfo);
        projView.rotate(yAngle, projView(1, 0), projView(1, 1), projView(1, 2));
        projView.rotate(xAngle, projView(0, 0), projView(0, 1), projView(0, 2));

        /* [3] update original view matrix
        */
        _view = QMatrix4x4();
        QMatrix4x4 inv = getProjView(viewInfo).inverted();
        _view = inv * projView;
    }

    void dragEnd(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        /* [1] update view matrix */
        if (qAbs(pnt.x() - _pntSave.x()) > ZERO
            || qAbs(pnt.y() - _pntSave.y()) > ZERO)
        {
            QMatrix4x4 viewNew = _view;

            _view = _viewSave;

            q->setView(viewNew, QString("Rotate View"));
        }
        else
        {
            _view = _viewSave;
        }
    }

public:
    World *q;

    Model *_model;      /* model data */

    float _extent;      /* scene size */

    QMatrix4x4 _view;   /* original view transform */

    QMatrix4x4 _viewSave;   /* save original view transform when dragging */
    QPoint _pntSave;        /* save original point when dragging */
};

World::World()
{
    d = new WorldImpl(this);

    connect(d->_model, &Model::setCoord, this, &World::setCoord);
}

World::~World()
{
    delete d;
}

void World::init(bool bFirst)
{
    /* [1] initialize OpenGL Functions
     */
    if (bFirst)
    {
        initializeOpenGLFunctions();

        connect(d->_model, &Model::sendCmd, this, &World::sendCmd);
    }

    d->init(bFirst);
}


void World::dragBegin(const ViewInfo& viewInfo, const QPoint& pnt, Qt::MouseButton btn)
{
    QMatrix4x4 projView = d->getProjView(viewInfo);
    QVector2D locPnt = d->dev2Loc(viewInfo, pnt);
    QVector3D wldPnt;
    QVector3D wldVec;

    getRay(projView, locPnt, wldPnt, wldVec);

    if (btn == Qt::LeftButton)
    {
        d->_model->dragBegin(projView, wldPnt, wldVec);
    }
    else if (btn == Qt::RightButton)
    {
        d->dragBegin(viewInfo, pnt);
    }
}

void World::dragging(const ViewInfo& viewInfo, const QPoint& pnt, Qt::MouseButton btn)
{
    QMatrix4x4 projView = d->getProjView(viewInfo);
    QVector2D locPnt = d->dev2Loc(viewInfo, pnt);
    QVector3D wldPnt;
    QVector3D wldVec;

    getRay(projView, locPnt, wldPnt, wldVec);
    emit setCoord(wldPnt);

    if (btn == Qt::LeftButton)
    {
        d->_model->dragging(projView, wldPnt, wldVec);
    }
    else if (btn == Qt::RightButton)
    {
        d->dragging(viewInfo, pnt);
    }
    else if (btn == Qt::NoButton)
    {
        d->_model->pick(projView, wldPnt, wldVec);
    }
}

void World::dragEnd(const ViewInfo& viewInfo, const QPoint& pnt, Qt::MouseButton btn)
{
    QMatrix4x4 projView = d->getProjView(viewInfo);
    QVector2D locPnt = d->dev2Loc(viewInfo, pnt);
    QVector3D wldPnt;
    QVector3D wldVec;

    getRay(projView, locPnt, wldPnt, wldVec);

    if (btn == Qt::LeftButton)
    {
        d->_model->dragEnd(projView, wldPnt, wldVec);
    }
    else if (btn == Qt::RightButton)
    {
        d->dragEnd(viewInfo, pnt);
    }
}

void World::zoom(int iZoomIn)
{
    const float zoomMax = 20.0f;

    float extent = d->_extent;
    float extentSave = d->_extent;

    /* [1] update "extent"
     */
    if (iZoomIn)
    {
        extent *= 1.1f;
        extent = extent > zoomMax ? zoomMax : extent;
    }
    else
    {
        extent /= 1.1f;
        extent = extent < 1 / zoomMax ? 1 / zoomMax : extent;
    }

    /* [2] send command
     */
    if (qAbs(d->_extent - extent) > ZERO)
    {
        QSharedPointer<Cmd> pCmd = QSharedPointer<Cmd>(new Cmd(
            [this, extentSave]()
        {
            d->_extent = extentSave;
        }, 
            [this, extent]()
        {
            d->_extent = extent;
        },
        QString("Zoom In/Out")));

        emit sendCmd(pCmd);
    }
    
}


void World::paint(const ViewInfo& viewInfo)
{
    /* [1] set viewport
     */
    glViewport(viewInfo._viewport.x, viewInfo._viewport.y, viewInfo._viewport.w, viewInfo._viewport.h);

    /* [2] render background
    */
    d->paintBackground(viewInfo);

    /* [3] render model
     */

    d->_model->draw(d->getProjMatrix(viewInfo), d->getViewMatrix(viewInfo));

    /* [4] render foreground
    */
    d->paintForeground(viewInfo);
}

void World::setView(const QMatrix4x4& view, const QString reason)
{
    QMatrix4x4 viewOld = d->_view;

    QSharedPointer<Cmd> pCmd = QSharedPointer<Cmd>(new Cmd(
        [this, viewOld]()
    {
        d->_view = viewOld;
    },
        [this, view]()
    {
        d->_view = view;
    },
    reason));

    emit sendCmd(pCmd);
}
