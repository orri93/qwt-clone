/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_cardinal.h"
#include "qwt_spline_parametrization.h"

namespace QwtSplineCardinalG1P
{
    struct param
    {
        param( const QwtSplineParametrization *p ):
            parameter( p )
        {
        }

        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return parameter->valueIncrement( p1, p2 );
        }

        const QwtSplineParametrization *parameter;
    };

    struct paramX
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementX( p1, p2 );
        }
    };

    struct paramUniform
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementUniform( p1, p2 );
        }
    };

    struct paramCentripetal
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementCentripetal( p1, p2 );
        }
    };

    struct paramChordal
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementChordal( p1, p2 );
        }
    };

    struct paramManhattan
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementManhattan( p1, p2 );
        }
    };


    class PathStore
    {
    public:
        inline void init( int )
        {
        }

        inline void start( const QPointF &p0 )
        {
            path.moveTo( p0 );
        }

        inline void addCubic( const QPointF &cp1,
            const QPointF &cp2, const QPointF &p2 )
        {
            path.cubicTo( cp1, cp2, p2 );
        }

        QPainterPath path;
    };

    class ControlPointsStore
    {
    public:
        inline ControlPointsStore():
            d_cp( NULL )
        {
        }

        inline void init( int size )
        {
            controlPoints.resize( size );
            d_cp = controlPoints.data();
        }

        inline void start( const QPointF & )
        {
        }

        inline void addCubic( const QPointF &cp1,
            const QPointF &cp2, const QPointF & )
        {
            QLineF &l = *d_cp++;
            l.setPoints( cp1, cp2 );
        }

        QVector<QLineF> controlPoints;

    private:
        QLineF* d_cp;
    };
}

