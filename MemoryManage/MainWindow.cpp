#include "MainWindow.h"

// 延时函数
// 在更新指令状态时，需要处理一些跟界面无关的但非常耗时的事情，如10次指令访问操作，这些事情跟界面在同一个线程中，由于时间太长，导致界面无法响应，处于“假死”状态。QApplication::processEvents()。该函数的作用是让程序处理那些还没有处理的事件，然后再把使用权返回给调用者。
void delay(int msec = 10)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initData();

    initUI();

    initConnect();
}


void MainWindow::initUI()
{
    ui.setupUi(this);
    this->setWindowTitle("Memory Management");
    int winWidth = sizeFrame > 7 ? (sizeFrame * (1000 / 7)) : 1000;
    this->resize(winWidth, 600); // 经检验，1000个像素大小刚好为7个控制块
    //this->resize(sizeFrame * (1000 / 7), 600); // 经检验，1000个像素大小刚好为7个控制块
    vLcd.resize(sizeFrame);
    vBlockList.resize(sizeFrame);
    vLabelFrame.resize(sizeFrame);

    for (size_t i = 0; i < vLabelFrame.size(); i++)
    {
        vLabelFrame[i] = new QLabel("Frame" + QString::number(i), this);
        vLabelFrame[i]->move(20 + i * 140, 40);
        vLabelFrame[i]->resize(70, 20);
        vLabelFrame[i]->adjustSize();
        vLabelFrame[i]->setWordWrap(true); 
        
       /* QFont changedFont = vLabelFrame[i]->font();
        changedFont.setPointSize(9);
        vLabelFrame[i]->setFont(changedFont);*/
    }

    for (size_t i = 0; i < vLcd.size(); i++)
    {
        vLcd[i] = new QLCDNumber(2, this); // 第一个参数为该LCD显示的位数，必须大于0才可，不是里面显示的数字内容
        vLcd[i]->setMode(QLCDNumber::Dec);
        vLcd[i]->move(10 + i * 140, 70);
        vLcd[i]->resize(60, 60);
        vLcd[i]->display(-1);
    }

    for (size_t i = 0; i < vBlockList.size(); i++)
    {
        vBlockList[i] = new QListWidget(this);
        vBlockList[i]->move(80 + i * 140, 20);
        vBlockList[i]->resize(60, 150);
    }

    { // 输出详细指令信息初始化UI
        instListAll = new QListWidget(this);
        instListAll->move(30, 220);
        instListAll->resize(100, 200);

        instListFault = new QListWidget(this);
        instListFault->move(180, 220);
        instListFault->resize(100, 200);

        logList = new QListWidget(this);
        logList->move(520, 220);
        logList->resize(250, 320);
    }

    qPageTable = new QTableWidget(sizeFrame + 1, 2, this);
    qPageTable->move(800, 220);
    qPageTable->resize(150, 320);
    QHeaderView* headerView = qPageTable->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::Stretch);
    qPageTable->verticalHeader()->hide(); //去除行号显示
    qPageTable->setItem(0, 0, new QTableWidgetItem(u8"逻辑页号"));
    qPageTable->setItem(0, 1, new QTableWidgetItem(u8"内存块号"));
    qPageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //使表格只读

    for (int i = 1; i <= qPageTable->rowCount(); i++) {
        qPageTable->setItem(i, 0, new QTableWidgetItem(QString::number(i - 1)));
        qPageTable->setItem(i, 1, new QTableWidgetItem(QString::number(-1)));
    }


    //接下来初始化UI内容，清空列表
    instListAll->clear();
    instListFault->clear();
    logList->clear();
    for (auto& block : vBlockList) {
        block->addItem(QString(u8"访问记录"));
    }
    ui.label_inst->setText("0");
    ui.label_faults->setText("0");
    ui.label_rate->setText("\\");
    
    //按钮可见性
    ui.label_choose->setVisible(false);
    ui.btn_FIFO->setVisible(false);
    ui.btn_LRU->setVisible(false);
    ui.btn_oneStep->setVisible(false);
    ui.btn_tenStep->setVisible(false);
    ui.btn_toEnd->setVisible(false);
    ui.btn_FIFO->setStyleSheet("background-color: rgb(255,255,255); ");
    ui.btn_LRU->setStyleSheet("background-color: rgb(255,255, 255); ");
    ui.btn_FIFO->setEnabled(true);
    ui.btn_LRU->setEnabled(true);

    refreshDPI(); //先把所有UI界面放好，然后根据dpi调整窗体大小

}

