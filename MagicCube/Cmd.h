#pragma once

typedef std::function<void(void)> Transaction;

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
    DECL_PRI(Cmd);
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
    DECL_PRI(CmdStack);
};