static inline QwtSplineCardinalG1::Tension qwtTension(
    double d13, double d23, double d24,
    const QPointF &p1, const QPointF &p2,
    const QPointF &p3, const QPointF &p4 )
{
    const bool b1 = ( d13 / 3.0 ) < d23;
    const bool b2 = ( d24 / 3.0 ) < d23;

    QwtSplineCardinalG1::Tension tension;
    if ( b1 && b2 )
    {
        tension.t1 = ( p1 != p2 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
        tension.t2 = ( p3 != p4 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
    }
    else
    {
        tension.t1 = d23 / ( b1 ? d24 : d13 );
        tension.t2 = d23 / ( b2 ? d13 : d24 );
    }

    return tension;
}

template< class Param >
static inline QVector<QwtSplineCardinalG1::Tension> qwtTensions( 
    const QPolygonF &points, bool isClosed, Param param ) 
{
    const int size = points.size();
    QVector<QwtSplineCardinalG1::Tension> tensions2( isClosed ? size : size - 1 );

    const QPointF *p = points.constData();
    QwtSplineCardinalG1::Tension *t = tensions2.data();

    const QPointF &p0 = isClosed ? p[size-1] : p[0];
    double d13 = param(p[0], p[2]);

    t[0] = qwtTension( param(p0, p[1]), param(p[0], p[1]), 
        d13, p0, p[0], p[1], p[2] );

    for ( int i = 1; i < size - 2; i++ )
    {
        const double d23 = param( p[i], p[i+1] );
        const double d24 = param( p[i], p[i+2] );

        t[i] = qwtTension( d13, d23, d24, p[i-1], p[i], p[i+1], p[i+2] );

        d13 = d24;
    }

    const QPointF &pn = isClosed ? p[0] : p[size-1];
    const double d24 = param( p[size-2], pn );

    t[size-2] = qwtTension( d13, param( p[size-2], p[size-1] ), d24,
        p[size - 3], p[size - 2], p[size - 1], pn );

    if ( isClosed )
    {
        const double d34 = param( p[size-1], p[0] );
        const double d35 = param( p[size-1], p[1] );

        t[size-1] = qwtTension( d24, d34, d35, p[size-2], p[size-1], p[0], p[1] );
    }

    return tensions2;
}

template< class SplineStore >
static SplineStore qwtSplinePathG1( 
    const QwtSplineCardinalG1 *spline, const QPolygonF &points ) 
{
    const int size = points.size();
    const int numTensions = spline->isClosing() ? size : size - 1;

    const QVector<QwtSplineCardinalG1::Tension> tensions2 = spline->tensions( points );
    if ( tensions2.size() != numTensions )
        return SplineStore();

    const QPointF *p = points.constData();
    const QwtSplineCardinalG1::Tension *t = tensions2.constData();

    SplineStore store;
    store.init( numTensions );
    store.start( p[0] );
    
    const QPointF &p0 = spline->isClosing() ? p[size-1] : p[0];
    QPointF vec1 = ( p[1] - p0 ) * 0.5;

    for ( int i = 0; i < size - 2; i++ )
    {
        const QPointF vec2 = ( p[i+2] - p[i] ) * 0.5;
        store.addCubic( p[i] + vec1 * t[i].t1, 
            p[i+1] - vec2 * t[i].t2, p[i+1] );
        
        vec1 = vec2;
    }   
    
    const QPointF &pn = spline->isClosing() ? p[0] : p[size-1];
    const QPointF vec2 = 0.5 * ( pn - p[size - 2] );

    store.addCubic( p[size - 2] + vec1 * t[size-2].t1,
        p[size - 1] - vec2 * t[size-2].t2, p[size - 1] );

    if ( spline->isClosing() )
    {
        const QPointF vec3 = 0.5 * ( p[1] - p[size-1] );
        store.addCubic( p[size-1] + vec2 * t[size-1].t1, 
            p[0] - vec3 * t[size-1].t2, p[0] );
    }

    return store;
}

template< class SplineStore, class Param >
static SplineStore qwtSplinePathPleasing( const QPolygonF &points, 
    bool isClosed, Param param )
{
    const int size = points.size();

    const QPointF *p = points.constData();

    SplineStore store;
    store.init( isClosed ? size : size - 1 );
    store.start( p[0] );

    const QPointF &p0 = isClosed ? p[size-1] : p[0];
    double d13 = param(p[0], p[2]);

    const QwtSplineCardinalG1::Tension t0 = qwtTension( 
        param(p0, p[1]), param(p[0], p[1]), d13, p0, p[0], p[1], p[2] );

    const QPointF vec0 = ( p[1] - p0 ) * 0.5;
    QPointF vec1 = ( p[2] - p[0] ) * 0.5;

    store.addCubic( p[0] + vec0 * t0.t1, p[1] - vec1 * t0.t2, p[1] );

    for ( int i = 1; i < size - 2; i++ )
    {
        const double d23 = param(p[i], p[i+1]);
        const double d24 = param(p[i], p[i+2]);
        const QPointF vec2 = ( p[i+2] - p[i] ) * 0.5;

        const QwtSplineCardinalG1::Tension t =
            qwtTension( d13, d23, d24, p[i-1], p[i], p[i+1], p[i+2] );

        store.addCubic( p[i] + vec1 * t.t1, p[i+1] - vec2 * t.t2, p[i+1] );

        d13 = d24;
        vec1 = vec2;
    }

    const QPointF &pn = isClosed ? p[0] : p[size-1];
    const double d24 = param( p[size-2], pn );

    const QwtSplineCardinalG1::Tension tn = qwtTension( 
        d13, param( p[size-2], p[size-1] ), d24, p[size-3], p[size-2], p[size-1], pn );

    const QPointF vec2 = 0.5 * ( pn - p[size-2] );
    store.addCubic( p[size-2] + vec1 * tn.t1, p[size-1] - vec2 * tn.t2, p[size-1] );

    if ( isClosed )
    {
        const double d34 = param( p[size-1], p[0] );
        const double d35 = param( p[size-1], p[1] );

        const QPointF vec3 = 0.5 * ( p[1] - p[size-1] );

        const QwtSplineCardinalG1::Tension tn = 
            qwtTension( d24, d34, d35, p[size-2], p[size-1], p[0], p[1] );
        store.addCubic( p[size-1] + vec2 * tn.t1, p[0] - vec3 * tn.t2, p[0] );
    }

    return store;
}

QwtSplineCardinalG1::QwtSplineCardinalG1()
{
}

QwtSplineCardinalG1::~QwtSplineCardinalG1()
{
}

QVector<QLineF> QwtSplineCardinalG1::bezierControlLines( 
    const QPolygonF &points ) const
{
    using namespace QwtSplineCardinalG1P;
    const ControlPointsStore store = 
        qwtSplinePathG1<ControlPointsStore>( this, points );

    return store.controlPoints;
}

/*!
  \brief Interpolate a curve with Bezier curves

  Interpolates a polygon piecewise with cubic Bezier curves
  and returns them as QPainterPath.

  The implementation calculates the tensions at the control points
  and converts them into painter path elements in an additional loop.
  
  \param points Control points
  \return QPainterPath Painter path, that can be rendered by QPainter

  \note Derived spline classes might overload painterPath() to avoid
        the extra loops for converting results into a QPainterPath

  \sa tensions()
 */
QPainterPath QwtSplineCardinalG1::painterPath( const QPolygonF &points ) const
{
    using namespace QwtSplineCardinalG1P;
    PathStore store = qwtSplinePathG1<PathStore>( this, points );
    if ( isClosing() )
        store.path.closeSubpath();

    return store.path;
}

uint QwtSplineCardinalG1::locality() const
{
    return 2;
}

QwtSplinePleasing::QwtSplinePleasing()
{
}

QwtSplinePleasing::~QwtSplinePleasing()
{
}

/*! 
  \brief Interpolate a curve with Bezier curves

  Interpolates a polygon piecewise with cubic Bezier curves
  and returns them as QPainterPath.
    
  \param points Control points
  \return QPainterPath Painter path, that can be rendered by QPainter
 */     
QPainterPath QwtSplinePleasing::painterPath( const QPolygonF &points ) const
{
    const int size = points.size();
    if ( size <= 2 )
        return QwtSplineCardinalG1::painterPath( points );

    using namespace QwtSplineCardinalG1P;

    PathStore store;
    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterX:
        {
            store = qwtSplinePathPleasing<PathStore>( points, 
                isClosing(), QwtSplineCardinalG1P::paramX() );
            break;
        }
        case QwtSplineParametrization::ParameterUniform:
        {
            store = qwtSplinePathPleasing<PathStore>( points, 
                isClosing(), QwtSplineCardinalG1P::paramUniform() );
            break;
        }
        case QwtSplineParametrization::ParameterCentripetal:
        {
            store = qwtSplinePathPleasing<PathStore>( points, 
                isClosing(), QwtSplineCardinalG1P::paramCentripetal() );
            break;
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            store = qwtSplinePathPleasing<PathStore>( points, 
                isClosing(), QwtSplineCardinalG1P::paramChordal() );
            break;
        }
        default:
        {
            store = qwtSplinePathPleasing<PathStore>( points, 
                isClosing(), QwtSplineCardinalG1P::param( parametrization() ) );
        }
    }

    if ( isClosing() )
        store.path.closeSubpath();

    return store.path;
}

QVector<QLineF> QwtSplinePleasing::bezierControlLines( 
    const QPolygonF &points ) const
{
    // TODO ...
    return QwtSplineCardinalG1::bezierControlLines( points );
}

QVector<QwtSplineCardinalG1::Tension> 
QwtSplinePleasing::tensions( const QPolygonF &points ) const
{
    QVector<Tension> ts;

    if ( points.size() <= 2 )
        return ts;

    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterX:
        {
            ts = qwtTensions( points, 
                isClosing(), QwtSplineCardinalG1P::paramX() );
            break;
        }
        case QwtSplineParametrization::ParameterUniform:
        {
            ts = qwtTensions( points, 
                isClosing(), QwtSplineCardinalG1P::paramUniform() );
            break;
        }
        case QwtSplineParametrization::ParameterCentripetal:
        {
            ts = qwtTensions( points, 
                isClosing(), QwtSplineCardinalG1P::paramCentripetal() );
            break;
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            ts = qwtTensions( points, 
                isClosing(), QwtSplineCardinalG1P::paramChordal() );
            break;
        }
        default:
        {
            ts = qwtTensions( points, 
                isClosing(), QwtSplineCardinalG1P::param( parametrization() ) );
        }
    }

    return ts;
}
