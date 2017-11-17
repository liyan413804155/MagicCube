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
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUseProgram(0);

        float white = 1.0f;
        float gray = 0.5f;
        float gradient = (white + gray) / 2;

        glMatrixMode(GL_MODELVIEW);
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

    QMatrix4x4 getProjView(const ViewInfo& viewInfo)
    {
        /* [1] calculate projection matrix
         */
        const float ZDEPTHMAX = 200.0f;

        float r = ((float)(viewInfo._viewport.width())) / (viewInfo._viewport.height());
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

        /* [2] get view matrix
         */
        QMatrix4x4 view(viewInfo._xform * _view);

        return projection * view;
    }

    QVector2D dev2Loc(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        return QVector2D(((float)pnt.x())/(viewInfo._viewport.width() / 2.0f) - 1.0f,
            ((float)pnt.y()) / (viewInfo._viewport.height() / 2.0f) - 1.0f);
    }

    void rotViewBegin(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        _pntSave = pnt;
        _viewSave = _view;
    }

    void rotViewDoing(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        /* [1] length to angle
        */
        float r = 180.0f * (_extent / modelLevel) / min(viewInfo._viewport.width(), viewInfo._viewport.width());
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

    void rotViewEnd(const ViewInfo& viewInfo, const QPoint& pnt)
    {
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

        glCullFace(GL_BACK);

        glPolygonOffset(-1.0f, -1.0f);
        glEnable(GL_POLYGON_OFFSET_LINE);

        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POLYGON_SMOOTH);

        connect(d->_model, &Model::sendCmd, this, &World::sendCmd);
    }

    d->init(bFirst);
}


void World::dragBegin(const ViewInfo& viewInfo, const QPoint& pnt, Qt::MouseButton btn)
{
    if (btn == Qt::LeftButton)
    {
        d->_model->dragBegin(d->getProjView(viewInfo), d->dev2Loc(viewInfo, pnt));
    }
    else if (btn == Qt::RightButton)
    {
        d->rotViewBegin(viewInfo, pnt);
    }

}

void World::dragging(const ViewInfo& viewInfo, const QPoint& pnt, Qt::MouseButton btn)
{
    if (btn == Qt::LeftButton)
    {
        d->_model->dragging(d->getProjView(viewInfo), d->dev2Loc(viewInfo, pnt));
    }
    else if (btn == Qt::RightButton)
    {
        d->rotViewDoing(viewInfo, pnt);
    }
    else if (btn == Qt::NoButton)
    {
        d->_model->pick(d->getProjView(viewInfo), d->dev2Loc(viewInfo, pnt));
    }
}

void World::dragEnd(const ViewInfo& viewInfo, const QPoint& pnt, Qt::MouseButton btn)
{
    if (btn == Qt::LeftButton)
    {
        d->_model->dragEnd(d->getProjView(viewInfo), d->dev2Loc(viewInfo, pnt));
    }
    else if (btn == Qt::RightButton)
    {
        d->rotViewEnd(viewInfo, pnt);
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
    glViewport(viewInfo._viewport.x(), viewInfo._viewport.y(), viewInfo._viewport.width(), viewInfo._viewport.height());

    /* [2] render background
    */
    d->paintBackground(viewInfo);

    /* [3] set projection matrix
     */
    QMatrix4x4 projView = d->getProjView(viewInfo);

    d->_model->draw(projView);

    /* [6] render foreground
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
