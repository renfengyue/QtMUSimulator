#include "muview.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MUView::MUView(MUPageIndex index)
{
    m_MUPageIndex = index;
}

MUView::~MUView()
{

}

void MUView::InitializeTable()
{
    m_pTable->setRowCount(8);
    m_pTable->setColumnCount(2);
    QStringList list;
    list<<"Value"<<"Degree";
    m_pTable->setHorizontalHeaderLabels(list);

    m_pTable->setItem(0,0,new QTableWidgetItem());
    m_pTable->setItem(0,1,new QTableWidgetItem());
    m_pTable->setItem(1,0,new QTableWidgetItem());
    m_pTable->setItem(1,1,new QTableWidgetItem());
    m_pTable->setItem(2,0,new QTableWidgetItem());
    m_pTable->setItem(2,1,new QTableWidgetItem());
    m_pTable->setItem(3,0,new QTableWidgetItem());
    m_pTable->setItem(3,1,new QTableWidgetItem());
    m_pTable->setItem(4,0,new QTableWidgetItem());
    m_pTable->setItem(4,1,new QTableWidgetItem());
    m_pTable->setItem(5,0,new QTableWidgetItem());
    m_pTable->setItem(5,1,new QTableWidgetItem());
    m_pTable->setItem(6,0,new QTableWidgetItem());
    m_pTable->setItem(6,1,new QTableWidgetItem());
    m_pTable->setItem(7,0,new QTableWidgetItem());
    m_pTable->setItem(7,1,new QTableWidgetItem());
    m_pTable->setItem(8,0,new QTableWidgetItem());
    m_pTable->setItem(8,1,new QTableWidgetItem());


    QStringList verticallist;
    verticallist<<"V1"<<"V2"<<"V3"<<"V4"<<"I1"<<"I2"<<"I3"<<"I4";
    m_pTable->setVerticalHeaderLabels(verticallist);
}


