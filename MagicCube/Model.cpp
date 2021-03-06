#include "stdafx.h"

#include "Cube.h"
#include "Model.h"

using namespace std;

const int modelLevel = 4;

const int vboStride = sizeof(float) * (3 /* vertex */ + 3 /* normal */ + 3 /* color */);

const QVector3D edgeNormalColor(0.0f, 0.0f, 0.0f);
const QVector3D edgePreHighlightColor(1.0f, 1.0f, 0.0f);
const QVector3D edgeHighlightColor(1.0f, 0.74f, 0.0f);

static float ambient = 0.2f;
static float diffuse = 0.4f;
static float shininess = 3.0f;
static float specular = 0.8f;

const QVector<QMatrix4x4> modelFaceXform = []()->QVector<QMatrix4x4>
{
    QVector<QMatrix4x4> faces(6);

    for (int i = 0; i < 6; i++)
    {
        faces[i] = getPlane(i);
        faces[i].translate(0.0f, 0.0f, modelLevel * 0.5f);
    }

    return faces;
}();

const QVector2D modelFaceVertex[4] =
{
    QVector2D(-modelLevel / 2.0f, -modelLevel / 2.0f),
    QVector2D( modelLevel / 2.0f, -modelLevel / 2.0f),
    QVector2D( modelLevel / 2.0f,  modelLevel / 2.0f),
    QVector2D(-modelLevel / 2.0f,  modelLevel / 2.0f),
};

class ModelImpl : public QOpenGLFunctions
{
public:
    ModelImpl()
    {
        _cubes.resize(modelLevel * modelLevel * modelLevel);
        for (int i = 0; i < _cubes.size(); i++)
        {
            _cubes[i] = QSharedPointer<Cube>(new Cube);
        }
    }
    ~ModelImpl()
    {

    }
public:
    void init(bool bFirst)
    {
        if (bFirst)
        {
            initializeOpenGLFunctions();

            bool isOK = true;

            isOK = _draw._faceShd.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Resources/face.vert");
            Q_ASSERT(isOK);
            isOK = _draw._faceShd.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Resources/face.frag");
            Q_ASSERT(isOK);
            isOK = _draw._faceShd.link();
            Q_ASSERT(isOK);

            isOK = _draw._edgeShd.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Resources/edge.vert");
            Q_ASSERT(isOK);
            isOK = _draw._edgeShd.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Resources/edge.frag");
            Q_ASSERT(isOK);
            isOK = _draw._edgeShd.link();
            Q_ASSERT(isOK);
        }

        for (int i = 0; i < _cubes.size(); i++)
        {
            _cubes[i]->init(QVector3D(
                i / (modelLevel * modelLevel) - (modelLevel - 1) / 2.0f,
                (i / modelLevel) % modelLevel - (modelLevel - 1) / 2.0f,
                i % modelLevel - (modelLevel - 1) / 2.0f));
            _pnt2cube[_cubes[i]->getOrig()] = i;
        }

        _draw._vbo.destroy();
        _draw._dragVbo.destroy();

        _drag._vbo.destroy();
        _drag._cube = -1;
        _drag._cubes.clear();

        _pick._vbo.destroy();
        _pick._cube = -1;
    }

public:

    void genVBO(const QVector<float>&data, QOpenGLBuffer& vbo, int& vtxCnt)
    {
        vbo.create();
        vbo.bind();
        vbo.allocate(&data[0], data.size() * sizeof(float));
        vtxCnt = data.size() * sizeof(float) / vboStride;
    }

    void genVBOAll()
    {
        QVector<float> data;
        QVector<float> dragData;

        for (int i = 0; i < _cubes.size(); i++)
        {
            if (_drag._cubes.count(i))
            {
                _cubes[i]->getVboData(dragData);
            }
            else
            {
                _cubes[i]->getVboData(data);
            }
        }

        genVBO(data, _draw._vbo, _draw._vtxCnt);

        if (_drag._cubes.size() > 0)
        {
            genVBO(dragData, _draw._dragVbo, _draw._dragVtxCnt);
        }
    }

    void genVBOPick()
    {
        QVector<float> data;

        _cubes[_pick._cube]->getVboData(data);
        genVBO(data, _pick._vbo, _pick._vtxCnt);
    }

