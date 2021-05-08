QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Source/computationhandler.cpp \
    Source/graphicslassoitem.cpp \
    Source/imagegraphicsview.cpp \
    Source/main.cpp \
    Source/mainwindow.cpp \
    Source/pastedsourceitem.cpp \
    Source/sourcegraphicsscene.cpp \
    Source/targetgraphicsscene.cpp

HEADERS += \
    Source/computationhandler.h \
    Source/graphicslassoitem.h \
    Source/imagegraphicsview.h \
    Source/mainwindow.h \
    Source/pastedsourceitem.h \
    Source/sourcegraphicsscene.h \
    Source/targetgraphicsscene.h

FORMS += \
    UI/mainwindow.ui


INCLUDEPATH += 3rdparty/eigen Source/

RC_ICONS = Resources/Painting.ico

# Disable attributes warnings on MSYS/MXE due to gcc bug spamming the logs: Issue #2771
win* | CONFIG(mingw-cross-env)|CONFIG(mingw-cross-env-shared) {
    QMAKE_CXXFLAGS += -Wno-attributes
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources/resources.qrc
