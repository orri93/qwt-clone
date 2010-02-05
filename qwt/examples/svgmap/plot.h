#include <qwt_plot.h>
#include <qrect.h>

class QwtPlotSvgItem;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);

public Q_SLOTS:
    void loadSVG();
    void loadSVG(const QString &);

private:
    void rescale();

    QwtPlotSvgItem *d_mapItem;
    const QRectF d_mapRect;
};
