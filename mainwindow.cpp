#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_database = QSqlDatabase::addDatabase("QSQLITE");



    ui->CustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    //设置基本坐标轴（左侧Y轴和下方X轴）可拖动、可缩放、曲线可选、legend可选、设置伸缩比例，使所有图例可见
    ui->CustomPlot_2->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom| QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->CustomPlot_2->legend->setSelectableParts(QCPLegend::spItems);
    connect(ui->CustomPlot_2, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectDatabaseBtn_clicked()
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
        //qDebug()<<database.lastError().text();
        qDebug() << "数据库打开失败";
        return;
    }
    else {
        qDebug() << "数据库打开成功！";
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
    xAxis->setTickLabelRotation(-60);     // 轴刻度文字旋转-60度
    xAxis->setSubTicks(false);           // 不显示子刻度
    xAxis->setTickLength(0, 4);          // 轴内外刻度的长度分别是0,4,也就是轴内的刻度线不显示
    xAxis->setRange(0, x.size() + 1);               // 设置范围
    xAxis->setLabel("x");
    xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);

    yAxis->setRange(0, ui->CustomPlot->height());
    yAxis->setPadding(35);             // 轴的内边距，可以到QCustomPlot之开始（一）看图解
    yAxis->setLabel("Power Consumption in\nKilowatts per Capita (2007)");
    yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    bars->setData(x, y);

}

void MainWindow::drawUrinaryBagWeight(QString userIdStr)//绘制折线图
{
    if("clearCrave" == userIdStr)return;

    m_queryUrinaryBagWeightRecord = readUrinaryBagWeightRecord();

    int i = 0;
    QVector<double> x, y;
    QVector<QString> labels;
    while (m_queryUrinaryBagWeightRecord.next())
    {
        if(userIdStr == m_queryUrinaryBagWeightRecord.value(1).toString()){
            x.append(i + 1);
            y.append(m_queryUrinaryBagWeightRecord.value(2).toDouble());
            labels.append(m_queryUrinaryBagWeightRecord.value(2).toString());
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
    // QVector<double> x(101),y(101);
    // //图形为y=x^3
    // for(int i=0;i<101;i++)
    // {
    //     x[i] = i/5.0-10;
    //     y[i] = x[i]*x[i]*x[i];//qPow(x[i],3)
    // }
    //添加图形
    customPlot->addGraph();
    //设置画笔
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    //设置画刷,曲线和X轴围成面积的颜色
    customPlot->graph(0)->setBrush(QBrush(QColor(255,255,0,50)));
    //设置右上角图形标注名称
    customPlot->graph(0)->setName("曲线");
    //传入数据，setData的两个参数类型为double
    customPlot->graph(0)->setData(x,y);

    // QVector<double> temp(20);
    // QVector<double> temp1(20);
    // //图形为y = 100*x;
    // for(int i=0;i<20;i++)
    // {
    //     temp[i] = i;
    //     temp1[i] = 10*i+10;
    // }
    //添加图形
    //customPlot->addGraph();
    //设置画笔
    //customPlot->graph(1)->setPen(QPen(Qt::red));
    //设置画刷,曲线和X轴围成面积的颜色
    //customPlot->graph(1)->setBrush(QBrush(QColor(0,255,0)));
    //传入数据
    //customPlot->graph(1)->setData(temp,temp1);

    /*-------------------------------------------*/
    //画动态曲线时，传入数据采用addData，通过定时器多次调用，并在之后调用customPlot->replot();
            //动态曲线可以通过另一种设置坐标的方法解决坐标问题：
            //setRange ( double  position, double  size, Qt::AlignmentFlag  alignment  )
    //参数分别为：原点，偏移量，对其方式，有兴趣的读者可自行尝试，欢迎垂询
            /*-------------------------------------------*/

    //设置右上角图形标注名称
    //customPlot->graph(1)->setName("直线");


    QCPAxis *xAxis = customPlot->xAxis;
    QCPAxis *yAxis = customPlot->yAxis;

    //设置X轴文字标注
    xAxis->setLabel("time");
    //设置Y轴文字标注
    yAxis->setLabel("temp/shidu");
    //设置X轴坐标范围
    xAxis->setRange(0,x.size() + 1);
    //设置Y轴坐标范围
    yAxis->setRange(0, customPlot->height());
    //在坐标轴右侧和上方画线，和X/Y轴一起形成一个矩形
    customPlot->axisRect()->setupFullAxesBox();




}

void MainWindow::selectionChanged()//折线图右上角选中时候可以选中对应曲线
{
    qDebug()<<"__FUNCTION__"<<__FUNCTION__;

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


void MainWindow::on_UserIdComboBox_currentTextChanged(const QString &userIdStr)
{

    qDebug()<<__FUNCTION__<<userIdStr;


    m_userIdStr = userIdStr;

    //柱状图
    ui->CustomPlot->clearGraphs();
    ui->CustomPlot->clearPlottables();
    ui->CustomPlot->replot();
    drawOneHourUrineVolumeCurve(userIdStr);
    ui->CustomPlot->replot();


    //折线图
    ui->CustomPlot_2->clearGraphs();
    ui->CustomPlot_2->clearPlottables();
    ui->CustomPlot_2->replot();
    drawUrinaryBagWeight(userIdStr);
    ui->CustomPlot_2->replot();

}


void MainWindow::on_resetBtn_clicked()//复位按钮(柱状图)  3.1.1
{
    on_clearBtn_clicked();
    drawOneHourUrineVolumeCurve(m_userIdStr);
    ui->CustomPlot->replot();
}



void MainWindow::on_clearBtn_clicked()//清除按钮(柱状图)  3.1.2
{
    QString nullStr = "clearCrave";
    ui->CustomPlot->clearGraphs();
    ui->CustomPlot->clearPlottables();

    drawOneHourUrineVolumeCurve(nullStr);
    ui->CustomPlot->replot();
}


void MainWindow::on_resetBtn_2_clicked()//复位按钮(折线图)    4.1.4
{
    on_clearBtn_2_clicked();
    drawUrinaryBagWeight(m_userIdStr);
    ui->CustomPlot_2->replot();
}


void MainWindow::on_clearBtn_2_clicked()//清除按钮(折线图)    4.1.5
{
    QString nullStr = "clearCrave";
    ui->CustomPlot_2->clearGraphs();
    ui->CustomPlot_2->clearPlottables();

    drawUrinaryBagWeight(nullStr);
    ui->CustomPlot_2->replot();
}

