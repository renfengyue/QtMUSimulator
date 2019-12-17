#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "muview.h"
#include "globaltype.h"
#include "profile.h"

namespace Ui {
class MainWindow;
}

enum EAPPSTATE
{
    eUninitialized = 0,
    eConfigurated,
    eRunning,
    eStopped
};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void OnOpen();
    void OnStart();
    void OnApply();
    void OnExit();

private:
    void InitMUView(MUView& view, MUPageIndex index);
    void WriteUI(const MU_CONF& conf);
    void WriteGeneral(const MU_CONF& conf);
    void WriteView(MUView& view, const MU_CONF& conf);
    void ReadUI(MU_CONF& conf);
    void ReadGeneral(MU_CONF& conf);
    void ReadView(MUView& view, MU_CONF& conf);
    void UpdateApplication();
    void PrepareEthList();


    Ui::MainWindow *ui;
    MUView* m_pMUViews[8];
    Profile m_Profile;
    EAPPSTATE m_appState;
};

#endif // MAINWINDOW_H