void MainWindow::initData()
{
    sizeFrame = gSizeFrame;
    sizeInst = gSizeInst;
    capacityPage = gCapacityPage;

    instArray.resize(sizeInst); // 指令大小初始化
    block.resize(sizeFrame);    // 内存块数量初始化
    
    priorityLRU.resize(sizeFrame);  // 各内存块优先级

    pageTable.resize(sizeInst / capacityPage); // 页表数目，为一级页表

    currentInst = 0;
    pageFaultCount = 0;

    // 指令地址初始化，也就是逻辑地址，我们假设指令顺序存放，那么就是以10个为单位顺序存放在一个页中
    for (size_t i = 0; i < static_cast<size_t>(sizeInst); i++) 
    {
        instArray[i] = i;
    }


    // 内存块尚未存放任何逻辑页面
    for (size_t i = 0; i < block.size(); i++)
    {
        block[i] = -1;
    }

    //页表内容初始化
    for (int i = 0; i < pageTable.size(); i++) 
    {
        this->pageTable[i] = -1;
    }

    for (size_t i = 0; i < priorityLRU.size(); i++)
    {
        priorityLRU[i] = 0;
    }
}

void MainWindow::initConnect()
{
    connect(ui.btn_start, &QPushButton::clicked, this,
        [=] {
            //recoverUI();
            instOrderGenerate();
            instOrderShow();

            ui.label_choose->setVisible(true);
            ui.btn_FIFO->setVisible(true);
            ui.btn_LRU->setVisible(true);
            //ui.btn_start->setText(u8"重新开始");

            ui.btn_LRU->setEnabled(true);
            ui.btn_FIFO->setEnabled(true);
        }
    );

    connect(ui.btn_FIFO, &QPushButton::clicked, this,
        [=] {
            ui.btn_FIFO->setStyleSheet("background-color : rgb(56, 184, 41); ");
            ui.btn_LRU->setEnabled(false);
            this->algo = FIFO;
            ui.btn_oneStep->setVisible(true);
            ui.btn_tenStep->setVisible(true);
            ui.btn_toEnd->setVisible(true);

            ui.btn_oneStep->setEnabled(true);
            ui.btn_tenStep->setEnabled(true);
            ui.btn_toEnd->setEnabled(true);
        });

    connect(ui.btn_LRU, &QPushButton::clicked, this,[=]{
            ui.btn_LRU->setStyleSheet("background-color : rgb(56, 184, 41); ");
            ui.btn_LRU->setEnabled(false);
            this->algo = LRU;
            ui.btn_oneStep->setVisible(true);
            ui.btn_tenStep->setVisible(true);
            ui.btn_toEnd->setVisible(true);

            ui.btn_oneStep->setEnabled(true);
            ui.btn_tenStep->setEnabled(true);
            ui.btn_toEnd->setEnabled(true);
        });

    connect(ui.btn_oneStep, &QPushButton::clicked, this, &MainWindow::oneStep);
    connect(ui.btn_tenStep, &QPushButton::clicked, this, &MainWindow::tenStep);
    connect(ui.btn_toEnd, &QPushButton::clicked, this, &MainWindow::toEnd);

    connect(ui.btn_end, &QPushButton::clicked, this, &MainWindow::close);
}

