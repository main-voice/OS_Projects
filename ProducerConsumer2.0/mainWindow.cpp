#include "mainWindow.h"

//ui settings and init
mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    IR = gIR.get(); // 将全局物品库赋值给当前物品库

    initIR_UI();

    this->ui = new Ui::mainWindowClass;
    ui->setupUi(this);

    initWindow_UI();

    initConnect();
}

mainWindow::~mainWindow()
{
    delete ui;

    for (auto& x : bufferIndexLabel) {
        if (x) {
            delete x;
        }
    }
    for (auto& y : bufferInfo) {
        if (y) {
            delete y;
        }
    }
}

void mainWindow::initIR_UI()
{
    bufferIndexLabel.resize(BUFFER_SIZE);
    bufferInfo.resize(BUFFER_SIZE);
    vProducer.resize(gThreadNum / 2);
    vConsumer.resize(gThreadNum / 2);

    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        bufferIndexLabel[i] = new QLabel(QString::number(i + 1));
        bufferInfo[i] = new QPlainTextEdit("0", this);

        IR->buffer[i] = 0;
    }

    for (size_t i = 0; i < gThreadNum / 2; i++)
    {
        vProducer[i] = new Producer(101 + i);
        vConsumer[i] = new Consumer(201 + i);
    }
}

void mainWindow::initWindow_UI()
{

    this->setWindowTitle("Producer and Consumer 2.0");

    this->resize(1280, 720);

    //detail text editer start
    detailBtn.setParent(this);
    detailBtn.setText("Details");
    detailBtn.move(70, 360);
    detailBtn.resize(120, 40);

    detailInfo.setParent(this);
    detailInfo.move(70, 410);
    detailInfo.resize(700, 300);
    detailInfo.setReadOnly(true);
    //detail text editer end


    //start btn start
    startBtn.setParent(this);
    startBtn.setText("Start");
    startBtn.resize(100, 40);
    startBtn.move(944, 20);
    //start btn end


    //end btn start
    endBtn.setParent(this);
    endBtn.setText("End");
    endBtn.resize(100, 40);
    endBtn.move(944, 70); // 数字仅为排位置，无特殊含义
    //end btn end


    //buffer btn start
    bufferBtn.setParent(this);
    bufferBtn.setText("Buffer");
    bufferBtn.resize(160, 40);
    bufferBtn.move(70, 22);
    //buffer btn end


    //buffer info start
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {

        bufferIndexLabel[i]->setParent(this);
        bufferIndexLabel[i]->resize(20, 20);
        bufferIndexLabel[i]->move(70 + i * 40, 70);

        bufferInfo[i]->resize(40, 40);
        bufferInfo[i]->move(70 + 40 * i, 100);
        bufferInfo[i]->setReadOnly(true);


    }
    //buffer info end

    {
        //cur producer button and info start
        curProducerBtn.setParent(this);
        curProducerBtn.setText("cur producer");
        curProducerBtn.resize(180, 70);
        curProducerBtn.move(60, 200);

        curProducerInfo.setParent(this);
        curProducerInfo.move(230, 200);
        curProducerInfo.resize(230, 110);
        curProducerInfo.setReadOnly(true);
        curProducerInfo.appendPlainText("0");
        //cur producer button and info end
    }

    //cur consumer button and info start
    curConsumerBtn.setParent(this);
    curConsumerBtn.setText("cur consumer");
    curConsumerBtn.resize(190, 70);
    curConsumerBtn.move(490, 200);

    curConsumerInfo.setParent(this);
    curConsumerInfo.move(680, 200);
    curConsumerInfo.resize(230, 110);
    curConsumerInfo.setReadOnly(true);
    curConsumerInfo.appendPlainText("0");
    //cur consumer button and info end


}

void mainWindow::initConnect()
{
    // consumer and producer
    for (size_t i = 0; i < gThreadNum / 2; i++)
    {
        // details info
        //将消费者发出的update函数发送到本窗口，本窗口接受后更新
        connect(vConsumer[i], SIGNAL(updateIRInfo(QString)), this, SLOT(receiveAllInfo(QString)));

        connect(vProducer[i], SIGNAL(updateIRInfo(QString)), this, SLOT(receiveAllInfo(QString)));
        // details info end

        //consumer and producer cache update
        connect(vConsumer[i], SIGNAL(updateCurConsumerCacheText(QString, QString, int, int)), this, SLOT(receiveCurConsumerCacheText(QString, QString, int, int)));

        connect(vProducer[i], SIGNAL(updateCurProducerCacheText(QString, QString, int, int)), this, SLOT(receiveCurProducerCacheText(QString, QString, int, int)));
        //consumer and producer cache update end


        //end this thread ?
        connect(vConsumer[i], SIGNAL(end()), this, SLOT(threadEnd()));
        connect(vProducer[i], SIGNAL(end()), this, SLOT(threadEnd()));
        //end
    }

    //connect button
    connect(&startBtn, SIGNAL(clicked()), this, SLOT(appStart()));
    connect(&endBtn, SIGNAL(clicked()), this, SLOT(close()));
}

void mainWindow::receiveAllInfo(const QString& text)
{
    detailInfo.appendPlainText(text);
}

void mainWindow::appStart()
{
    startBtn.setDisabled(true);
    endBtn.setDisabled(true);

    //strat the producers and consumers
    for (const auto& p : vProducer) {
        p->start(); // because producer is derived from QThread
    }
    for (const auto& c : vConsumer) {
        c->start();
    }
}


void mainWindow::threadEnd()
{
    gVaildThreadNum--;
    if (gVaildThreadNum == 0) {
        endBtn.setDisabled(false);
    }
}

void mainWindow::receiveCurProducerCacheText(const QString& P_ID, const QString& T_ID, int number, int location)
{
    bufferInfo[location]->clear(); // clear old data
    bufferInfo[location]->appendPlainText(QString::number(number));

    curProducerInfo.clear();
    curProducerInfo.appendPlainText("Producer ID: " + P_ID);
    curProducerInfo.appendPlainText("Thread ID: " + T_ID);
    curProducerInfo.appendPlainText("produce_item: " + QString::number(number));
    curProducerInfo.appendPlainText("put_in_position: " + QString::number(location + 1));
}


void mainWindow::receiveCurConsumerCacheText(const QString& C_ID, const QString& T_ID, int number, int location)
{
    bufferInfo[location]->clear(); // clear old data
    bufferInfo[location]->appendPlainText("-1");

    curConsumerInfo.clear();
    curConsumerInfo.appendPlainText("Consumer ID: " + C_ID);
    curConsumerInfo.appendPlainText("Thread ID: " + T_ID);
    curConsumerInfo.appendPlainText("consume_item: " + QString::number(number));
    curConsumerInfo.appendPlainText("from_position: " + QString::number(location + 1));
}

