######################################################################
# Automatically generated by qmake (2.01a) Fr Mai 1 20:01:45 2009
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += . \
              include \
              src \
              include/plugins \
              src/plugins \
              include/plugins/environmentviewer \
              include/plugins/modulestatisticsviewer \
              include/plugins/objxviewer \
              include/plugins/scnxviewer \
              include/plugins/sharedimageviewer \
              include/plugins/timeControl \
              src/plugins/environmentviewer \
              src/plugins/modulestatisticsviewer \
              src/plugins/objxviewer \
              src/plugins/scnxviewer \
              src/plugins/sharedimageviewer \
              src/plugins/timeControl
INCLUDEPATH += . \
               include \
               include/plugins \
               include/plugins/objxviewer \
               include/plugins/scnxviewer \
               include/plugins/sharedimageviewer \
               include/plugins/environmentviewer \
               include/plugins/timeControl \
               include/plugins/modulestatisticsviewer

# Input
HEADERS += include/ActiveBuffer.h \
           include/BufferedFIFOMultiplexer.h \
           include/ContainerObserver.h \
           include/DataStoreManager.h \
           include/FIFOMultiplexer.h \
           include/MdiPlugIn.h \
           include/Monitor.h \
           include/MonitorWindow.h \
           include/QtIncludes.h \
           include/plugins/AbstractGLWidget.h \
           include/plugins/GLControlFrame.h \
           include/plugins/MasterPlugIn.h \
           include/plugins/PlugIn.h \
           include/plugins/PlugInProvider.h \
           include/plugins/environmentviewer/EnvironmentViewerPlugIn.h \
           include/plugins/environmentviewer/EnvironmentViewerWidget.h \
           include/plugins/modulestatisticsviewer/LoadPerModule.h \
           include/plugins/modulestatisticsviewer/LoadPlot.h \
           include/plugins/modulestatisticsviewer/ModuleStatisticsViewerPlugIn.h \
           include/plugins/modulestatisticsviewer/ModuleStatisticsViewerWidget.h \
           include/plugins/objxviewer/OBJXGLWidget.h \
           include/plugins/objxviewer/OBJXViewerPlugIn.h \
           include/plugins/objxviewer/OBJXViewerWidget.h \
           include/plugins/scnxviewer/SCNXViewerPlugIn.h \
           include/plugins/scnxviewer/SCNXViewerWidget.h \
           include/plugins/sharedimageviewer/SharedImageViewerPlugIn.h \
           include/plugins/sharedimageviewer/SharedImageViewerWidget.h \
           include/plugins/timeControl/TimeControlPlugIn.h \
           include/plugins/timeControl/TimeControlWidget.h
SOURCES += src/ActiveBuffer.cpp \
           src/BufferedFIFOMultiplexer.cpp \
           src/ContainerObserver.cpp \
           src/DataStoreManager.cpp \
           src/FIFOMultiplexer.cpp \
           src/MainModule.cpp \
           src/MdiPlugIn.cpp \
           src/Monitor.cpp \
           src/MonitorWindow.cpp \
           src/plugins/AbstractGLWidget.cpp \
           src/plugins/GLControlFrame.cpp \
           src/plugins/MasterPlugIn.cpp \
           src/plugins/PlugIn.cpp \
           src/plugins/PlugInProvider.cpp \
           src/plugins/environmentviewer/EnvironmentViewerPlugIn.cpp \
           src/plugins/environmentviewer/EnvironmentViewerWidget.cpp \
           src/plugins/modulestatisticsviewer/LoadPerModule.cpp \
           src/plugins/modulestatisticsviewer/LoadPlot.cpp \
           src/plugins/modulestatisticsviewer/ModuleStatisticsViewerPlugIn.cpp \
           src/plugins/modulestatisticsviewer/ModuleStatisticsViewerWidget.cpp \
           src/plugins/objxviewer/OBJXGLWidget.cpp \
           src/plugins/objxviewer/OBJXViewerPlugIn.cpp \
           src/plugins/objxviewer/OBJXViewerWidget.cpp \
           src/plugins/scnxviewer/SCNXViewerPlugIn.cpp \
           src/plugins/scnxviewer/SCNXViewerWidget.cpp \
           src/plugins/sharedimageviewer/SharedImageViewerPlugIn.cpp \
           src/plugins/sharedimageviewer/SharedImageViewerWidget.cpp \
           src/plugins/timeControl/TimeControlPlugIn.cpp \
           src/plugins/timeControl/TimeControlWidget.cpp
RESOURCES += monitor.qrc
TRANSLATIONS += monitor_de_DE.ts