void MainWindow::recoverUI()
{

    for (size_t i = 0; i < vLcd.size(); i++)
    {
        vLcd[i]->display(-1);
    }

    for (size_t i = 0; i < vBlockList.size(); i++)
    {
        vBlockList[i]->clear();
    }

    for (int i = 1; i <= qPageTable->rowCount(); i++) {
        qPageTable->setItem(i, 0, new QTableWidgetItem(QString::number(i - 1)));
        qPageTable->setItem(i, 1, new QTableWidgetItem(QString::number(-1)));
    }


    //接下来初始化UI内容，清空列表
    instListAll->clear();
    instListFault->clear();
    logList->clear();
    for (auto& block : vBlockList) {
        block->addItem(QString(u8"访问记录"));
    }
    ui.label_inst->setText("0");
    ui.label_faults->setText("0");
    ui.label_rate->setText("\\");

    //按钮可见性
    ui.label_choose->setVisible(false);

    ui.btn_FIFO->setVisible(false);
    ui.btn_LRU->setVisible(false);

    ui.btn_oneStep->setVisible(false);
    ui.btn_tenStep->setVisible(false);
    ui.btn_toEnd->setVisible(false);

    ui.btn_FIFO->setEnabled(true);
    ui.btn_LRU->setEnabled(true);

    ui.btn_FIFO->setStyleSheet("background-color: rgb (255, 255, 255);");
    ui.btn_LRU->setStyleSheet("background-color: rgb (255, 255,255); ");
}

// 产生指令访问次序
void MainWindow::instOrderGenerate()
{
    srand(time(NULL));
    int m = rand() % (sizeInst); // [0, sizeInst)

    int count = sizeInst / 4; // 一共320条指令，一次循环生成4条指令，一共循环80次，该处4与内存块数量无关，取决于指令生成算法
    for (size_t i = 0; i < count; i++)
    {
        while (m == 0) {
            m = rand() % sizeInst; // 如果m是0，后续取余不方便
        }

        int m1 = rand() % (m);

        instArray[i * 4] = m1;
        instArray[i * 4 + 1] = m1 + 1;

        int m2 = (rand() % (sizeInst - 1 - m1)) + m1 + 2;

        instArray[i * 4 + 2] = m2;
        instArray[i * 4 + 3] = m2 + 1;

        m = m1;
    }

#if _DEBUG_
    instArray.clear();
    instArray.resize(20);
    instArray = { 70,0,10,20,0,30,0,40,20,30,0,30,20,10,20,0,10,70,0,10 };
#endif // _DEBUG_

}

void MainWindow::instOrderShow()
{
#if _DEBUG_
    for (auto& x : instArray) {
        qDebug() << x;
    }
#endif

    for (size_t i = 0; i < sizeInst; i++)
    {
        QString inst = QString("%1").arg(instArray[i], 3, 10, QChar('0'));
        QString instPage = QString::number(instArray[i] / capacityPage);

        instListAll->addItem(inst + " (" + instPage + ")");
    }
}

void MainWindow::oneStep()
{
    //指令已经访问完
    if (currentInst == sizeInst) {
        ui.btn_FIFO->setEnabled(false);
        ui.btn_LRU->setEnabled(false);
        ui.btn_start->setEnabled(false);
        ui.btn_oneStep->setEnabled(false);
        ui.btn_tenStep->setEnabled(false);
        ui.btn_toEnd->setEnabled(false);
        return;
    }

    ui.label_inst->setText(QString::number(currentInst));
    instListAll->setCurrentRow(currentInst);

    if (algo == FIFO) {
#if _DEBUG_
        qDebug() << currentInst << instArray[currentInst];
#endif // _DEBUG_

        AlgoFIFO(instArray[currentInst++]);
    }
    else if (algo == LRU) {
        AlgoLRU(instArray[currentInst++]);
#if _DEBUG_
        for (auto& x : priorityLRU) {
            std::cout << x << " ";
        }
        std::cout<<"\n";
#endif // _DEBUG_

    }
    else {

    }

    float faultRate = float(pageFaultCount) / float(currentInst);
    ui.label_inst->setText(QString::number(currentInst));
    ui.label_rate->setText(QString::number(faultRate));
}

void MainWindow::tenStep()
{
    ui.btn_start->setEnabled(false);
    ui.btn_oneStep->setEnabled(false);
    ui.btn_toEnd->setEnabled(false);

    for (size_t i = 0; i < 10 && currentInst < sizeInst; i++)
    {
        oneStep();
        delay();
    }

    ui.btn_start->setEnabled(true);
    ui.btn_oneStep->setEnabled(true);
    ui.btn_toEnd->setEnabled(true);

    //指令已经访问完
    if (currentInst == sizeInst) {
        ui.btn_FIFO->setEnabled(false);
        ui.btn_LRU->setEnabled(false);
        ui.btn_start->setEnabled(false);
        ui.btn_oneStep->setEnabled(false);
        ui.btn_tenStep->setEnabled(false);
        ui.btn_toEnd->setEnabled(false);
        return;
    }
}

