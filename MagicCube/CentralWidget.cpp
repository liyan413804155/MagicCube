#include "stdafx.h"

#include "CentralWidget.h"
#include "World.h"

class CentralWidgetImpl
{
public:
    CentralWidgetImpl()
    {
        _world = new World();
        _childView.resize(5);

    }
    ~CentralWidgetImpl()
    {
        delete _world;
    }

public:
    void init(bool bFirst)
    {
        if (bFirst)
        {
            glCullFace(GL_BACK);
            glPolygonOffset(-1.0f, -1.0f);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_POLYGON_SMOOTH);
        }

        _width = 0;
        _height = 0;
        _iDraggingView = 0;
        _iMainView = 1;
        _btn = Qt::NoButton;

        _childView[0]._xform = getPlane(D_FRONT);   /* main view */
        _childView[1]._xform = getPlane(D_FRONT);   /* front view */
        _childView[2]._xform = getPlane(D_TOP);     /* top view */
        _childView[3]._xform = getPlane(D_LEFT);    /* left view */
        _childView[4]._xform = getPlane(D_SIDE);    /* side view */

        _world->init(bFirst);
    }

    int getChildView(const QPoint& devPnt)
    {
        for (int i = 0; i < _childView.size(); i++)
        {
            if (_childView[i]._viewport.contains(devPnt))
            {
                return i;
            }
        }
        return 0;
    }

    void drawActiveViewFrame()
    {
        float x0 = ((float)_childView[_iMainView]._viewport.x) / (_width / 2) - 1.0f;
        float y0 = ((float)_childView[_iMainView]._viewport.y) / (_height / 2) - 1.0f;
        float w = ((float)_childView[_iMainView]._viewport.w) / (_width / 2);
        float h = ((float)_childView[_iMainView]._viewport.h) / (_height / 2);
        float x1 = x0 + w;
        float y1 = y0 + h;

        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1.0f, 1.0f, 0.0f);

        glBegin(GL_QUADS);
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();
    }

public:
    World *_world;  /* model world */

    int _width;     /* scene width */
    int _height;    /* scene height */

    QVector<ViewInfo> _childView;  /* child view information  */

    int _iDraggingView;     /* current dragging view */
    int _iMainView;

    Qt::MouseButton _btn;   /* current button */
};

CentralWidget::CentralWidget(QWidget *parent /* = Q_NULLPTR */)
    : QOpenGLWidget(parent)
{
    d = new CentralWidgetImpl;

    connect(d->_world, &World::sendCmd, this, &CentralWidget::sendCmd);
    connect(d->_world, &World::setCoord, this, &CentralWidget::setCoord);

    setMouseTracking(true);
}

CentralWidget::~CentralWidget()
{
    delete d->_world;
}

void CentralWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    d->init(true);
}

void CentralWidget::resizeGL(int w, int h)
{
    d->_width = w;
    d->_height = h;

    /* repeat calculate child view size */

    if (d->_width > d->_height)
    {
        int childViewWidth = d->_height / (d->_childView.size() - 1);

        d->_childView[0]._viewport = GLRect(0, 0, d->_width - childViewWidth, d->_height);

        for (int i = 1; i < d->_childView.size(); i++)
        {
            d->_childView[i]._viewport = GLRect(QPoint(d->_width - childViewWidth,
                d->_height - i * childViewWidth),
                QSize(childViewWidth, childViewWidth));
        }
        
    }
    else
    {
        int childViewWidth = d->_width / (d->_childView.size() - 1);

        d->_childView[0]._viewport = GLRect(0, childViewWidth, d->_width, d->_height - childViewWidth);

        for (int i = 1; i < d->_childView.size(); i++)
        {
            d->_childView[i]._viewport = GLRect(
                QPoint(d->_width - (d->_childView.size() - i) * childViewWidth,
                0),
                QSize(childViewWidth, childViewWidth));
        }
        
    }
}

void CentralWidget::paintGL()
{
    /* [1] draw model
     */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < d->_childView.size(); i++)
    {
        d->_world->paint(d->_childView[i]);
    }

    /* [2]
     */
    glViewport(0, 0, d->_width, d->_height);

    d->drawActiveViewFrame();
}

void CentralWidget::mousePressEvent(QMouseEvent *e)
{
    QPoint pnt = QPoint(e->localPos().x(), d->_height - e->localPos().y());
    int iChildView = d->getChildView(pnt);

    d->_btn = e->button();

    /* [1] switch children view
     */
    if (d->_btn == Qt::LeftButton && iChildView != 0 && d->_iMainView != iChildView)
    {
        int oldView = d->_iMainView;

        QSharedPointer<Cmd> pCmd = QSharedPointer<Cmd>(new Cmd(
            [this, oldView]()
        {
            d->_iMainView = oldView;
            d->_childView[0]._xform = d->_childView[d->_iMainView]._xform;
        },
            [this, iChildView]()
        {
            d->_iMainView = iChildView;
            d->_childView[0]._xform = d->_childView[d->_iMainView]._xform;
        },
            QString("Switch Child View %1").arg(d->_iDraggingView - 1)));

        emit sendCmd(pCmd);
    }


    /* [2] set current dragged view
     */
    if (d->_btn == Qt::LeftButton || d->_btn == Qt::RightButton)
    {
        d->_iDraggingView = iChildView;
    }

    /* [3] event handling
     */
    d->_world->dragBegin(d->_childView[iChildView], pnt - d->_childView[iChildView]._viewport.source(), d->_btn);

    update();
    __super::mousePressEvent(e);
}

void CentralWidget::mouseMoveEvent(QMouseEvent *e)
{
    QPoint pnt = QPoint(e->localPos().x(), d->_height - e->localPos().y());

    int iChildView = d->_btn == Qt::NoButton ? d->getChildView(pnt) : d->_iDraggingView;

    if (d->_childView[iChildView]._viewport.contains(pnt))
    {
        d->_world->dragging(d->_childView[iChildView],
            pnt - d->_childView[iChildView]._viewport.source(),
            d->_btn);
    }

    update();
    __super::mouseMoveEvent(e);
}

void CentralWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint pnt = QPoint(e->localPos().x(), d->_height - e->localPos().y());

    d->_world->dragEnd(d->_childView[d->_iDraggingView], 
        pnt - d->_childView[d->_iDraggingView]._viewport.source(), d->_btn);

    d->_btn = Qt::NoButton;

    update();
    __super::mouseReleaseEvent(e);
}

void CentralWidget::wheelEvent(QWheelEvent *e)
{
    d->_world->zoom(e->delta() > 0);

    update();
    __super::wheelEvent(e);
}

void CentralWidget::setAlignView(const QMatrix4x4& alignView, const QString& dir)
{
    d->_world->setView(alignView, dir);
}

void CentralWidget::setCoord(const QVector3D& coord)
{
    QString txt = QString(" (%1, %2, %3)")
        .arg(coord.x())
        .arg(coord.y())
        .arg(coord.z());

    emit setMsg(txt);
}
