/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_LEGEND_H
#define QWT_LEGEND_H

#include <qframe.h>
#include "qwt_global.h"
#if QT_VERSION < 0x040000
#include <qvaluelist.h>
#else
#include <qlist.h>
#endif

class QScrollBar;
class QwtLegendItemManager;

/*!
  \brief The legend widget

  The QwtLegend widget is a tabular arrangement of legend items. Legend
  items might be any type of widget, but in general they will be
  a QwtLegendItem.

  \sa QwtLegendItem, QwtLegendItemManager QwtPlot
*/

class QWT_EXPORT QwtLegend : public QFrame
{
    Q_OBJECT

public:
    /*!
      \brief Interaction mode for the legend items

       - ReadOnlyItem\n
         The legend item is not interactive, like a label

       - ClickableItem\n
         The legend item is clickable, like a push button

       - CheckableItem\n
         The legend item is checkable, like a checkable button

       Default is ReadOnlyItem.
       \sa setItemMode(), itemMode(), QwtLegendItem::IdentifierMode
           QwtLegendItem::clicked(), QwtLegendItem::checked(),
           QwtPlot::legendClicked(), QwtPlot::legendChecked()
     */

    enum LegendItemMode
    {
        ReadOnlyItem,
        ClickableItem,
        CheckableItem
    };

    explicit QwtLegend(QWidget *parent = NULL);
    virtual ~QwtLegend();
    
    void setItemMode(LegendItemMode);
    LegendItemMode itemMode() const;

    QWidget *contentsWidget();
    const QWidget *contentsWidget() const;

    void insert(const QwtLegendItemManager *, QWidget *);
    void remove(const QwtLegendItemManager *);

    QWidget *find(const QwtLegendItemManager *) const;
    QwtLegendItemManager *find(const QWidget *) const;

#if QT_VERSION < 0x040000
    virtual QValueList<QWidget *> legendItems() const;
#else
    virtual QList<QWidget *> legendItems() const;
#endif

    void clear();
    
    bool isEmpty() const;
    uint itemCount() const;

    virtual bool eventFilter(QObject *, QEvent *);

    virtual QSize sizeHint() const;
    virtual int heightForWidth(int w) const;

    QScrollBar *horizontalScrollBar() const;
    QScrollBar *verticalScrollBar() const;

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void layoutContents();

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif // QWT_LEGEND_H
