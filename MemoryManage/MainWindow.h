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

// 缺页，表面上是缺页，实际上是缺的该页中的某一个数据或者指令。数据存放在大小为10的页面中
// 所以想要访问数据与指令，必须要有这个页在内存中才行
// 如果不在，那就从磁盘中调页，放到内存中，如果内存有空闲帧，那就直接放上
// 如果没有，那就必须找一个替换，这里分别有FIFO算法和LRU算法
// 

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


#ifndef GlobalVar
#define GlobalVar

const int gSizeFrame = 4;
const int gSizeInst = 320;
const int gCapacityPage = 10; // 每个页面的容量为10，可以容纳10个指令
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

    void refreshDPI(); //刷新dpi

    void changeObjectSize(const QObject& o, double objectRate); //修改所有控件尺寸
    //修改字体大小，自动缩放
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

    //指令inst将要进入id为blockid的内存中
    void setBlock(int blockID, int inst); 

    void updateLog(int blockID, int inst); //更新日志信息

    void updatePageTable(int row, QTableWidgetItem* newItem); // 更新页表

    //更新缺页指令与缺页率
    void updateFault(int inst); 
private:
    Ui::MainWindowClass ui;

    AlgoType algo;

//UIWidget, 各种输出信息的组件，如果是静态的组件直接在ui图中画好
    std::vector<QLCDNumber*> vLcd;  // 显示当前内存块所对应页的页号
    std::vector<QListWidget*> vBlockList;  // 显示当前内存块访问指令的历史
    std::vector<QLabel*> vLabelFrame;  // 显示当前内存块存放页号的历史

    QListWidget* instListAll;    // 所有的待访问指令序列
    QListWidget* instListFault;  // 发生缺页的指令序列
    QListWidget* logList;        // 所有访问信息

    QTableWidget* qPageTable;     // 一级页表
//UIWidget

//data and structure
    //AlgoType algoType;//算法类型
    std::vector<int> instArray;  //指令数组
    std::vector<int> block;      //记录各内存块当前所存页

    int sizeFrame;  // 内存块大小
    int sizeInst;   // 指令数量
    int capacityPage; // 每个页能存放的指令数量

    int currentInst;    //当前指令对应序号，我们假设指令顺序存放，那么就是以10个为单位顺序存在一个页中
    int pageFaultCount; //缺页数量

    std::vector<int> pageTable;  //页表中记录各页(1-32逻辑页)对应的物理内存页号
    std::queue<int> queueFIFO;   //记录FIFO算法中各内存块优先级，对应于相应页的存入时间
    std::vector<int> priorityLRU;    //记录LRU 算法中各内存块优先级，对应于未被使用时间
    
//data and structure
};
