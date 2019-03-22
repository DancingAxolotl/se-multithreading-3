TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    ThreadPool.cpp \
    PooledThread.cpp

HEADERS += \
    ThreadPool.h \
    PooledThread.h