void MainWindow::toEnd()
{
    ui.btn_start->setEnabled(false);
    ui.btn_oneStep->setEnabled(false);
    ui.btn_tenStep->setEnabled(false);

    while (currentInst < sizeInst)
    {
        oneStep();
        delay();
    }

    ui.btn_start->setEnabled(true);
    ui.btn_oneStep->setEnabled(true);
    ui.btn_tenStep->setEnabled(true);

    //指令已经访问完
    if (currentInst == sizeInst) {
        ui.btn_FIFO->setEnabled(false);
        ui.btn_LRU->setEnabled(false);
        ui.btn_start->setEnabled(false);
        ui.btn_oneStep->setEnabled(false);
        ui.btn_tenStep->setEnabled(false);
        ui.btn_toEnd->setEnabled(false);
        return;
    }
}

void MainWindow::AlgoLRU(int inst)
{
    for (size_t i = 0; i < sizeFrame; i++)
    {
        if (block[i] == inst / capacityPage) {
            qDebug() << inst << "is in block " << i;
            this->priorityLRU[i] = 0;

            qDebug() << "priorityLRU[i]" << " is " << priorityLRU[i];


            for (size_t j = 0; j < sizeFrame; j++)
            {
                if (j != i && block[j] != -1) {
                    priorityLRU[j]++;
                }
            }
            setBlock(i, inst); // 当前指令已在内存，设置
            return;
        }
    }


    // 下面缺页，分两种情况，需要换出与不需要
    updateFault(inst);

    // 刚开始有空闲内存
    for (size_t i = 0; i < sizeFrame; i++)
    {
        if (block[i] == -1) {
            setBlock(i, inst);
            this->priorityLRU[i] = 0;

            for (size_t j = 0; j < sizeFrame; j++)
            {
                if (j != i && block[j] != -1) {
                    priorityLRU[j]++;
                }
            }
            return;
        }
    }

    //缺页中断，换出最近未被访问内存
    int maxIndex = 0;
    int maxTime = -1;
    for (int i = 0; i < sizeFrame; i++) {
        if (priorityLRU[i] > maxTime) {
            maxTime = priorityLRU[i];
            maxIndex = i;
        }
    }

    setBlock(maxIndex, inst);

    for (size_t i = 0; i < sizeFrame; i++)
    {
        priorityLRU[i]++;
    }
    priorityLRU[maxIndex] = 0;

}

void MainWindow::AlgoFIFO(int inst)
{
    for (size_t i = 0; i < sizeFrame; i++)
    {
        if (block[i] == inst / capacityPage) {
            setBlock(i, inst); // 当前指令已在内存，设置
            return;
        }
    }

    // 下面缺页，分两种情况，需要换出与不需要
    updateFault(inst);
    
    // 刚开始有空闲内存
    for (size_t i = 0; i < sizeFrame; i++)
    {
        if (block[i] == -1) {
#if _DEBUG_
            qDebug() << "free memory : block[" << i << "]";
#endif // _DEBUG_

            setBlock(i, inst);
#if _DEBUG_
            qDebug() << "the " << i << " -th block enter queue";
#endif // _DEBUG_
            queueFIFO.push(i);
            return;
        }
    }

    // 之后需要替换某一个
    if (queueFIFO.size() == sizeFrame) {
#if _DEBUG_
        qDebug() << "queue is full, pop the " << queueFIFO.front() << " -th block's page";
#endif // _DEBUG_

        int freeBlockID = queueFIFO.front();
        queueFIFO.pop();
        setBlock(freeBlockID, inst);
        queueFIFO.push(freeBlockID);
    }
}

