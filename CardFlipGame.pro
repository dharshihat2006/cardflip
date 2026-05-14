QT += core gui widgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


SOURCES += \
    database.cpp \
    gamepage.cpp \
    leaderboardpage.cpp \
    loginpage.cpp \
    main.cpp \
    mainwindow.cpp \
    menupage.cpp

HEADERS += \
    database.h \
    gamepage.h \
    leaderboardpage.h \
    loginpage.h \
    mainwindow.h \
    menupage.h

FORMS += \
    gamepage.ui \
    leaderboardpage.ui \
    loginpage.ui \
    mainwindow.ui \
    menupage.ui

# Default rules for deployment.



