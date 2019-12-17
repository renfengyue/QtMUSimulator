#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "muconfiguration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "all.h"

#include <QFileDialog>

extern "C" void StartSimu(MU_CONF* MUConf);
extern "C" void StopSimu();
extern "C" void DetectEthIF(ETH_INTERFACE_LIST* list);
extern "C" void ChooseEth(MU_CONF* MUConf, int32 EthIndex);
extern "C" char ascii_to_uint (const char *astr, unsigned int *u_int);


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for(int i = eMU1; i <= eMU8; i++)
    {
        m_pMUViews[i] = new MUView(static_cast<MUPageIndex>(i));
        InitMUView(*m_pMUViews[i], static_cast<MUPageIndex>(i));
        m_pMUViews[i]->InitializeTable();
    }
    m_appState = eUninitialized;

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(OnOpen()));
    connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(OnStart()));
    connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(OnApply()));
    connect(ui->btnExit, SIGNAL(clicked()), this, SLOT(OnExit()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(OnExit()));

    UpdateApplication();
    PrepareEthList();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::PrepareEthList()
{
    DetectEthIF(&EthInterfaces);
    ui->cmbNetwork->clear();
    for(int i = 0; i < EthInterfaces.number; i++)
    {
        ui->cmbNetwork->addItem(EthInterfaces.name[i]);
    }
}


void MainWindow::OnOpen()
{
    QString target = QFileDialog::getOpenFileName(this, tr("Open File"), ".");
    Profile::ReadProfile(target.toStdString().data(), MUConfiguration::GetInstance().m_cacheConfiguration);
    ChooseEth(&MUConfiguration::GetInstance().m_cacheConfiguration, ui->cmbNetwork->currentIndex());

    MUConfiguration::GetInstance().Save();

    WriteUI(MUConfiguration::GetInstance().m_Configuration);
    m_appState = eConfigurated;
    UpdateApplication();
}
void MainWindow::OnExit()
{
    if(m_appState == eRunning)
    {
        StopSimu();
    }
    this->close();
    qApp->quit();
}
void MainWindow::OnStart()
{
    if(m_appState == eUninitialized) return;
    if((m_appState == eConfigurated)||(m_appState == eStopped))
    {
        StartSimu(&MUConfiguration::GetInstance().m_Configuration);
        m_appState = eRunning;
        UpdateApplication();
    }
    else if(m_appState == eRunning)
    {
        StopSimu();
        m_appState = eConfigurated;
        UpdateApplication();
    }
}
void MainWindow::OnApply()
{
    if((m_appState == eConfigurated)||(m_appState == eStopped))
    {
        ReadUI(MUConfiguration::GetInstance().m_cacheConfiguration);
        ChooseEth(&MUConfiguration::GetInstance().m_cacheConfiguration, ui->cmbNetwork->currentIndex());
        if(memcmp(&MUConfiguration::GetInstance().m_cacheConfiguration, &MUConfiguration::GetInstance().m_Configuration,sizeof(MU_CONF)) != 0)
        {
            MUConfiguration::GetInstance().Save();
        }
    }
}

void MainWindow::UpdateApplication()
{
    if(m_appState == eUninitialized)
    {
        ui->btnStart->setEnabled(false);
        ui->btnStart->setText("Start");
        ui->btnApply->setEnabled(false);
    }
    else if((m_appState == eConfigurated)||(m_appState == eStopped))
    {
        ui->btnStart->setEnabled(true);
        ui->btnStart->setText("Start");
        ui->btnApply->setEnabled(true);
    }
    else if(m_appState == eRunning)
    {
        ui->btnStart->setEnabled(true);
        ui->btnStart->setText("Stop");
        ui->btnApply->setEnabled(false);
    }
}
void MainWindow::WriteUI(const MU_CONF& conf)
{
    WriteGeneral(conf);
    for(int i = eMU1; i <= eMU8; i++)
    {
       WriteView(*m_pMUViews[i], conf);
    }
}

void MainWindow::WriteGeneral(const MU_CONF& conf)
{
    ui->spinMUCount->setValue(conf.mu_no);
    ui->spinCnlCount->setValue(conf.channel_no);
    ui->chklost_1->setChecked(conf.enableMULostFrame[0]);
    ui->chklost_2->setChecked(conf.enableMULostFrame[1]);
    ui->chklost_3->setChecked(conf.enableMULostFrame[2]);
    ui->chklost_4->setChecked(conf.enableMULostFrame[3]);
    ui->chklost_5->setChecked(conf.enableMULostFrame[4]);
    ui->chklost_6->setChecked(conf.enableMULostFrame[5]);
    ui->chklost_7->setChecked(conf.enableMULostFrame[6]);
    ui->chklost_8->setChecked(conf.enableMULostFrame[7]);
    if(conf.freq == 60)
    {
        ui->cmbFreq->setCurrentIndex(1);
    }
    else
    {
        ui->cmbFreq->setCurrentIndex(0);
    }
    if(conf.asdu_no == 2)
    {
        ui->cmbAsdu->setCurrentIndex(1);
    }
    else if(conf.asdu_no == 1)
    {
        ui->cmbAsdu->setCurrentIndex(0);
    }
    char offset[32];
    snprintf(offset, 32, "%d", conf.offset);
    ui->edtOffset->setText(offset);
    ui->cmbSimuType->setCurrentIndex(conf.sim_type);
    char lost[1024];
    memset(lost, 0, 1024);
    for(uint32 i = 0; i < conf.lost_num; i++)
    {
        char temp[5];
        snprintf(temp, 5, "%d:", conf.lost[i]);
        strcat(lost, temp);
    }
    lost[strlen(lost)-1] = '\0';
    ui->edtLostSmps->setText(lost);
}

void MainWindow::WriteView(MUView& view, const MU_CONF& conf)
{
    char value[32];

    snprintf(value, 32, "%.3f", conf.ua[view.m_MUPageIndex]);
    view.m_pTable->item(0,0)->setText(value);
    snprintf(value, 32, "%.3f", conf.phase_a[view.m_MUPageIndex]);
    view.m_pTable->item(0,1)->setText(value);

    snprintf(value, 32, "%.3f", conf.ub[view.m_MUPageIndex]);
    view.m_pTable->item(1,0)->setText(value);
    snprintf(value, 32, "%.3f", conf.phase_b[view.m_MUPageIndex]);
    view.m_pTable->item(1,1)->setText(value);

    snprintf(value, 32, "%.3f", conf.uc[view.m_MUPageIndex]);
    view.m_pTable->item(2,0)->setText(value);
    snprintf(value, 32, "%.3f", conf.phase_c[view.m_MUPageIndex]);
    view.m_pTable->item(2,1)->setText(value);

    snprintf(value, 32, "%.3f", conf.ia[view.m_MUPageIndex]);
    view.m_pTable->item(4,0)->setText(value);
    snprintf(value, 32, "%.3f", conf.phase_a[view.m_MUPageIndex]);
    view.m_pTable->item(4,1)->setText(value);

    snprintf(value, 32, "%.3f", conf.ib[view.m_MUPageIndex]);
    view.m_pTable->item(5,0)->setText(value);
    snprintf(value, 32, "%.3f", conf.phase_b[view.m_MUPageIndex]);
    view.m_pTable->item(5,1)->setText(value);

    snprintf(value, 32, "%.3f", conf.ic[view.m_MUPageIndex]);
    view.m_pTable->item(6,0)->setText(value);
    snprintf(value, 32, "%.3f", conf.phase_c[view.m_MUPageIndex]);
    view.m_pTable->item(6,1)->setText(value);

    view.m_pSvId->setText(conf.sv_id);
    view.m_pInvalid->setChecked(conf.invalid & (1 << view.m_MUPageIndex));
    view.m_pQues->setChecked(conf.quest & (1 << view.m_MUPageIndex));
    view.m_pTest->setChecked(conf.test & (1 << view.m_MUPageIndex));
    snprintf(value, 32, "%f", conf.ctratio[view.m_MUPageIndex]);
    view.m_pCTr->setText(value);
    snprintf(value, 32, "%f", conf.vtratio[view.m_MUPageIndex]);
    view.m_pVTr->setText(value);
    view.m_pSyncMode->setCurrentIndex(conf.sync[view.m_MUPageIndex] - 0x30);

    view.m_pSimul->setChecked(conf.simul & (1 << view.m_MUPageIndex));
}


void MainWindow::ReadUI(MU_CONF& conf)
{
    ReadGeneral(conf);
    for(int i = eMU1; i <= eMU8; i++)
    {
       ReadView(*m_pMUViews[i], conf);
    }
}

void MainWindow::ReadGeneral(MU_CONF& conf)
{
    conf.mu_no = ui->spinMUCount->value();
    conf.channel_no = ui->spinCnlCount->value();
    conf.enableMULostFrame[0] = ui->chklost_1->isChecked();
    conf.enableMULostFrame[1] = ui->chklost_2->isChecked();
    conf.enableMULostFrame[2] = ui->chklost_3->isChecked();
    conf.enableMULostFrame[3] = ui->chklost_4->isChecked();
    conf.enableMULostFrame[4] = ui->chklost_5->isChecked();
    conf.enableMULostFrame[5] = ui->chklost_6->isChecked();
    conf.enableMULostFrame[6] = ui->chklost_7->isChecked();
    conf.enableMULostFrame[7] = ui->chklost_8->isChecked();

    if(ui->cmbFreq->currentIndex() == 1)
    {
        conf.freq = 60;
    }
    else
    {
        conf.freq = 50;
    }
    if(ui->cmbAsdu->currentIndex() == 1)
    {
        conf.asdu_no = 2;
    }
    else
    {
        conf.asdu_no = 1;
    }


    conf.offset = ui->edtOffset->text().toFloat();
    conf.sim_type = static_cast<SIM_TYPE>(ui->cmbSimuType->currentIndex());


    char lost[1024];
    char *p_token2 = NULL;
    memset(lost, 0, 1024);
    snprintf(lost, 1024, "%s", ui->edtLostSmps->text().toStdString().c_str());
    conf.lost_num = 0;

    p_token2 = strtok (lost, ":");

    for (uint32 idx = 0; idx < 20; idx++)
    {
        if (p_token2 != NULL)
        {
            conf.lost_num++;
            (void) ascii_to_uint (p_token2, (uint32 *)&conf.lost[idx]);
        }
        p_token2 = strtok (NULL, ":");
    }
}

void MainWindow::ReadView(MUView& view, MU_CONF& conf)
{
    conf.ua[view.m_MUPageIndex] = view.m_pTable->item(0,0)->text().toFloat();
    conf.phase_a[view.m_MUPageIndex] = view.m_pTable->item(0,1)->text().toFloat();

    conf.ub[view.m_MUPageIndex] = view.m_pTable->item(1,0)->text().toFloat();
    conf.phase_b[view.m_MUPageIndex] = view.m_pTable->item(1,1)->text().toFloat();

    conf.uc[view.m_MUPageIndex] = view.m_pTable->item(2,0)->text().toFloat();
    conf.phase_c[view.m_MUPageIndex] = view.m_pTable->item(2,1)->text().toFloat();

    conf.ia[view.m_MUPageIndex] = view.m_pTable->item(4,0)->text().toFloat();
    conf.phase_a[view.m_MUPageIndex] = view.m_pTable->item(4,1)->text().toFloat();

    conf.ib[view.m_MUPageIndex] = view.m_pTable->item(5,0)->text().toFloat();
    conf.phase_b[view.m_MUPageIndex] = view.m_pTable->item(5,1)->text().toFloat();

    conf.ic[view.m_MUPageIndex] = view.m_pTable->item(6,0)->text().toFloat();
    conf.phase_c[view.m_MUPageIndex] = view.m_pTable->item(6,1)->text().toFloat();


    snprintf(conf.sv_id, 64, "%s", view.m_pSvId->text().toStdString().c_str());


    if(view.m_pInvalid->isChecked() == true)
    {
        conf.invalid |= 1 << view.m_MUPageIndex;
    }
    else
    {
        conf.invalid &= ~(1 << view.m_MUPageIndex);
    }

    if(view.m_pQues->isChecked() == true)
    {
        conf.quest |= 1 << view.m_MUPageIndex;
    }
    else
    {
        conf.quest &= ~(1 << view.m_MUPageIndex);
    }

    if(view.m_pTest->isChecked() == true)
    {
        conf.test |= 1 << view.m_MUPageIndex;
    }
    else
    {
        conf.test &= ~(1 << view.m_MUPageIndex);
    }

    conf.ctratio[view.m_MUPageIndex] = view.m_pCTr->text().toFloat();
    conf.vtratio[view.m_MUPageIndex] = view.m_pVTr->text().toFloat();
    conf.sync[view.m_MUPageIndex] = view.m_pSyncMode->currentIndex() + 0x30;

    if(view.m_pSimul->isChecked() == true)
    {
        conf.simul |= 1 << view.m_MUPageIndex;
    }
    else
    {
        conf.simul &= ~(1 << view.m_MUPageIndex);
    }
}




void MainWindow::InitMUView(MUView& view, MUPageIndex index)
{
    switch(index)
    {
    case eMU1:
        view.m_pTable = ui->tbMU_1;
        view.m_pSvId = ui->edtSvId_1;
        view.m_pTest = ui->chkqTest_1;
        view.m_pInvalid = ui->chkqInvalid_1;
        view.m_pQues = ui->chkqQues_1;
        view.m_pSyncMode = ui->cmbSyncMode_1;
        view.m_pCTr = ui->edtCTr_1;
        view.m_pVTr = ui->edtVTr_1;
        view.m_pSimul = ui->chk_Simu_1;
        break;
    case eMU2:
        view.m_pTable = ui->tbMU_2;
        view.m_pSvId = ui->edtSvId_2;
        view.m_pTest = ui->chkqTest_2;
        view.m_pInvalid = ui->chkqInvalid_2;
        view.m_pQues = ui->chkqQues_2;
        view.m_pSyncMode = ui->cmbSyncMode_2;
        view.m_pCTr = ui->edtCTr_2;
        view.m_pVTr = ui->edtVTr_2;
        view.m_pSimul = ui->chk_Simu_2;
        break;
    case eMU3:
        view.m_pTable = ui->tbMU_3;
        view.m_pSvId = ui->edtSvId_3;
        view.m_pTest = ui->chkqTest_3;
        view.m_pInvalid = ui->chkqInvalid_3;
        view.m_pQues = ui->chkqQues_3;
        view.m_pSyncMode = ui->cmbSyncMode_3;
        view.m_pCTr = ui->edtCTr_3;
        view.m_pVTr = ui->edtVTr_3;
        view.m_pSimul = ui->chk_Simu_3;
        break;
    case eMU4:
        view.m_pTable = ui->tbMU_4;
        view.m_pSvId = ui->edtSvId_4;
        view.m_pTest = ui->chkqTest_4;
        view.m_pInvalid = ui->chkqInvalid_4;
        view.m_pQues = ui->chkqQues_4;
        view.m_pSyncMode = ui->cmbSyncMode_4;
        view.m_pCTr = ui->edtCTr_4;
        view.m_pVTr = ui->edtVTr_4;
        view.m_pSimul = ui->chk_Simu_4;
        break;
    case eMU5:
        view.m_pTable = ui->tbMU_5;
        view.m_pSvId = ui->edtSvId_5;
        view.m_pTest = ui->chkqTest_5;
        view.m_pInvalid = ui->chkqInvalid_5;
        view.m_pQues = ui->chkqQues_5;
        view.m_pSyncMode = ui->cmbSyncMode_5;
        view.m_pCTr = ui->edtCTr_5;
        view.m_pVTr = ui->edtVTr_5;
        view.m_pSimul = ui->chk_Simu_5;
        break;
    case eMU6:
        view.m_pTable = ui->tbMU_6;
        view.m_pSvId = ui->edtSvId_6;
        view.m_pTest = ui->chkqTest_6;
        view.m_pInvalid = ui->chkqInvalid_6;
        view.m_pQues = ui->chkqQues_6;
        view.m_pSyncMode = ui->cmbSyncMode_6;
        view.m_pCTr = ui->edtCTr_6;
        view.m_pVTr = ui->edtVTr_6;
        view.m_pSimul = ui->chk_Simu_6;
        break;
    case eMU7:
        view.m_pTable = ui->tbMU_7;
        view.m_pSvId = ui->edtSvId_7;
        view.m_pTest = ui->chkqTest_7;
        view.m_pInvalid = ui->chkqInvalid_7;
        view.m_pQues = ui->chkqQues_7;
        view.m_pSyncMode = ui->cmbSyncMode_7;
        view.m_pCTr = ui->edtCTr_7;
        view.m_pVTr = ui->edtVTr_7;
        view.m_pSimul = ui->chk_Simu_7;
        break;
    case eMU8:
        view.m_pTable = ui->tbMU_8;
        view.m_pSvId = ui->edtSvId_8;
        view.m_pTest = ui->chkqTest_8;
        view.m_pInvalid = ui->chkqInvalid_8;
        view.m_pQues = ui->chkqQues_8;
        view.m_pSyncMode = ui->cmbSyncMode_8;
        view.m_pCTr = ui->edtCTr_8;
        view.m_pVTr = ui->edtVTr_8;
        view.m_pSimul = ui->chk_Simu_8;
        break;
    default:
        break;
    }
}