void MainWindow::setBlock(int blockID, int inst)
{
    updateLog(blockID, inst);
    // 不是目标页，需要替换出去
    int instPage = inst / capacityPage;
    if (instPage != block[blockID]) {
        block[blockID] = instPage;
        vBlockList[blockID]->clear(); // 清除原有界面中的指令信息，实际中需要把这块复制到磁盘里面
        vBlockList[blockID]->addItem(QString(u8"访问记录"));
        vLcd[blockID]->display(instPage);
    }

    QString accessInfo = QString("%1").arg(inst, 3, 10, QChar('0'));
    vBlockList[blockID]->addItem(accessInfo);
}

void MainWindow::updateLog(int blockID, int inst)
{
    QString newPage = QString::number(inst / capacityPage);
    QString LogString = u8"访问指令" + QString("%1").arg(inst, 3, 10, QChar('0'));
    LogString += " (";
    LogString += newPage;
    LogString += "). ";

    int instPage = inst / capacityPage;
    if (instPage == block[blockID]) {
        LogString += u8"在Frame" + QString::number(block[blockID]) + u8"中";
    }
    else {
        LogString += u8"缺页中断. ";
        if (block[blockID] != -1) {
            LogString += "Frame" + QString::number(blockID) + u8"换出页" + QString::number(block[blockID]) + u8", 装入页" + newPage;
            updatePageTable(block[blockID] + 1, new QTableWidgetItem(QString::number(-1)));
        }
        else {
            LogString += "Frame" + QString::number(blockID) + u8"装入页" + newPage;
        }
#if _DEBUG_
        qDebug() << "blockid is : " << blockID;
#endif // _DEBUG_

        updatePageTable(inst / capacityPage + 1,new QTableWidgetItem(QString::number(blockID)));
    }

    logList->addItem(LogString);
    logList->setCurrentRow(currentInst);
}

void MainWindow::updatePageTable(int row, QTableWidgetItem* newItem)
{
    qPageTable->setItem(row, 1, newItem);
}

void MainWindow::updateFault(int inst)
{
    // 工作：缺页输出列表更新，缺页数量更新，当前缺页更新
    QString faultInst = QString("%1").arg(inst, 3, 10, QChar('0'));
    QString faultPage = QString::number(inst / 10);

    instListFault->addItem(faultInst + " (" + faultPage + ")");

    this->pageFaultCount++;
    instListFault->setCurrentRow(pageFaultCount - 1);

    ui.label_faults->setText(QString::number(pageFaultCount));
    
}


//刷新dpi
void MainWindow::refreshDPI()
{
    //计算dpi
    QList<QScreen*> screens = QApplication::screens();
    QScreen* screen = screens[0];
    qreal dpi = screen->logicalDotsPerInch();
    //计算dpi对应的缩放比例
    double objectRate = dpi / 96.0;
    changeObjectSize(*this, objectRate);
    //qDebug() << "width " << width() << "height " << height();
    resize(width() * objectRate, height() * objectRate);

    //上面只是将组件的大小与电脑分辨率缩放对应起来
    //还需要调整Label中的字体大小，防止电脑缩放分辨率后，组件变了，但是字体大小不变
    //pointSize是与电脑分辨率以及缩放有关的，设置后即可
    //pixelSize只与像素多少有关，如果使用该方法，还需手动控制缩放
    QFont changedFont{};
    changedFont.setPointSize(10);
    changeFontSize(*this, changedFont);

}

//修改所有控件尺寸
void MainWindow::changeObjectSize(const QObject& o, double objectRate)
{

    for (int i = 0; i < o.children().size(); ++i) {

        QWidget* pWidget = qobject_cast<QWidget*>(o.children().at(i));
        if (pWidget != nullptr) {

            //qDebug() << pWidget->width() << pWidget->height();
            //pWidget->resize(pWidget->width()*objectRate, pWidget->height()*objectRate);
            pWidget->setGeometry(pWidget->x() * objectRate, pWidget->y() * objectRate,
                pWidget->width() * objectRate, pWidget->height() * objectRate);
            changeObjectSize(*(o.children().at(i)), objectRate);
        }
    }
}

void MainWindow::changeFontSize(const QObject& o, const QFont& font)
{
    for (size_t i = 0; i < o.children().size(); i++)
    {
        QWidget* pWidget = qobject_cast<QWidget*>(o.children().at(i));
        if (pWidget != nullptr) {
            pWidget->setFont(font);
            changeFontSize(*o.children().at(i), font);
        }
    }
}

