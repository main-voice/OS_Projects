#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "QLabel"
#include "QLCDNumber"
#include "QListWidget"
#include "QTableWidget"
#include "QScreen"
#include "QHeaderView"
#include "QDebug"
#include "QTime"

#include <vector>
#include <queue>
#include <iostream>

// ȱҳ����������ȱҳ��ʵ������ȱ�ĸ�ҳ�е�ĳһ�����ݻ���ָ����ݴ���ڴ�СΪ10��ҳ����
// ������Ҫ����������ָ�����Ҫ�����ҳ���ڴ��в���
// ������ڣ��ǾʹӴ����е�ҳ���ŵ��ڴ��У�����ڴ��п���֡���Ǿ�ֱ�ӷ���
// ���û�У��Ǿͱ�����һ���滻������ֱ���FIFO�㷨��LRU�㷨
// 

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


#ifndef GlobalVar
#define GlobalVar

const int gSizeFrame = 4;
const int gSizeInst = 320;
const int gCapacityPage = 10; // ÿ��ҳ�������Ϊ10����������10��ָ��
#define _DEBUG_ false 
#endif // !GlobalVar


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum AlgoType { LRU, FIFO };

    MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow() {
        for (auto& lcd : vLcd) {
            if (lcd) {
                delete lcd;
            }
        }
        for (auto& block : vBlockList) {
            if (block) {
                delete block;
            }
        }
        delete instListAll;
        delete instListFault;
        delete logList;
        delete qPageTable;
    }

    void refreshDPI(); //ˢ��dpi

    void changeObjectSize(const QObject& o, double objectRate); //�޸����пؼ��ߴ�
    //�޸������С���Զ�����
    void changeFontSize(const QObject& o, const QFont& font);

    void initUI();
    void initData();
    void initConnect();
    void recoverUI();

    void instOrderGenerate();
    void instOrderShow();

    void oneStep();
    void tenStep();
    void toEnd();

    void AlgoLRU(int inst);
    void AlgoFIFO(int inst);

    //ָ��inst��Ҫ����idΪblockid���ڴ���
    void setBlock(int blockID, int inst); 

    void updateLog(int blockID, int inst); //������־��Ϣ

    void updatePageTable(int row, QTableWidgetItem* newItem); // ����ҳ��

    //����ȱҳָ����ȱҳ��
    void updateFault(int inst); 
private:
    Ui::MainWindowClass ui;

    AlgoType algo;

//UIWidget, ���������Ϣ�����������Ǿ�̬�����ֱ����uiͼ�л���
    std::vector<QLCDNumber*> vLcd;  // ��ʾ��ǰ�ڴ������Ӧҳ��ҳ��
    std::vector<QListWidget*> vBlockList;  // ��ʾ��ǰ�ڴ�����ָ�����ʷ
    std::vector<QLabel*> vLabelFrame;  // ��ʾ��ǰ�ڴ����ҳ�ŵ���ʷ

    QListWidget* instListAll;    // ���еĴ�����ָ������
    QListWidget* instListFault;  // ����ȱҳ��ָ������
    QListWidget* logList;        // ���з�����Ϣ

    QTableWidget* qPageTable;     // һ��ҳ��
//UIWidget

//data and structure
    //AlgoType algoType;//�㷨����
    std::vector<int> instArray;  //ָ������
    std::vector<int> block;      //��¼���ڴ�鵱ǰ����ҳ

    int sizeFrame;  // �ڴ���С
    int sizeInst;   // ָ������
    int capacityPage; // ÿ��ҳ�ܴ�ŵ�ָ������

    int currentInst;    //��ǰָ���Ӧ��ţ����Ǽ���ָ��˳���ţ���ô������10��Ϊ��λ˳�����һ��ҳ��
    int pageFaultCount; //ȱҳ����

    std::vector<int> pageTable;  //ҳ���м�¼��ҳ(1-32�߼�ҳ)��Ӧ�������ڴ�ҳ��
    std::queue<int> queueFIFO;   //��¼FIFO�㷨�и��ڴ�����ȼ�����Ӧ����Ӧҳ�Ĵ���ʱ��
    std::vector<int> priorityLRU;    //��¼LRU �㷨�и��ڴ�����ȼ�����Ӧ��δ��ʹ��ʱ��
    
//data and structure
};