    void genVBODrag()
    {
        QVector<float> data;

        _cubes[_drag._cube]->getVboData(data);
        genVBO(data, _drag._vbo, _drag._vtxCnt);
    }

    void drawFace(const QMatrix4x4& projMatrix, const QMatrix4x4& viewModelMatrix, QOpenGLBuffer& vbo, int vtxCnt)
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        _draw._faceShd.bind();
        vbo.bind();

        _draw._faceShd.setUniformValue("proj", projMatrix);
        _draw._faceShd.setUniformValue("viewModel", viewModelMatrix);
        _draw._faceShd.setUniformValue("ambient", ambient);
        _draw._faceShd.setUniformValue("diffuse", diffuse);
        _draw._faceShd.setUniformValue("shininess", shininess);
        _draw._faceShd.setUniformValue("specular", specular);

        int offset = 0;
        int vertexLocation = _draw._faceShd.attributeLocation("vertex");
        _draw._faceShd.enableAttributeArray(vertexLocation);
        _draw._faceShd.setAttributeBuffer(vertexLocation, GL_FLOAT, offset* sizeof(float), 3, vboStride);

        offset += 3;
        int normalLocation = _draw._faceShd.attributeLocation("normal");
        _draw._faceShd.enableAttributeArray(normalLocation);
        _draw._faceShd.setAttributeBuffer(normalLocation, GL_FLOAT, offset * sizeof(float), 3, vboStride);

        offset += 3;
        int colorLocation = _draw._faceShd.attributeLocation("color");
        _draw._faceShd.enableAttributeArray(colorLocation);
        _draw._faceShd.setAttributeBuffer(colorLocation, GL_FLOAT, offset * sizeof(float), 3, vboStride);

        glDrawArrays(GL_QUADS, 0, vtxCnt);

