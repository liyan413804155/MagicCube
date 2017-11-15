#include "stdafx.h"

#include "CentralWidget.h"
#include "World.h"

class CentralWidgetImpl
{
public:
    CentralWidgetImpl()
    {
        _world = new World();
        _width = 0;
        _height = 0;
        _iDraggingView = 0;
        _btn = Qt::NoButton;

        _childView.resize(5);

        QMatrix4x4 m;

        /* main view */
        _childView[0]._xform = m;

        /* front view */
        _childView[1]._xform = m;

        /* top view */
        m.setToIdentity();
        m.rotate(90.0f, 1.0f, 0.0f, 0.0f);
        _childView[2]._xform = m;

        /* left view */
        m.setToIdentity();
        m.rotate(90.0f, 0.0f, 1.0f, 0.0f);
        _childView[3]._xform = m;

        /* side view */
        m.setToIdentity();
        m.rotate(45.0f, 1.0f, 1.0f, 1.0f);
        _childView[4]._xform = m;
    }
    ~CentralWidgetImpl()
    {
        delete _world;
    }

public:
    int getChildView(const QPoint& pnt)
    {
        for (int i = 0; i < _childView.size(); i++)
        {
            if (_childView[i]._viewport.contains(pnt))
            {
                return i;
            }
        }
        return 0;
    }

public:
    World *_world;  /* model world */

    int _width;     /* scene width */
    int _height;    /* scene height */

    QVector<ViewInfo> _childView;  /* child view information  */

    int _iDraggingView;     /* current dragging view */

    Qt::MouseButton _btn;   /* current button */
};

CentralWidget::CentralWidget(QWidget *parent /* = Q_NULLPTR */)
    : QOpenGLWidget(parent)
{
    d = new CentralWidgetImpl;

    connect(d->_world, &World::sendCmd, this, &CentralWidget::sendCmd);

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

    d->_world->init();

}

void CentralWidget::resizeGL(int w, int h)
{
    d->_width = w;
    d->_height = h;

    /* repeat calculate child view size */

    if (d->_width > d->_height)
    {
        int childViewWidth = d->_height / (d->_childView.size() - 1);

        d->_childView[0]._viewport = QRect(0, 0, d->_width - childViewWidth, d->_height);

        for (int i = 1; i < d->_childView.size(); i++)
        {
            d->_childView[i]._viewport = QRect(QPoint(d->_width - childViewWidth,
                d->_height - i * childViewWidth),
                QSize(childViewWidth, childViewWidth));
        }
        
    }
    else
    {
        int childViewWidth = d->_width / (d->_childView.size() - 1);

        d->_childView[0]._viewport = QRect(0, childViewWidth, d->_width, d->_height - childViewWidth);

        for (int i = 1; i < d->_childView.size(); i++)
        {
            d->_childView[i]._viewport = QRect(
                QPoint(d->_width - (d->_childView.size() - i + 1) * childViewWidth,
                0),
                QSize(childViewWidth, childViewWidth));
        }
        
    }
}

void CentralWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < d->_childView.size(); i++)
    {
        d->_world->paint(d->_childView[i]);
    }
}

void CentralWidget::mousePressEvent(QMouseEvent *e)
{
    QPoint pnt = QPoint(e->localPos().x(), d->_height - e->localPos().y());
    int iChildView = d->getChildView(pnt);

    d->_btn = e->button();

    /* [1] switch children view
     */
    if (d->_btn == Qt::LeftButton && iChildView != 0 && d->_childView[0]._xform != d->_childView[iChildView]._xform)
    {
        QMatrix4x4 oldXform = d->_childView[0]._xform;
        QMatrix4x4 newXform = d->_childView[d->_iDraggingView]._xform;

        QSharedPointer<Cmd> pCmd = QSharedPointer<Cmd>(new Cmd(
            [this, oldXform]()
        {
            d->_childView[0]._xform = oldXform;
        },
            [this, newXform]()
        {
            d->_childView[0]._xform = newXform;;
        },
            QString("Switch Child View %1").arg(d->_iDraggingView - 1)));

        emit sendCmd(pCmd);
    }

    /* [2] switch event handler
    */
    if (d->_btn == Qt::LeftButton)
    {
        d->_world->setStat(World::ROT_MODE);
    } 
    else if (d->_btn == Qt::RightButton)
    {
        d->_world->setStat(World::ROT_VIEW);
    }

    /* [3] set current dragged view
     */
    if (d->_btn == Qt::LeftButton || d->_btn == Qt::RightButton)
    {
        d->_iDraggingView = iChildView;
    }

    /* [4] event handling
     */
    d->_world->dragBegin(d->_childView[iChildView], pnt - d->_childView[iChildView]._viewport.topLeft());

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
            pnt - d->_childView[iChildView]._viewport.topLeft());
    }

    update();
    __super::mouseMoveEvent(e);
}

void CentralWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint pnt = QPoint(e->localPos().x(), d->_height - e->localPos().y());

    d->_world->dragEnd(d->_childView[d->_iDraggingView], 
        pnt - d->_childView[d->_iDraggingView]._viewport.topLeft());

    d->_btn = Qt::NoButton;

    d->_world->setStat(World::NOTHING);

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