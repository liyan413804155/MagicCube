#pragma once

class Cmd
{
public:
    Cmd(Transaction fUndo, Transaction fRedo, const QString& sName);
    ~Cmd();

public:
    QString name();
    void undo();
    void redo();

private:
    friend class CmdImpl;
    CmdImpl *d;
};

class CmdStack
{
public:
    CmdStack();
    ~CmdStack();

public:
    void undo();
    void redo();
    bool undoCmdName(QString& name);
    bool redoCmdName(QString& name);
    void revCmd(QSharedPointer<Cmd> pCmd);

private:
    friend class CmdStackImpl;
    CmdStackImpl *d;
};