        vbo.release();
        _draw._faceShd.release();
    }

    void drawEdge(const QMatrix4x4& projViewModel, QOpenGLBuffer& vbo, int vtxCnt, const QVector3D& color, bool isThru)
    {
        if (isThru)
        {
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
        }
        else
        {
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        _draw._edgeShd.bind();
        vbo.bind();

        _draw._edgeShd.setUniformValue("projViewModel", projViewModel);
        _draw._edgeShd.setUniformValue("edgeColor", color);

        int offset = 0;
        int vertexLocation = _draw._edgeShd.attributeLocation("vertex");
        _draw._edgeShd.enableAttributeArray(vertexLocation);
        _draw._edgeShd.setAttributeBuffer(vertexLocation, GL_FLOAT, offset* sizeof(float), 3, vboStride);

        glDrawArrays(GL_QUADS, 0, vtxCnt);

        vbo.release();
        _draw._edgeShd.release();
    }

public:

    bool checkPick(const QMatrix4x4& projView, const QVector3D& wldPnt, const QVector3D& wldVec, QVector3D& isectPnt, int& dir, int& cube)
    {
        /* [1] traverse all face to find which face are picked and intersect point
        */
        QVector<QVector3D> isectPnts;
        QVector<int> dirs;

        for (int i = 0; i < 6; i++)
        {
            QVector3D isectPnt;
            if (isectLine2Face(wldPnt, wldVec, modelFaceXform[i], modelFaceVertex[0], modelFaceVertex[2], isectPnt))
            {
                isectPnts.push_back(isectPnt);
                dirs.push_back(i);
            }
        }

        if (isectPnts.size() == 0)
            return false;

        /* [2] find the nearest picked face and intersect point
        */
        int index = 0;
        float zMin = (projView * isectPnts[0]).z();

        for (int i = 1; i < isectPnts.size(); i++)
        {
            float z = (projView * isectPnts[i]).z();
            if (z < zMin)
            {
                index = i;
                zMin = z;
            }
        }

        isectPnt = isectPnts[index];
        dir = dirs[index];

        /* [3] find the intersect point belong to which cube
         */
        QVector3D zAxis = modelFaceXform[dir].column(2).toVector3D();
        QVector3D pnt = isectPnt - 0.5f * zAxis;
        if (modelLevel % 2)
        {
            pnt = QVector3D(floor(pnt.x() + 0.5f), floor(pnt.y() + 0.5f), floor(pnt.z() + 0.5f));
        }
        else
        {
            pnt = QVector3D(floor(pnt.x()) + 0.5f, floor(pnt.y()) + 0.5f, floor(pnt.z()) + 0.5f);
        }
        Q_ASSERT(_pnt2cube.find(pnt) != _pnt2cube.end());
        cube = _pnt2cube[pnt];

        return true;
    }

    bool getDragCubes(const QVector3D& vec)
    {
        /* [1] get drag angle
         */
        QVector3D xAxis = modelFaceXform[_drag._normal].column(0).toVector3D();
        QVector3D yAxis = modelFaceXform[_drag._normal].column(1).toVector3D();
        float xAngle = QVector3D::dotProduct(xAxis, vec) * 90.f / modelLevel;
        float yAngle = QVector3D::dotProduct(yAxis, vec) * 90.f / modelLevel;
        const float minAngle = 5.0f;

        if (abs(xAngle) < minAngle && abs(yAngle) < minAngle)
            return false;

        /* [2] get rotate axis and direction
         */
        if (abs(xAngle) < abs(yAngle))
        {
            _drag._vec = -yAxis;
            _drag._axis = xAxis;
        }
        else
        {
            _drag._vec = xAxis;
            _drag._axis = yAxis;
        }

        /* [3] get drag cubes
         */
        QVector3D orig = _cubes[_drag._cube]->getOrig() - modelLevel * _drag._vec;
        QVector3D zAxis = modelFaceXform[_drag._normal].column(2).toVector3D();

        for (int i = 0; i < modelLevel; i++)
        {
            for (int j = 0; j < modelLevel * 2; j++)
            {
                QVector3D pnt = orig - i * zAxis + j * _drag._vec;
                if (_pnt2cube.find(pnt) != _pnt2cube.end())
                {
                    _drag._cubes.insert(_pnt2cube[pnt]);
                }
            }
        }

        _draw._vbo.destroy();
        _draw._dragVbo.destroy();

        return true;
    }

public:

    QVector<QSharedPointer<Cube>> _cubes;  /* child cubes */
    map<QVector3D, int> _pnt2cube;

    struct
    {
        QOpenGLBuffer _vbo;
        int _vtxCnt;
        QOpenGLBuffer _dragVbo;
        int _dragVtxCnt;
        QOpenGLShaderProgram _faceShd;   /* face shader */
        QOpenGLShaderProgram _edgeShd;   /* edge shader */
    }_draw;
    
    struct 
    {
        int _cube;
        QOpenGLBuffer _vbo;
        int _vtxCnt;

        QVector3D _pnt;
        int _normal;
        QVector3D _vec;
        QVector3D _axis;
        float _angle;

        set<int> _cubes;
        QMatrix4x4 _model;
    }_drag;

    struct
    {
        int _cube;
        QOpenGLBuffer _vbo;
        int _vtxCnt;
    }_pick;
};

Model::Model()
{
    d = new ModelImpl;
}

Model::~Model()
{
    delete d;
}

void Model::init(bool bFirst)
{
    if (bFirst)
    {
        initializeOpenGLFunctions();
    }
    d->init(bFirst);
}

bool Model::pick(const QMatrix4x4& projView, const QVector3D& wldPnt, const QVector3D& wldVec)
{
    QVector3D isectPnt;
    int dir;
    int cube;

    if (!d->checkPick(projView, wldPnt, wldVec, isectPnt, dir, cube))
    {
        d->_pick._vbo.destroy();
        d->_pick._cube = -1;
        return false;
    }

    emit setCoord(isectPnt);

    if (cube != d->_pick._cube)
        d->_pick._vbo.destroy();
    d->_pick._cube = cube;
    return true;
}

bool Model::dragBegin(const QMatrix4x4& projView, const QVector3D& wldPnt, const QVector3D& wldVec)
{
    d->_drag._cubes.clear();
    d->_draw._vbo.destroy();
    d->_draw._dragVbo.destroy();

    int cube;
    if (!d->checkPick(projView, wldPnt, wldVec, d->_drag._pnt, d->_drag._normal, cube))
    {
        d->_drag._vbo.destroy();
        d->_drag._cube = -1;
        return false;
    }

    if (cube != d->_drag._cube)
        d->_drag._vbo.destroy();
    d->_drag._cube = cube;

    return true;
}

