#include "stdafx.h"

#include "World.h"
#include "Cube.h"
#include "isect.h"

static float ZDEPTHMAX = 200.0f;
static const int LEVEL = 4;

QVector3D edgeNormalColor(0.0f, 0.0f, 0.0f);
QVector3D edgePreHighlightColor(1.0f, 1.0f, 0.0f);
QVector3D edgeHighlightColor(1.0f, 0.74f, 0.0f);

class WorldImpl : public QOpenGLFunctions
{
public:
    typedef void (WorldImpl::*dragEvent)(const ViewInfo& viewInfo, const QPoint& pnt);

public:
    WorldImpl(World* self)
    {
        q = self;

        _extent = LEVEL * 1.2f;

        _view.rotate(45.0f, 1.0f, 1.0f, 1.0f);

        for (int i = 0; i < LEVEL * LEVEL * LEVEL; i++)
        {
            _cube[i] = QSharedPointer<Cube>(new Cube(QVector3D(
                i / (LEVEL*LEVEL) - (LEVEL - 1) / 2.0f,
                (i / LEVEL) % LEVEL - (LEVEL - 1) / 2.0f,
                i % LEVEL - (LEVEL - 1) / 2.0f)));
        }

        _picked = false;

    }

    ~WorldImpl()
    {

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
        projection.ortho(-x, x, -y, y, ZDEPTHMAX, -ZDEPTHMAX);

        QMatrix4x4 view(viewInfo._xform * _view);

        return projection * view;
    }

    bool checkPick(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        QVector3D locPnt = dev2Loc(viewInfo, pnt);
        QMatrix4x4 xform = getProjView(viewInfo);
        QMatrix4x4 xformRev = xform.inverted();
        QVector3D wldPnt = xformRev * locPnt;
        QVector3D wldVec = xformRev.column(2).toVector3D(); 

        QVector<int> pickCubes;
        QVector<QVector3D> pickPnts;
        QVector<int> pickDirs;

        _picked = false;

        for (int i = 0; i < LEVEL * LEVEL * LEVEL; i++)
        {
            QVector3D pickPnt;
            int pickDir;
            if (_cube[i]->checkPick(wldPnt, wldVec, pickDir, pickPnt))
            {
                pickCubes.push_back(i);
                pickPnts.push_back(pickPnt);
                pickDirs.push_back(pickDir);
            }
        }

        if (pickPnts.size() == 0)
            return false;

        float zMin = (xform * pickPnts[0]).z();
        int pickCube = 0;

        for (int i = 1; i < pickPnts.size(); i++)
        {
            float z = (xform * pickPnts[i]).z();
            if (z < zMin)
            {
                pickCube = i;
                zMin = z;
            }
        }

        _pickCube = pickCubes[pickCube];
        _pickDir = pickDirs[pickCube];
        _picked = true;

        return true;
    }

