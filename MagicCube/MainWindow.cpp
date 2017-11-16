#include "stdafx.h"

#include "MainWindow.h"
#include "CentralWidget.h"

struct AlignViewInfo
{
    QString _icon;
    QString _name;
    QMatrix4x4 _view;
};

const static QVector<AlignViewInfo> gsAlignViewInfo =
{
    { QString(":/Resources/sideView.png"), QString("Side View"), getPlane(D_SIDE) },
    { QString(":/Resources/topView.png"), QString("Top View"), getPlane(D_TOP) },
    { QString(":/Resources/leftView.png"), QString("Left View"), getPlane(D_LEFT) },
    { QString(":/Resources/frontView.png"), QString("Front View"), getPlane(D_FRONT) },
    { QString(":/Resources/rightView.png"), QString("Right View"), getPlane(D_RIGHT) },
    { QString(":/Resources/backView.png"), QString("Back View"), getPlane(D_BACK) },
    { QString(":/Resources/bottomView.png"), QString("Bottom View"), getPlane(D_BOTTOM) },
};

class MainWindowImpl
{
public:
    MainWindowImpl()
    {
        _centralWidget = nullptr;
        _undo = nullptr;
        _redo = nullptr;
        _cmdStack = QSharedPointer<CmdStack>(new CmdStack);
    }

    ~MainWindowImpl()
    {

    }

    void updateUndoRedoAction()
    {
        QString tip;

        if (_cmdStack->undoCmdName(tip))
        {
            _undo->setEnabled(true);
            _undo->setToolTip(QString("Undo : ") + tip);
        }
        else
        {
            _undo->setEnabled(false);
            _undo->setToolTip("");
        }

        if (_cmdStack->redoCmdName(tip))
        {
            _redo->setEnabled(true);
            _redo->setToolTip(QString("Redo : ") + tip);
        }
        else
        {
            _redo->setEnabled(false);
            _redo->setToolTip("");
        }
    }

public:
    CentralWidget * _centralWidget;
    QAction *_undo;
    QAction *_redo;
    QSharedPointer<CmdStack> _cmdStack;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /* [1] new central widget 
     */
    d = new MainWindowImpl;
    d->_centralWidget = new CentralWidget(this);
    connect(d->_centralWidget, &CentralWidget::sendCmd, this, &MainWindow::revCmd);

    /* [2] add tool bar, add undo/redo action
     */
    QToolBar *tbEdit = addToolBar(tr("Edit"));

    const QIcon undoIcon(":/Resources/undo.png");
    d->_undo = tbEdit->addAction(undoIcon, tr("Undo"), this, &MainWindow::undo);
    d->_undo->setShortcut(QKeySequence::Undo);

    const QIcon redoIcon (":/Resources/redo.png");
    d->_redo = tbEdit->addAction(redoIcon, tr("Redo"), this, &MainWindow::redo);
    d->_redo->setShortcut(QKeySequence::Redo);

    d->updateUndoRedoAction();

    /* [3] add tool bar, add align view action
    */
    QToolBar *tbAlignView = addToolBar(tr("Align View"));

    for (int i = 0; i < gsAlignViewInfo.size(); i++)
    {
        QAction *aligViewAction = tbAlignView->addAction(
            QIcon(gsAlignViewInfo[i]._icon), 
            gsAlignViewInfo[i]._name, 
            this, 
            &MainWindow::alignView);
        aligViewAction->setProperty("index", i);
    }

    /* [4] set window size
     */
    setCentralWidget(d->_centralWidget);
    setMinimumSize(QSize(500, 350));
    setBaseSize(QSize(500, 350));

}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::undo()
{
    d->_cmdStack->undo();
    d->_centralWidget->update();
    d->updateUndoRedoAction();
}

void MainWindow::redo()
{
    d->_cmdStack->redo();
    d->_centralWidget->update();
    d->updateUndoRedoAction();
}

void MainWindow::revCmd(QSharedPointer<Cmd> pCmd)
{
    d->_cmdStack->revCmd(pCmd);
    pCmd->redo();
    d->updateUndoRedoAction();
}

void MainWindow::alignView()
{
    QAction *pSender = dynamic_cast<QAction*>(sender());
    if (pSender)
    {
        int index = pSender->property("index").toInt();
        d->_centralWidget->setAlignView(gsAlignViewInfo[index]._view, gsAlignViewInfo[index]._name);
        d->_centralWidget->update();
    }
}
