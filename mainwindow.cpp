#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    initDatabase();

    initPlot();

    //表格类初始化
    initTableWidget();

    initTimer();

    initConnections();
}

void MainWindow::initDatabase()
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
}

void MainWindow::initPlot()
{
    ui->CustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    //设置基本坐标轴（左侧Y轴和下方X轴）可拖动、可缩放、曲线可选、legend可选、设置伸缩比例，使所有图例可见
    ui->CustomPlot_2->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom| QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->CustomPlot_2->legend->setSelectableParts(QCPLegend::spItems);
    connect(ui->CustomPlot_2, &QCustomPlot::selectionChangedByUser, this, &MainWindow::selectionChanged);

    //游标功能
    connect(ui->CustomPlot_2, SIGNAL(mouseMove(QMouseEvent*)), this,SLOT(showTracer(QMouseEvent*)));
    m_TracerY = QSharedPointer<CurveTracer> (new CurveTracer(ui->CustomPlot_2, ui->CustomPlot_2->graph(0), DataTracer));
    //m_TraserX = QSharedPointer<myTracer> (new myTracer(CustomPlot, CustomPlot->graph(0), XAxisTracer));
}

void MainWindow::initTableWidget()
{
    m_oneHourTableWidetModel = new QStandardItemModel();

    initOneHourUrineTableWidget();
}

void MainWindow::initTimer()
{
    //动态曲线计时器初始化
    m_DynamicCurveTimer = new QTimer(this);
    connect(m_DynamicCurveTimer,SIGNAL(timeout()),this,SLOT(slotTimeout()));
}



void MainWindow::initDatabase(QString path)
{
    //设置数据库驱动名称
    if (m_database.isOpen()) {
        m_database.close();
    }

    m_database.setDatabaseName(path);
    //看是否能正确打开
    if (!m_database.open())
    {
        //QDBG database.lastError().text();
        QDBG "数据库打开失败";
        return;
    }
    else {
        QDBG "数据库打开成功！";
    }
}

QSqlQuery MainWindow::readUserForm()
{
    QSqlQuery query(m_database);
    QString sqlstr = QString("SELECT * from User");
    query.exec(sqlstr);
    return query;
}

QSqlQuery MainWindow::readHourRecord()
{
    QSqlQuery query(m_database);
    QString sqlstr = QString("SELECT * from HourRecord");
    query.exec(sqlstr);
    return query;
}

QSqlQuery MainWindow::readUrinaryBagWeightRecord()
{
    QSqlQuery query(m_database);
    QString sqlstr = QString("SELECT * from Weight");
    query.exec(sqlstr);
    return query;
}

