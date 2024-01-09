#ifndef MAINWINDOW_H
#define MAINWINDOW_H


/**
 * @brief QCustomPlot功能接口练习
 * @author 王宾
 * @date 创建时间 2024-1-5
 * @version 1.0
 * @property 引言:这里通过分析一张数据库表, 练习QCustomPlot的各种功能接口
 * (数据源的获取形式不一,我这里用的数据库文件分析,用网络接口等方式做绘图亦可,这里数据获取逻辑就不再赘述)
 *
 *  已实现功能:
 *      重置绘图\绘图位置修正[复位](√), 绘图清空(√), 自适应区域绘制(√),游标(x),图形标记选中(√),另存为图片(x)

    1.引入QCustomPlot库, 这里需要从QCustomPlot的官网中下载.
    2.生成QCustomPlot动态库,这里的目的是为了在调用QCustomPlot库的时候提升代码的执行效率.
    (以上前两条在往期的QCustomPlot相关文档中已经详细阐述过, 这里不再赘述, 需要的请自行翻阅)
    3.然后是通过QCustomPlot实现的一些具体功能, 首先是生成一个柱状图(柱状图笔记)
        3.1 柱状图需要实现的功能:
            3.1.1图形复位(图形窗口自适应):
                从数据库取值X轴是日期时间, Y轴是对应的值. 首先是X轴,考虑到我们调用的QCustomPlot的窗口在软件中占用的区域是有限的,但
            可能存在某个表中X数据很多, 为了保证X轴能再绘制的时候能够完全显示全, 设定X轴的setRange的时候需要获取表中X周对应的value的
            数量index, 即: " xAxis->setRange(0, x.size() + 1); ". 然后是Y轴, Y轴的值范围设定需要考虑几点, 表中的Y轴可能存在几
            个极端的大值, 因为控件表会自适应, 这些极端大值相当于是降低里样本的"普遍性",所以单就我的这张数据库表, 自适应Y轴不能取样本的
            极大值, 我这设定的Y轴区域范围是0到控件的实际高度,  即:"yAxis->setRange(0, ui->CustomPlot->height());",这是在我的这
            个数据库样本的情况下的最优解. 在点击按钮之后, 放大或者缩小过的图像可以恢复首次读取时候的状态

            3.1.2图形清空功能
                点击按钮情况柱状图的绘制

            3.1.3柱状图每个单元value值显示
                这里是柱状图的每条的数值显示, 需要注意设置透明色

    4.接下来是QCustomPlot折现图的功能接口, 这里我会分开放在不同函数中, 方便理解.
        4.1折线图需要实现的功能
            4.1.1折线图的绘制
            4.1.2折线图的框选功能
            4.1.3折线图的鼠标指针指向的时候显示数值的功能
            4.1.4折线图的复位功能(窗口自适应)
            4.1.5折线图的清空功能

    5.动态波形图(待补充),根据动态接口响应的数据,实时变化波形图曲线


 *
 */



#include <QMainWindow>
#include <QSqlQuery>
#include <qcustomplot.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selectDatabaseBtn_clicked();

    void on_UserIdComboBox_currentTextChanged(const QString &userIdStr);

    void on_resetBtn_clicked();//复位按钮(柱状图)  3.1.1

    void on_clearBtn_clicked();//清除按钮(柱状图)  3.1.2

    void on_resetBtn_2_clicked();//复位按钮(折线图)    4.1.4

    void on_clearBtn_2_clicked();//清除按钮(折线图)    4.1.5

private:
    Ui::MainWindow *ui;
    QSqlDatabase m_database;//初始化数据库
    QSqlQuery m_queryUser,m_queryOneHourUrineVolume,m_queryUrinaryBagWeightRecord;//查询表[用户表][一小时尿量表][尿袋重量表]
    QString m_userIdStr;//获取数据库表中用户的id


    void initDatabase(QString path);//初始化数据库
    QSqlQuery readUserForm();//读取用户表
    QSqlQuery readHourRecord();//读取一小时记录表
    QSqlQuery readUrinaryBagWeightRecord();//读取尿袋重量记录表

    void drawOneHourUrineVolumeCurve(QString userIdStr);//绘制一小时尿流率柱状图 [3.]
    void drawUrinaryBagWeight(QString userIdStr);//绘制尿袋重量折线图 [4.]
    void selectionChanged();//4.1.2折线图的框选功能

};
#endif // MAINWINDOW_H
