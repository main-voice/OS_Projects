#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainWindow.h"
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>

#include "Producer.h"
#include "Consumer.h"
#include "GlobalHead.h"


class mainWindow : public QMainWindow
{
    Q_OBJECT


private:

    QPushButton  startBtn, endBtn;

    //buffer
    QPushButton bufferBtn;
    std::vector<QLabel*> bufferIndexLabel;
    std::vector<QPlainTextEdit*> bufferInfo;
    
    //consumer and producer
    QPushButton  curProducerBtn, curConsumerBtn;
    QPlainTextEdit curProducerInfo, curConsumerInfo;

    //details
    QPushButton  detailBtn;
    QPlainTextEdit detailInfo;

    //only one repository
    ItemRepo* IR;
public:
    mainWindow(QWidget *parent = Q_NULLPTR);
    ~mainWindow();

    //my define start
    std::vector<Producer*> vProducer;
    std::vector<Consumer*> vConsumer;

    //init the ui of IR(buffer)
    void initIR_UI();

    //init the ui of window
    void initWindow_UI();

    //init the connection between consumers/producers and text
    //init the connection between button and click
    void initConnect();
    //my define end


protected slots://自定义槽函数
    void receiveAllInfo(const QString& text);//在大窗口出现结果
    void appStart();//用于按钮启动
    void threadEnd();//用于结束一个线程
    void receiveCurProducerCacheText(const QString& P_ID, const QString& T_ID, int number, int location);//对应生产者缓冲区文本, 参数：线程id，当前物品的数字，当前物品的位置
    void receiveCurConsumerCacheText(const QString& C_ID, const QString& T_ID, int number, int location);//对应下方缓冲区文本
private:
    Ui::mainWindowClass* ui;
};