void MainWindow::drawOneHourUrineVolumeCurve(QString userIdStr)//绘制柱状图
{
    m_queryOneHourUrineVolume = readHourRecord();

    int i = 0;
    QVector<double> x, y;
    QVector<QString> labels;
    while (m_queryOneHourUrineVolume.next())
    {
        if(userIdStr == m_queryOneHourUrineVolume.value(1).toString()){
            x.append(i + 1);
            y.append(m_queryOneHourUrineVolume.value(4).toDouble());
            labels.append(m_queryOneHourUrineVolume.value(2).toString());
            i++;
        }
    }


    //在柱状图顶部添加数据,(绘制其他图表的时候不会清空前表的异常,[未修复])
    // for(int m = 0; m < x.size(); m++)
    // {
    //     //QCPItemText的效果类似于一个label
    //     QCPItemText *itemText = new QCPItemText(ui->CustomPlot);
    //     itemText->setClipToAxisRect(false);
    //     //customPlot->removeItem(itemText);
    //     //设置itemText的位置是跟随坐标系的
    //     itemText->position->setType(QCPItemPosition::ptPlotCoords);
    //     //设置itemText跟随的坐标系为 ui->customplot->xAxis, ui->customplot->yAxis
    //     itemText->position->setAxes(ui->CustomPlot->xAxis, ui->CustomPlot->yAxis);
    //     //设置itemText放置的位置
    //     itemText->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
    //     //设置itemText显示的内容
    //     itemText->setText(QString::number(y.at(m)));
    //     //设置itemText显示的位置的坐标
    //     itemText->position->setCoords(x.at(m), y.at(m));
    // }


    QCPAxis *xAxis = ui->CustomPlot->xAxis;
    QCPAxis *yAxis = ui->CustomPlot->yAxis;
    QCPBars *bars = new QCPBars(xAxis, yAxis);  // 使用xAxis作为柱状图的key轴，yAxis作为value轴

    bars->setAntialiased(false); // 为了更好的边框效果，关闭抗齿锯
    bars->setName("bars fuels"); // 设置柱状图的名字，可在图例中显示
    bars->setPen(QPen(QColor(0, 168, 140).lighter(130))); // 设置柱状图的边框颜色
    bars->setBrush(QColor(28, 168, 106, 50));  // 设置柱状图的画刷颜色


    // 为柱状图设置一个文字类型的key轴，ticks决定了轴的范围，而labels决定了轴的刻度文字的显示
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(x, labels);
    xAxis->setTicker(textTicker);        // 设置为文字轴
    xAxis->setTickLabelRotation(60);     // 轴刻度文字旋转60度
    xAxis->setSubTicks(false);           // 不显示子刻度
    xAxis->setTickLength(0, 4);          // 轴内外刻度的长度分别是0,4,也就是轴内的刻度线不显示
    xAxis->setRange(0, x.size() + 1);               // 设置范围
    xAxis->setLabel("x");
    xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);

    yAxis->setRange(0, ui->CustomPlot->height());
    yAxis->setPadding(35);             // 轴的内边距，可以到QCustomPlot之开始（一）看图解
    yAxis->setLabel("OneHourUrineVolume\nLexin Tec");
    yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    bars->setData(x, y);

}

void MainWindow::drawUrinaryBagWeight(QString userIdStr)//绘制折线图
{
    m_queryUrinaryBagWeightRecord = readUrinaryBagWeightRecord();

    int i = 0;
    QVector<double> x, y;
    QVector<QString> labels;
    int y_max = 0;
    while (m_queryUrinaryBagWeightRecord.next())
    {
        if("clearCrave" == userIdStr)break;
        if(userIdStr == m_queryUrinaryBagWeightRecord.value(1).toString()){
            x.append(i + 1);
            y.append(m_queryUrinaryBagWeightRecord.value(2).toDouble());
            if(y_max < m_queryUrinaryBagWeightRecord.value(2).toDouble()){
                y_max = m_queryUrinaryBagWeightRecord.value(2).toDouble();
            }

            QDateTime dateTime_half;
            QString time = QString("%1").arg(m_queryUrinaryBagWeightRecord.value(4).toInt(), 6, 10, QLatin1Char('0'));
            QDateTime dd = dateTime_half.fromString(m_queryUrinaryBagWeightRecord.value(3).toString() + time, "yyyyMMddhhmmss");
            qint64 timestamp = dd.toMSecsSinceEpoch();
            // 将时间戳转换为秒
            int64_t seconds = timestamp / 1000;
            // 创建日期时间对象
            QDateTime dateTime = QDateTime::fromSecsSinceEpoch(seconds);
            // 将日期时间对象转换为字符串（格式为 "yyyy-MM-dd HH:mm:ss"）
            QString dateString = dateTime.toString("yyyy-MM-dd HH:mm:ss");

            labels.append(dateString);
            i++;
        }
    }

    QCustomPlot* customPlot = ui->CustomPlot_2;

    //设定背景为黑色
    //ui->widget->setBackground(QBrush(Qt::black));
    //设定右上角图形标注可见
    customPlot->legend->setVisible(true);
    //设定右上角图形标注的字体
    customPlot->legend->setFont(QFont("Helvetica", 9));
    //添加图形
    customPlot->addGraph();
    //设置画笔
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    //设置画刷,曲线和X轴围成面积的颜色
    customPlot->graph(0)->setBrush(QBrush(QColor(255,255,0,50)));
    //设置右上角图形标注名称
    customPlot->graph(0)->setName("尿袋重量");
    //传入数据，setData的两个参数类型为double
    customPlot->graph(0)->setData(x,y);

    // 设置一个文字类型的key轴，ticks决定了轴的范围，而labels决定了轴的刻度文字的显示
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);

    //x轴的标点不能太多, 绘图会导致卡顿, 这里判断只均分取20个点位
    ///2024-7-9 备注:这种方法就无法适配数据大量的时候的时间显示, 最好的方式根据既有的时间格式通过转换时间戳的方式引用时间轴
    if(x.size()>20){
        double x_slip = static_cast<double>(x.size()) / 19.0;
        QDBG "x_slip"<<x_slip;
        for(int m =0; m<19; m++){
            //QDBG "1:"<<x_slip * m<<",2:"<<int(x_slip * m);
            textTicker->addTick(x.at(int(x_slip * m)),labels.at(int(x_slip * m)));
        }
        textTicker->addTick(x.last(),labels.last());
    }

    customPlot->xAxis->setTicker(textTicker);
    customPlot->xAxis->setTickLabelRotation(60);     // 轴刻度文字旋转60度


    QCPAxis *xAxis = customPlot->xAxis;
    QCPAxis *yAxis = customPlot->yAxis;

    //设置X轴文字标注
    xAxis->setLabel("datatime");
    //设置Y轴文字标注
    yAxis->setLabel("weight");
    //设置X轴坐标范围
    xAxis->setRange(0,x.size() + 1);
    //设置Y轴坐标范围
    yAxis->setRange(0, y_max*1.05);
    // yAxis->setRange(0, customPlot->height());
    //在坐标轴右侧和上方画线，和X/Y轴一起形成一个矩形
    //customPlot->axisRect()->setupFullAxesBox();
    QCPAxis *yAxis2 = ui->CustomPlot_2->yAxis2;

    //设置右边的Y轴可见，默认为不可见
    yAxis2->setVisible(true);
    //设置右边Y轴的范围
    yAxis2->setRange(0,10);


    // wideAxisRect->setupFullAxesBox(true); //创建四个轴，默认上轴，右轴刻度值不显示
    // wideAxisRect->axis(QCPAxis::atRight, 0)->setTickLabels(true); //右轴刻度值显示




}

