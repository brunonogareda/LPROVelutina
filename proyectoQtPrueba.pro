#-------------------------------------------------
#
# Project created by QtCreator 2017-03-07T16:44:07
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = proyectoQtPrueba
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH +=  "D:\opencv\build\include\opencv" \
    "D:\opencv\build\include"

LIBS += -L"D:\opencv\build\x64\vc11\lib" \
    -lopencv_calib3d2413  \
    -lopencv_imgproc2413  \
    -lopencv_core2413 \
    -lopencv_highgui2413 \
    -lopencv_ml2413 \
    -lopencv_objdetect2413 \
    -lopencv_flann2413 \
    -lopencv_features2d2413 \
 -lopencv_nonfree2413 \
-lopencv_video2413 \


SOURCES += main.cpp
