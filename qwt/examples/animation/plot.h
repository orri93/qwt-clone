#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>
#include <qdatetime.h>

class Curve;

class Plot: public QwtPlot
{
public:
    Plot( QWidget * = NULL);

protected:
    virtual void timerEvent( QTimerEvent * ) QWT_OVERRIDE;

private:
    void updateCurves();

    enum { CurveCount = 4 };
    Curve *d_curves[CurveCount];

    QTime d_time;
};

#endif