void MainWindow::selectionChanged()//折线图右上角选中时候可以选中对应曲线
{
    QCustomPlot* customPlot = ui->CustomPlot_2;

    if (customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<customPlot->graphCount(); ++i)
    {
        QCPGraph *graph = customPlot->graph(i);
        QCPPlottableLegendItem *item = customPlot->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            //注意：这句需要Qcustomplot2.0系列版本
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
            //这句1.0系列版本即可
            //graph->setSelected(true);
        }
    }
}

void MainWindow::initOneHourUrineTableWidget()
{


    // 设置表头内容
    QStringList headers;
    headers <<  "开始时间" << "停止时间" << "重量" << "尿比重";

        // 添加表头
        m_oneHourTableWidetModel->setHorizontalHeaderLabels(headers);
    QStandardItem *item = new QStandardItem();
    item->setData("参数值", Qt::DisplayRole); // 将参数值设置到DisplayRole角色中
    //m_oneHourTableWidetModel->setItem(-1, -1, item); // 设置到(0,0)的位置
    //m_oneHourTableWidetModel->setVerticalHeaderItem(0,item);

    // 设置列宽不可变动
    QHeaderView* headerView = ui->tableView->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::Fixed);
    headerView->setDefaultAlignment(Qt::AlignCenter); // 设置表头文本居中对齐
    headerView->setSectionResizeMode(QHeaderView::Stretch); // 设置表头自适应宽度

    // 设置最后一栏自适应长度
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    // 设置表头颜色为灰色
    QString headerStyleSheet = "QHeaderView::section { background-color: grey; }";
    ui->tableView->horizontalHeader()->setStyleSheet(headerStyleSheet);
    ui->tableView->verticalHeader()->setStyleSheet(headerStyleSheet);

    // 利用 setModel() 方法将数据模型与 QTableView 绑定
    ui->tableView->setModel(m_oneHourTableWidetModel);
}

void MainWindow::setTableWidget(QString userIdStr)
{
    setTableOneHourUrineTableWidget(userIdStr);
}

