#  Copyright 2025 Jose Maria Castelo Ares
#
#  Contact: <jose.maria.castelo@gmail.com>
#  Repository: <https://github.com/jmcastelo/Fosforo>
#
#  This file is part of Fosforo.
#
#  Fosforo is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Fosforo is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Fosforo.  If not, see <https://www.gnu.org/licenses/>.

TARGET = fosforo

DESTDIR = ../../

TEMPLATE = app

CONFIG += qt c++20

unix:QMAKE_CXXFLAGS += -DLIBREMIDI_ALSA=1 -DLIBREMIDI_HEADER_ONLY=1 -pthread
unix:LIBS += -lasound -pthread

win32:QMAKE_CXXFLAGS += -DLIBREMIDI_WINUWP=1 -DLIBREMIDI_HEADER_ONLY=1
win32:LIBS += "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64\WindowsApp.lib"
win32:QMAKE_LIBDIR += "$$_PRO_FILE_PWD_\..\ffmpeg\install\lib"
win32:INCLUDEPATH += "..\libremidi\include"
win32:INCLUDEPATH += "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\cppwinrt"

QT += widgets openglwidgets multimedia opengl

RESOURCES += resource.qrc

HEADERS += \
    src/applicationcontroller.h \
    src/colorpath.h \
    src/configparser.h \
    src/controlwidget.h \
    src/cycle.h \
    src/cyclesearch.h \
    src/edge.h \
    src/edgewidget.h \
    src/factory.h \
    src/graphwidget.h \
    src/gridwidget.h \
    src/histogramwidget.h \
    src/imageoperation.h \
    src/imageoperationnode.h \
    src/inputdata.h \
    src/message.h \
    src/midicontrol.h \
    src/midiinputport.h \
    src/midilinkmanager.h \
    src/midilistwidget.h \
    src/midisignals.h \
    src/morphowidget.h \
    src/node.h \
    src/nodemanager.h \
    src/operationbuilder.h \
    src/operationparser.h \
    src/operationwidget.h \
    src/overlay.h \
    src/parameters/baseuniformparameter.h \
    src/parameters/number.h \
    src/parameters/optionsparameter.h \
    src/parameters/parameter.h \
    src/parameters/uniformmat4parameter.h \
    src/parameters/uniformparameter.h \
    src/plotswidget.h \
    src/recorder.h \
    src/rendermanager.h \
    src/rgbwidget.h \
    src/seed.h \
    src/seedwidget.h \
    src/texformat.h \
    src/videoinputcontrol.h \
    src/widgets/focuswidgets.h \
    src/widgets/layoutformat.h \
    src/widgets/optionswidget.h \
    src/widgets/parameterwidget.h \
    src/widgets/uniformmat4widget.h \
    src/widgets/uniformwidget.h

SOURCES += \
    src/applicationcontroller.cpp \
    src/colorpath.cpp \
    src/configparser.cpp \
    src/controlwidget.cpp \
    src/cycle.cpp \
    src/cyclesearch.cpp \
    src/edge.cpp \
    src/edgewidget.cpp \
    src/factory.cpp \
    src/graphwidget.cpp \
    src/gridwidget.cpp \
    src/histogramwidget.cpp \
    src/imageoperation.cpp \
    src/imageoperationnode.cpp \
    src/main.cpp \
    src/message.cpp \
    src/midicontrol.cpp \
    src/midiinputport.cpp \
    src/midilinkmanager.cpp \
    src/midilistwidget.cpp \
    src/morphowidget.cpp \
    src/node.cpp \
    src/nodemanager.cpp \
    src/operationbuilder.cpp \
    src/operationparser.cpp \
    src/operationwidget.cpp \
    src/overlay.cpp \
    src/parameters/baseuniformparameter.cpp \
    src/parameters/optionsparameter.cpp \
    src/parameters/uniformmat4parameter.cpp \
    src/parameters/uniformparameter.cpp \
    src/plotswidget.cpp \
    src/recorder.cpp \
    src/rendermanager.cpp \
    src/rgbwidget.cpp \
    src/seed.cpp \
    src/seedwidget.cpp \
    src/videoinputcontrol.cpp \
    src/widgets/uniformmat4widget.cpp \
    src/widgets/uniformwidget.cpp
