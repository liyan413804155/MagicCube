#include "stdafx.h"

#include "Cmd.h"

using namespace QtSharedPointer;

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
    QStack<QSharedPointer<Cmd>> _undoCmd;
    QStack<QSharedPointer<Cmd>> _redoCmd;
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

    auto pCmd = d->_undoCmd.top();
    d->_undoCmd.pop();
    pCmd->undo();

    d->_redoCmd.push(pCmd);
}

void CmdStack::redo()
{
    if (d->_redoCmd.empty())
        return;

    auto pCmd = d->_redoCmd.top();
    d->_redoCmd.pop();
    pCmd->redo();

    d->_undoCmd.push(pCmd);
}

bool CmdStack::undoCmdName(QString& name)
{
    if (d->_undoCmd.empty())
        return false;

    name = d->_undoCmd.top()->name();

    return true;
}

bool CmdStack::redoCmdName(QString& name)
{
    if (d->_redoCmd.empty())
        return false;

    name = d->_redoCmd.top()->name();

    return true;
}

void CmdStack::revCmd(QSharedPointer<Cmd> pCmd)
{
    d->_undoCmd.push(pCmd);
    d->_redoCmd.clear();
}