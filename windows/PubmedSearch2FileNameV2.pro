QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 staticLib
OPENSSL_LIBS += "D:\openssl-1.1.1d\x64\lib -lssl -lcrypto" ./configure -openssl-linked
#INCLUDEPATH += "D:\openssl-1.1.1d\x64\include"
RESOURCES = Resource.qrc
RC_FILE = Resource.rc
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainDialog.cpp \
    widget.cpp

HEADERS += \
    mainDialog.h \
    ui_widget.h \
    widget.h

TRANSLATIONS += \
    PubmedSearch2FileNameV2_ja_JP.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    widget.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../openssl-1.1.1d/x64/lib/ -llibcrypto
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../openssl-1.1.1d/x64/lib/ -llibcryptod
else:unix: LIBS += -L$$PWD/../../../openssl-1.1.1d/x64/lib/ -llibcrypto

INCLUDEPATH += $$PWD/../../../openssl-1.1.1d/x64/include
DEPENDPATH += $$PWD/../../../openssl-1.1.1d/x64/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/liblibcrypto.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/liblibcryptod.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/libcrypto.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/libcryptod.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/liblibcrypto.a

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../openssl-1.1.1d/x64/lib/ -llibssl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../openssl-1.1.1d/x64/lib/ -llibssld
else:unix: LIBS += -L$$PWD/../../../openssl-1.1.1d/x64/lib/ -llibssl

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/liblibssl.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/liblibssld.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/libssl.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/libssld.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../../openssl-1.1.1d/x64/lib/liblibssl.a

RESOURCES += \
    Resource.qrc
