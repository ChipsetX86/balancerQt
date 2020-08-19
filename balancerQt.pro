QT       += core network
CONFIG   += c++11 console

SOURCES += \
    serverpool.cpp \
    balancerproxy.cpp \
    main.cpp
HEADERS += \
    serverpool.h \
    balancerproxy.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md
