#ifndef MUVIEW_H
#define MUVIEW_H
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <QComboBox>

#include "globaltype.h"

class MUView
{
public:
    MUView(MUPageIndex index);
    ~MUView();
    void InitializeTable();

    MUPageIndex m_MUPageIndex;
    QTableWidget* m_pTable;
    QLineEdit* m_pSvId;
    QCheckBox* m_pInvalid;
    QCheckBox* m_pTest;
    QCheckBox* m_pQues;
    QLineEdit* m_pCTr;
    QLineEdit* m_pVTr;
    QComboBox* m_pSyncMode;
    QCheckBox* m_pSimul;
};

#endif // MUVIEW_H