    QVector3D dev2Loc(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        return QVector3D(((float)pnt.x())/(viewInfo._viewport.width() / 2.0f) - 1.0f,
            ((float)pnt.y()) / (viewInfo._viewport.height() / 2.0f) - 1.0f,
            0.0f);
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
        float r = 180.0f * (_extent / LEVEL) /
            (viewInfo._viewport.width()  > viewInfo._viewport.height() ?
            viewInfo._viewport.height() : viewInfo._viewport.width());
        float yAngle = -(pnt.x() - _pntSave.x()) * r;
        float xAngle = (pnt.y() - _pntSave.y()) * r;

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

    void rotModeBegin(const ViewInfo& viewInfo, const QPoint& pnt)
    {
//         if (checkPick(viewInfo, pnt))
//         {
//             _picked = true;
//             _edgeColor = edgeHighlightColor;
//         }
//         else
//         {
//             _picked = false;
//             _edgeColor = edgeNormalColor;
//         }
    }

    void rotModeDoing(const ViewInfo& viewInfo, const QPoint& pnt)
    {

    }

    void rotModeEnd(const ViewInfo& viewInfo, const QPoint& pnt)
    {

    }

    void NothingBegin(const ViewInfo& viewInfo, const QPoint& pnt)
    {

    }

    void NothingDoing(const ViewInfo& viewInfo, const QPoint& pnt)
    {
//         if (checkPick(viewInfo, pnt))
//         {
//             _edgeColor = edgePreHighlightColor;
//         }
//         else
//         {
//             _edgeColor = edgeNormalColor;
//         }
        checkPick(viewInfo, pnt);
    }

    void NothingEnd(const ViewInfo& viewInfo, const QPoint& pnt)
    {

    }

public:
    World *q;

    float _extent;      /* scene size */

    QMatrix4x4 _view;   /* original view transform */

    QMatrix4x4 _viewSave;   /* save original view transform when dragging */
    QPoint _pntSave;        /* save original point when dragging */

    QOpenGLShaderProgram _faceShader;   /* face shader */
    QOpenGLShaderProgram _edgeShader;   /* edge shader */

    QSharedPointer<Cube> _cube[LEVEL * LEVEL * LEVEL];  /* child cube */

    QVector3D _edgeColor;

    int _pickCube;
    int _pickDir;
    bool _picked;

    dragEvent _dragBegin;
    dragEvent _dragging;
    dragEvent _dragEnd;
};

World::World()
{
    d = new WorldImpl(this);
    setStat(NOTHING);
}

World::~World()
{
    delete d;
}

void World::init()
{
    /* [1] initialize OpenGL Functions
     */
    initializeOpenGLFunctions();
    d->initializeOpenGLFunctions();

    /* [2] initialize shader
     */
    Q_ASSERT(d->_faceShader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Resources/face.vert"));
    Q_ASSERT(d->_faceShader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Resources/face.frag"));
    Q_ASSERT(d->_faceShader.link());

    Q_ASSERT(d->_edgeShader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Resources/edge.vert"));
    Q_ASSERT(d->_edgeShader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Resources/edge.frag"));
    Q_ASSERT(d->_edgeShader.link());

    /* [3] 
     */
    for (auto cube : d->_cube)
    {
        cube->init();
    }

    /* [4]
     */
    glCullFace(GL_BACK);

    glPolygonOffset(-1.0f, -1.0f);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
}

void World::dragBegin(const ViewInfo& viewInfo, const QPoint& pnt)
{
    (d->*(d->_dragBegin))(viewInfo, pnt);
}

void World::dragging(const ViewInfo& viewInfo, const QPoint& pnt)
{
    (d->*(d->_dragging))(viewInfo, pnt);
}

void World::dragEnd(const ViewInfo& viewInfo, const QPoint& pnt)
{
    (d->*(d->_dragEnd))(viewInfo, pnt);
}

void World::zoom(int iZoomIn)
{
    static float zoomMax = 20.0f;

    float extent = d->_extent;
    float extentSave = d->_extent;

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

    /* [4] render face
     */
    d->_faceShader.bind();
    d->_faceShader.setUniformValue("projView", projView);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    for (int i = 0; i < LEVEL *LEVEL *LEVEL; i++)
    {
        d->_cube[i]->drawFace(d->_faceShader);
    }

    d->_faceShader.release();

    /* [5] render edge
    */
    d->_edgeShader.bind();
    d->_edgeShader.setUniformValue("projView", projView);
    d->_edgeShader.setUniformValue("edgeColor", edgeNormalColor);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for (int i = 0; i < LEVEL *LEVEL *LEVEL; i++)
    {
        d->_cube[i]->drawEdge(d->_edgeShader);
    }

    /* render highlight edge */
    if (d->_picked)
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        d->_edgeShader.setUniformValue("edgeColor", edgePreHighlightColor);
        d->_cube[d->_pickCube]->drawEdge(d->_edgeShader);
    }

    d->_edgeShader.release();

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

void World::setStat(World::STAT s)
{
    if (s == ROT_VIEW)
    {
        d->_dragBegin = &WorldImpl::rotViewBegin;
        d->_dragging = &WorldImpl::rotViewDoing;
        d->_dragEnd = &WorldImpl::rotViewEnd;
    }
    else if (s == ROT_MODE)
    {
        d->_dragBegin = &WorldImpl::rotModeBegin;
        d->_dragging = &WorldImpl::rotModeDoing;
        d->_dragEnd = &WorldImpl::rotModeEnd;
    }
    else if (s == NOTHING)
    {
        d->_dragBegin = &WorldImpl::NothingBegin;
        d->_dragging = &WorldImpl::NothingDoing;
        d->_dragEnd = &WorldImpl::NothingEnd;
    }
}