void MainWindow::setTableOneHourUrineTableWidget(QString userIdStr)
{
    m_queryOneHourUrineVolume = readHourRecord();

    int i = 0;
    QVector<QString> startTime,stopTime,weight,urineSpecificGravity;
    // QVector<double> weight;
    // QVector<float> urineSpecificGravity;
    while (m_queryOneHourUrineVolume.next())
    {
        if(userIdStr == m_queryOneHourUrineVolume.value(1).toString()){
            startTime.append(m_queryOneHourUrineVolume.value(2).toString());
            stopTime.append(m_queryOneHourUrineVolume.value(3).toString());
            weight.append(m_queryOneHourUrineVolume.value(4).toString());
            urineSpecificGravity.append(m_queryOneHourUrineVolume.value(5).toString());
            i++;
        }
    }

    m_oneHourTableWidetModel->setRowCount(i);
    for(int m=0;m<i;m++){
        m_oneHourTableWidetModel->setHeaderData(m,Qt::Vertical, m);
        m_oneHourTableWidetModel->setItem(m, 0, new QStandardItem(startTime.at(m)));
        m_oneHourTableWidetModel->setItem(m, 1, new QStandardItem(stopTime.at(m)));
        m_oneHourTableWidetModel->setItem(m, 2, new QStandardItem(weight.at(m)));
        m_oneHourTableWidetModel->setItem(m, 3, new QStandardItem(urineSpecificGravity.at(m)));
        QDBG "1_"<<weight.at(m)<<",2_"<<urineSpecificGravity.at(m);
    }


    // 利用 setModel() 方法将数据模型与 QTableView 绑定
    ui->tableView->setModel(m_oneHourTableWidetModel);
}

void MainWindow::selectDatabaseBtn_slot()
{

    QString toOpen = QFileDialog::getOpenFileName(this, tr("Choose a database"), "", "*.db");
    initDatabase(toOpen);
    m_queryUser = readUserForm();
    int i = 0;
    while (m_queryUser.next())
    {
        QString str = m_queryUser.value(0).toString();
        ui->UserIdComboBox->addItem(str, i);
        i++;
    }
}

void MainWindow::UserIdComboBox_currentTextChanged_slot(const QString &userIdStr)
{
    QDBG userIdStr;

    m_userIdStr = userIdStr;
    setTableWidget(userIdStr);

    //柱状图
    ui->CustomPlot->clearGraphs();
    ui->CustomPlot->clearPlottables();
    drawOneHourUrineVolumeCurve(userIdStr);
    ui->CustomPlot->replot();


    //折线图
    ui->CustomPlot_2->clearGraphs();
    ui->CustomPlot_2->clearPlottables();
    drawUrinaryBagWeight(userIdStr);
    ui->CustomPlot_2->replot();
}

void MainWindow::resetBtn_slot()
{
    ui->CustomPlot->clearGraphs();
    ui->CustomPlot->clearPlottables();
    drawOneHourUrineVolumeCurve(m_userIdStr);
    ui->CustomPlot->replot();
}

void MainWindow::clearBtn_slot()
{
    QString nullStr = "clearCrave";
    ui->CustomPlot->clearGraphs();
    ui->CustomPlot->clearPlottables();

    drawOneHourUrineVolumeCurve(nullStr);
    ui->CustomPlot->replot();
}

