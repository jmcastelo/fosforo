/*
*  Copyright 2025 Jose Maria Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/fosforo>
*
*  This file is part of Fosforo.
*
*  Fosforo is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Fosforo is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Fosforo.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once



#include "factory.h"
#include "nodemanager.h"
#include "rendermanager.h"
#include "graphwidget.h"
#include "midilinkmanager.h"

#include <QObject>
#include <QString>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>



class ConfigurationParser: public QObject
{
    Q_OBJECT

public:
    ConfigurationParser(Factory* factory, NodeManager* nodeManager, RenderManager* renderManager, GraphWidget* graphWidget, MidiLinkManager* midiLinkManager) :
    mFactory { factory },
    mNodeManager { nodeManager },
    mRenderManager { renderManager},
    mGraphWidget { graphWidget },
    mMidiLinkManager { midiLinkManager }
    {}

signals:
    void newImageSizeRead(int width, int height);

public slots:
    void write(QString filename);
    void read(QString filename);

private:
    Factory* mFactory;
    NodeManager* mNodeManager;
    RenderManager* mRenderManager;
    GraphWidget* mGraphWidget;
    MidiLinkManager* mMidiLinkManager;

    void writeDisplay(QXmlStreamWriter& stream);
    void writeSeedNode(QUuid id, Seed* seed, QXmlStreamWriter& stream);
    void writeOperationNode(ImageOperationNode* node, QXmlStreamWriter& stream);
    void writeMidiData(QXmlStreamWriter& stream);

    void readDisplay(int& width, int& height, QUuid &outputNodeId, QXmlStreamReader& stream);
    void readSeedNode(QXmlStreamReader& stream);
    void readOperationNode(QMap<QUuid, QMap<QUuid, InputData*>>& connections, QXmlStreamReader& stream);
    void readMidiData(QXmlStreamReader& stream);
};
