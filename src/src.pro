TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += \
    qml \
    quick

#qtHaveModule(gui):contains(QT_CONFIG, opengl(es1|es2)?) {
#    SUBDIRS += \
#        quick \
#        qmltest \
#        particles
#}

SUBDIRS += \
#    plugins \
#    imports \
#    qmldevtools

#qtHaveModule(quick):qtHaveModule(widgets): SUBDIRS += quickwidgets

qmldevtools.CONFIG = host_build