void Model::dragging(const QMatrix4x4& projView, const QVector3D& wldPnt, const QVector3D& wldVec)
{
    if (d->_drag._cube < 0)
        return;

    /* [1] get the intersection point
     */
    QVector3D pnt;
    if (!isectLine2Plane(wldPnt, wldVec, modelFaceXform[d->_drag._normal], pnt))
        return;

    emit setCoord(pnt);

    /* [2] get which cubes are dragged
     */
    QVector3D vec = pnt - d->_drag._pnt;

    if (d->_drag._cubes.empty())
    {
        if (!d->getDragCubes(vec))
            return;
    }

    /* [3] set the cubes model matrix
     */
    d->_drag._angle = QVector3D::dotProduct(d->_drag._vec, vec) * 90.f / modelLevel;
    d->_drag._model.setToIdentity();
    d->_drag._model.rotate(d->_drag._angle, d->_drag._axis);
}

void Model::dragEnd(const QMatrix4x4& projView, const QVector3D& wldPnt, const QVector3D& wldVec)
{
    d->_drag._model.setToIdentity();

    if (d->_drag._cube < 0)
        return;

    if (d->_drag._cubes.empty())
        return;

    /* [1] calculate the rotate angle
     */

    int iAngle = d->_drag._angle > 0.0f ? (d->_drag._angle + 45.0f) / 90.0f 
        : (d->_drag._angle - 45.0f) / 90.0f;

    if (!iAngle)
        return;
    d->_drag._angle = iAngle * 90.f;

    /* [2] send command
     */
    QMatrix4x4 xform;
    xform.rotate(d->_drag._angle, d->_drag._axis);

    set<int> dragCube = d->_drag._cubes;

    auto cmdFunc = [this, dragCube](const QMatrix4x4& xform)
    {
        for (auto i : dragCube)
        {
            d->_cubes[i]->setXform(xform);
            d->_pnt2cube[d->_cubes[i]->getOrig()] = i;
        }
        d->_drag._cubes.clear();
        d->_draw._vbo.destroy();
        d->_draw._dragVbo.destroy();
    };

    QSharedPointer<Cmd> pCmd = QSharedPointer<Cmd>(new Cmd(
        [cmdFunc, xform]()
    {
        QMatrix4x4 inv = xform.inverted();
        cmdFunc(inv);
    },
        [cmdFunc, xform]()
    {
        cmdFunc(xform);
    },
        QString("model transform")));

    emit sendCmd(pCmd);
}

void Model::draw(const QMatrix4x4& projMatrix, const QMatrix4x4& viewMatrix)
{
    if (!d->_draw._vbo.isCreated())
    {
        d->genVBOAll();
    }

    /* [1] normal cube face
     */
    d->drawFace(projMatrix, viewMatrix, d->_draw._vbo, d->_draw._vtxCnt);

    /* [2] dragging cube face
     */
    if (d->_drag._cubes.size())
    {
        d->drawFace(projMatrix, viewMatrix * d->_drag._model, d->_draw._dragVbo, d->_draw._dragVtxCnt);
    }

    /* [3] normal cube edge
    */
    d->drawEdge(projMatrix * viewMatrix, d->_draw._vbo, d->_draw._vtxCnt, edgeNormalColor, false);

    /* [4] dragging cube edge
    */
    if (d->_drag._cubes.size())
    {
        d->drawEdge(projMatrix * viewMatrix * d->_drag._model, d->_draw._dragVbo, d->_draw._dragVtxCnt, edgeHighlightColor, true);
    }

    /* [5] cube edge that will drag
     */
    if (d->_drag._cube >= 0 && !d->_drag._cubes.count(d->_drag._cube))
    {
        if (!d->_drag._vbo.isCreated())
        { 
            d->genVBODrag();
        }
        d->drawEdge(projMatrix * viewMatrix, d->_drag._vbo, d->_drag._vtxCnt, edgeHighlightColor, true);
    }

    /* [6] pick cube edge
    */
    if (d->_pick._cube >= 0 && !d->_drag._cubes.count(d->_pick._cube) && d->_pick._cube != d->_drag._cube)
    {
        if (!d->_pick._vbo.isCreated())
        {
            d->genVBOPick();
        }
        d->drawEdge(projMatrix * viewMatrix, d->_pick._vbo, d->_pick._vtxCnt, edgePreHighlightColor, true);
    }
}
