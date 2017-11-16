#include "stdafx.h"

#include "Cmd.h"

using namespace QtSharedPointer;

const int MAX_UNDO_COUNT = 100;

class CmdImpl
{
public:
    QString _name;
    Transaction _undo;
    Transaction _redo;
};


Cmd::Cmd(Transaction fUndo, Transaction fRedo, const QString& sName)
{
    d = new CmdImpl;
    d->_name = sName;
    d->_undo = fUndo;
    d->_redo = fRedo;
}

Cmd::~Cmd()
{
    delete d;
}

void Cmd::undo()
{
    d->_undo();
}

void Cmd::redo()
{
    d->_redo();
}

QString Cmd::name()
{
    return d->_name;
}

class CmdStackImpl
{
public:
    CmdStackImpl()
    {

    }
    ~CmdStackImpl()
    {

    }
public:
    QList<QSharedPointer<Cmd>> _undoCmd;
    QList<QSharedPointer<Cmd>> _redoCmd;
};

CmdStack::CmdStack()
{
    d = new CmdStackImpl;
}

CmdStack::~CmdStack()
{
    delete d;
}

void CmdStack::undo()
{
    if (d->_undoCmd.empty())
        return;

    auto pCmd = d->_undoCmd.back();
    d->_undoCmd.removeLast();
    pCmd->undo();

    d->_redoCmd.push_back(pCmd);
}

void CmdStack::redo()
{
    if (d->_redoCmd.empty())
        return;

    auto pCmd = d->_redoCmd.back();
    d->_redoCmd.removeLast();
    pCmd->redo();

    d->_undoCmd.push_back(pCmd);
}

bool CmdStack::undoCmdName(QString& name)
{
    if (d->_undoCmd.empty())
        return false;

    name = d->_undoCmd.back()->name();

    return true;
}

bool CmdStack::redoCmdName(QString& name)
{
    if (d->_redoCmd.empty())
        return false;

    name = d->_redoCmd.back()->name();

    return true;
}

void CmdStack::revCmd(QSharedPointer<Cmd> pCmd)
{
    d->_undoCmd.push_back(pCmd);
    d->_redoCmd.clear();

    if (d->_undoCmd.size() > MAX_UNDO_COUNT)
    {
        d->_undoCmd.removeFirst();
    }
}