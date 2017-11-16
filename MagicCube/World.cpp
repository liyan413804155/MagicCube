#include "stdafx.h"

#include "World.h"
#include "Cube.h"

using namespace std;

/* cube level
 */
static const int LEVEL = 4;

/* define edge color
 */
const QVector3D edgeNormalColor(0.0f, 0.0f, 0.0f);
const QVector3D edgePreHighlightColor(1.0f, 1.0f, 0.0f);
const QVector3D edgeHighlightColor(1.0f, 0.74f, 0.0f);

class WorldImpl : public QOpenGLFunctions
{
public:
    typedef void (WorldImpl::*dragEvent)(const ViewInfo& viewInfo, const QPoint& pnt);

public:
    WorldImpl(World* self)
    {
        q = self;

        _extent = LEVEL * 1.2f;

        /* init view matrix
         */
        _view.rotate(45.0f, 1.0f, 1.0f, 1.0f);

        /* init all cube
         */
        _cubes.resize(LEVEL * LEVEL * LEVEL);

        for (int i = 0; i < _cubes.size(); i++)
        {
            _cubes[i] = QSharedPointer<Cube>(new Cube(QVector3D(
                i / (LEVEL*LEVEL) - (LEVEL - 1) / 2.0f,
                (i / LEVEL) % LEVEL - (LEVEL - 1) / 2.0f,
                i % LEVEL - (LEVEL - 1) / 2.0f)));
        }

        /* init pick information
         */
        _prePicked = false;
        _prePickCube = -1;
        _prePickDir = -1;

        _picked = false;
        _pickCube = -1;
        _pickDir = -1;
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

    bool checkPick(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        /* [1] calculate ray in world coordinate
         */
        QVector3D locPnt = dev2Loc(viewInfo, pnt);
        QMatrix4x4 xform = getProjView(viewInfo);
        QMatrix4x4 xformRev = xform.inverted();
        QVector3D wldPnt = xformRev * locPnt;                   /* ray point */
        QVector3D wldVec = xformRev.column(2).toVector3D();     /* ray direct */

        /* [2] traverse all cubes to find which cubes are picked
         */
        QVector<int> pickCubes;
        QVector<QVector3D> pickPnts;
        QVector<int> pickDirs;

        _prePicked = false;
        _prePickCube = -1;
        _prePickDir = -1;

        for (int i = 0; i < _cubes.size(); i++)
        {
            QVector<QVector3D> tPickPnts;
            QVector<int> tPickDirs;
            if (_cubes[i]->checkPick(wldPnt, wldVec, tPickDirs, tPickPnts))
            {
                for (int j = 0; j < tPickPnts.size(); j++)
                {
                    pickCubes.push_back(i);
                    pickPnts.push_back(tPickPnts[j]);
                    pickDirs.push_back(tPickDirs[j]);
                }
            }
        }

        if (pickPnts.size() == 0)
            return false;

        /* [3] find the nearest picked cube
         */
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

        _prePickCube = pickCubes[pickCube];
        _prePickDir = pickDirs[pickCube];
        _prePicked = true;

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

    void rotModeBegin(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        _picked = checkPick(viewInfo, pnt);

        _dragCube.clear();
        _pickCube = -1;
        _pickDir = -1;
        _dragAngle = 0.0f;


        if (_picked)
        {
            _pickCube = _prePickCube;
            _pickDir = _prePickDir;
            _pntSave = pnt;
        }
    }

    void rotModeDoing(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        if (_picked)
        {
            QVector3D dragVec = QVector3D(pnt.x() - _pntSave.x(), _pntSave.y() -pnt.y(), 0.0f);
            if (dragVec.length() < 10.0f)
                return;

            QSharedPointer<Cube> pickCube = _cubes[_pickCube];
            
            QMatrix4x4 pickCubeFace = pickCube->getFace(_pickDir);
            QVector3D pickCubeXAxis = pickCubeFace.column(0).toVector3D();
            QVector3D pickCubeYAxis = pickCubeFace.column(1).toVector3D();
            QVector3D pickCubeZAxis = -pickCubeFace.column(2).toVector3D();
            QVector3D pickCubOrig = pickCube->getOrig();

            QMatrix4x4 projViewInv = getProjView(viewInfo).inverted();
            QVector3D dragToWld = (projViewInv * QVector4D(dragVec, 0.0f)).toVector3D();
            float dragXProj = QVector3D::dotProduct(pickCubeXAxis, dragToWld);
            float dragYProj = QVector3D::dotProduct(pickCubeYAxis, dragToWld);

            if (_dragCube.empty())
            {
                QVector3D axis;

                if (qAbs(dragXProj) > qAbs(dragYProj))
                {
                    axis = pickCubeXAxis;
                    _dragDir = 0;
                }
                else
                {
                    axis = pickCubeYAxis;
                    _dragDir = 1;
                }

                QVector3D orig = pickCubOrig - LEVEL * axis;
                set<QVector3D> origSet;
                for (int i = 0; i < LEVEL; i++)
                {
                    for (int j = 0; j < LEVEL * 2; j++)
                    {
                        origSet.insert(orig + i * pickCubeZAxis + j * axis);
                    }
                }

                for (int i = 0; i < _cubes.size(); i++)
                {
                    if (origSet.count(_cubes[i]->getOrig()))
                    {
                        _dragCube.insert(i);
                    }
                }
            }

            float r = 90.0f * (_extent / LEVEL) / qMin(viewInfo._viewport.width(), viewInfo._viewport.height());

            if (_dragDir)
            {
                _dragAngle = r * dragYProj;
                _dragXform.setToIdentity();
                _dragXform.rotate(_dragAngle, pickCubeXAxis);
            }
            else
            {
                _dragAngle = r * dragXProj;
                _dragXform.setToIdentity();
                _dragXform.rotate(_dragAngle, pickCubeYAxis);
            }

            for (auto i : _dragCube)
            {
                _cubes[i]->setModel(_dragXform);
            }
        }
    }

    void rotModeEnd(const ViewInfo& viewInfo, const QPoint& pnt)
    {
        if (_picked && !_dragCube.empty())
        {
            int n = _dragAngle > 0.0f ? (_dragAngle + 45.0f) / 90.0f : (_dragAngle - 45.0f) / 90.0f;

            if (n)
            {
                _dragAngle = n * 90;

                QSharedPointer<Cube> pickCube = _cubes[_pickCube];

                QMatrix4x4 pickCubeFace = pickCube->getFace(_pickDir);
                QVector3D pickCubeXAxis = pickCubeFace.column(0).toVector3D();
                QVector3D pickCubeYAxis = pickCubeFace.column(1).toVector3D();

                if (_dragDir)
                {
                    _dragXform.setToIdentity();
                    _dragXform.rotate(_dragAngle, pickCubeXAxis);
                }
                else
                {
                    _dragXform.setToIdentity();
                    _dragXform.rotate(_dragAngle, pickCubeYAxis);
                }

                QMatrix4x4 xform = _dragXform;
                QMatrix4x4 xformInv = _dragXform.inverted();
                set<int> dragCube = _dragCube;

                QSharedPointer<Cmd> pCmd = QSharedPointer<Cmd>(new Cmd(
                    [this, dragCube, xformInv]()
                {
                    for (auto i : dragCube)
                    {
                        _cubes[i]->setXform(xformInv);
                    }
                },
                    [this, dragCube, xform]()
                {
                    for (auto i : dragCube)
                    {
                        _cubes[i]->setXform(xform);
                    }
                },
                    QString("model transform")));

                emit q->sendCmd(pCmd);
            }



            for (auto i : _dragCube)
            {
                _cubes[i]->setModel(QMatrix4x4());
            }
        }

    }

    void NothingBegin(const ViewInfo& viewInfo, const QPoint& pnt)
    {

    }

    void NothingDoing(const ViewInfo& viewInfo, const QPoint& pnt)
    {
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

    QVector<QSharedPointer<Cube>> _cubes;  /* child cube */

    int _prePickCube;
    int _prePickDir;
    bool _prePicked;

    set<int> _dragCube;
    int _dragDir;       /* cube dragged direction. 0 x-axis, 1 y-axis */
    QMatrix4x4 _dragXform;
    float _dragAngle;

    int _pickCube;  /* picked cube index */
    int _pickDir;   /* cube picked face direction */
    bool _picked;   /*  */

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

    /* [3] init cube
     */
    for (int i = 0; i < d->_cubes.size(); i++)
    {
        d->_cubes[i]->init();
    }

    /* [4] Setup OpenGL state 
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

    /* [4] render face
     */
    d->_faceShader.bind();
    d->_faceShader.setUniformValue("projView", projView);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    for (int i = 0; i < d->_cubes.size(); i++)
    {
        d->_cubes[i]->drawFace(d->_faceShader);
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
    for (int i = 0; i < d->_cubes.size(); i++)
    {
        d->_cubes[i]->drawEdge(d->_edgeShader);
    }

    /* render highlight edge */
    if (d->_prePicked || d->_picked)
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        if (d->_picked)
        {
            d->_edgeShader.setUniformValue("edgeColor", edgeHighlightColor);
            d->_cubes[d->_pickCube]->drawEdge(d->_edgeShader);
            for (auto i : d->_dragCube)
            {
                d->_cubes[i]->drawEdge(d->_edgeShader);
            }
        }

        if (d->_prePicked && (d->_pickCube != d->_prePickCube && 
            !d->_dragCube.count(d->_prePicked)))
        {
            d->_edgeShader.setUniformValue("edgeColor", edgePreHighlightColor);
            d->_cubes[d->_prePickCube]->drawEdge(d->_edgeShader);
        }

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