void MainWindow::showTracer(QMouseEvent *event)
{
    double x = ui->CustomPlot_2->xAxis->pixelToCoord(event->pos().x());
    double y = 0;
    QSharedPointer<QCPGraphDataContainer> tmpContainer;
    if(ui->CustomPlot_2->graph(0) == nullptr)return;//添加曲线绘图之前要先屏蔽一下,防止空指针
    tmpContainer = ui->CustomPlot_2->graph(0)->data();
    //使用二分法快速查找所在点数据！！！敲黑板，下边这段是重点
    int low = 0, high = tmpContainer->size();
    while(high > low)
    {
        int middle = (low + high) / 2;
        if(x < tmpContainer->constBegin()->mainKey() ||
            x > (tmpContainer->constEnd()-1)->mainKey())
            break;

        if(x == (tmpContainer->constBegin() + middle)->mainKey())
        {
            y = (tmpContainer->constBegin() + middle)->mainValue();
            break;
        }
        if(x > (tmpContainer->constBegin() + middle)->mainKey())
        {
            low = middle;
        }
        else if(x < (tmpContainer->constBegin() + middle)->mainKey())
        {
            high = middle;
        }
        if(high - low <= 1)
        {   //差值计算所在位置数据
            y = (tmpContainer->constBegin()+low)->mainValue() + ( (x - (tmpContainer->constBegin() + low)->mainKey()) *
                                                                   ((tmpContainer->constBegin()+high)->mainValue() - (tmpContainer->constBegin()+low)->mainValue()) ) /
                                                                      ((tmpContainer->constBegin()+high)->mainKey() - (tmpContainer->constBegin()+low)->mainKey());
            break;
        }

    }
    //QDBG "y="<<y;
    //显示x轴的鼠标动态坐标
    //m_TraserX->updatePosition(x, 0);
    //m_TraserX->setText(QString::number(x, 'f', 0));
    //显示y轴的鼠标动态坐标，缺点无法定位xy所以无法附加单位，附加单位仍需继续修改setText传参
    //m_TracerY->updatePosition(x, y);
    //m_TracerY->setText(QString::number(y, 'f', 2));
    //由原来的x，y分别显示改为x，y显示在一起，xy单位直接在setText中设置好
    m_TracerY->updatePosition(x, y);
    m_TracerY->setText(QString::number(x, 'f', 0),QString::number(y, 'f', 2));//x轴取整数，y轴保留两位小数
    ui->CustomPlot_2->replot();
}

void MainWindow::resetBtn_2_slot()
{
    ui->CustomPlot_2->clearGraphs();
    ui->CustomPlot_2->clearPlottables();
    drawUrinaryBagWeight(m_userIdStr);
    ui->CustomPlot_2->replot();
}

void MainWindow::clearBtn_2_slot()
{
    QString nullStr = "clearCrave";
    ui->CustomPlot_2->clearGraphs();
    ui->CustomPlot_2->clearPlottables();

    drawUrinaryBagWeight(nullStr);
    ui->CustomPlot_2->replot();
}

void MainWindow::showDynamicCurveBtn_slot()//动态曲线展示  5.1.1
{
    if(!m_DynamicCurveTimer->isActive())
    {
        m_DynamicCurveTimer->start(50);
    }
}

void MainWindow::stopDynamicCurveBtn_slot()//动态曲线停止  5.1.2
{
    if(m_DynamicCurveTimer->isActive())
    {
        m_DynamicCurveTimer->stop();
    }
}

void MainWindow::slotTimeout()
{
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.002) // at most add point every 2 ms
    {
        // add data to lines
        double vv1 = qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843);
        double vv2 = qCos(key)+qrand()/(double)RAND_MAX*0.5*qSin(key/0.4364);
        QMap<int,double> mapData;
        mapData.insert(1,vv1);
        mapData.insert(2,vv2);
        // m_dock->AddData(key,mapData);
        ui->CustomPlot_3->addGraph();
        lastPointKey = key;
    }
}



void MainWindow::initConnections()
{
    connect(ui->selectDatabaseBtn,&QPushButton::clicked,this,&MainWindow::selectDatabaseBtn_slot);
    connect(ui->resetBtn,&QPushButton::clicked,this,&MainWindow::resetBtn_slot);
    connect(ui->clearBtn,&QPushButton::clicked,this,&MainWindow::clearBtn_slot);
    connect(ui->resetBtn_2,&QPushButton::clicked,this,&MainWindow::resetBtn_2_slot);
    connect(ui->clearBtn_2,&QPushButton::clicked,this,&MainWindow::clearBtn_2_slot);
    connect(ui->showDynamicCurveBtn,&QPushButton::clicked,this,&MainWindow::showDynamicCurveBtn_slot);
    connect(ui->stopDynamicCurveBtn,&QPushButton::clicked,this,&MainWindow::stopDynamicCurveBtn_slot);
    connect(ui->UserIdComboBox,&QComboBox::currentTextChanged,this,&MainWindow::UserIdComboBox_currentTextChanged_slot);
}
