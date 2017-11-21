#pragma once

#include "Cmd.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

private slots:
    void undo();
    void redo();
    void revCmd(QSharedPointer<Cmd> pCmd);
    void alignView();
    void setMsg(const QString& msg);

private:
    DECL_PRI(MainWindow);
};
