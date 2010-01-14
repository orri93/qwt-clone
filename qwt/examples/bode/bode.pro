# -*- mode: sh -*- ################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
###################################################################

include( ../examples.pri )

TARGET       = bode
QT          += svg

HEADERS = \
    mainwindow.h \
    plot.h \
    complexnumber.h \ 
    pixmaps.h

SOURCES = \
    plot.cpp \
    mainwindow.cpp \
    main.cpp
