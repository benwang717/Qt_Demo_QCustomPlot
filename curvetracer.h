#ifndef CURVETRACER_H
#define CURVETRACER_H


#include <QObject>
#include <qcustomplot.h>

enum TracerType
{
    XAxisTracer,
    YAxisTracer,
    DataTracer
};

class CurveTracer : public QObject
{
    Q_OBJECT
public:



public:
    explicit CurveTracer(QCustomPlot *_plot,QCPGraph *_graph, TracerType _type);//这里与原贴不同，按照原贴构造总是过不去
    ~CurveTracer();

    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void setText(const QString &text,const QString &text1);
    //void setText(const QString &text);
    void setLabelPen(const QPen &pen);
    void updatePosition(double xValue, double yValue);

protected:
    void setVisible(bool visible);
protected:
    QCustomPlot *plot ;	     //传入实例化的QcustomPlot
    QCPGraph *graph;	   	 //这里是存传入的绘图图层
    QCPItemTracer *tracer;   // 跟踪的点
    QCPItemText *label;   	 // 显示的数值
    QCPItemLine *arrow;  	 // 箭头

    TracerType type;
    bool visible;

signals:

public slots:

};

#endif // CURVETRACER_H
