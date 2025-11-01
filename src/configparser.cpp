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



#include "configparser.h"
#include "operationparser.h"

#include <QFile>



void ConfigurationParser::write(QString filename)
{
    QFile outFile(filename);
    if (outFile.open(QIODevice::WriteOnly))
    {
        QXmlStreamWriter mStream;
        mStream.setDevice(&outFile);
        mStream.setAutoFormatting(true);

        mStream.writeStartDocument();

        mStream.writeStartElement("fosforo");

        mStream.writeAttribute("version", mNodeManager->version);

        // Display

        writeDisplay(mStream);

        // Nodes: seeds and operation nodes

        mStream.writeStartElement("nodes");

        for (auto [id, seed] : mNodeManager->seedsMap().asKeyValueRange()) {
            writeSeedNode(id, seed, mStream);
        }

        foreach (ImageOperationNode* node, mNodeManager->operationNodesMap()) {
            writeOperationNode(node, mStream);
        }

        mStream.writeEndElement();

        // Midi

        if (mMidiLinkManager->enabled()) {
            writeMidiData(mStream);
        }

        mStream.writeEndElement();

        mStream.writeEndDocument();

        outFile.close();
    }
}



void ConfigurationParser::writeDisplay(QXmlStreamWriter& stream)
{
    stream.writeStartElement("display");

    stream.writeStartElement("size");
    stream.writeStartElement("width");
    stream.writeCharacters(QString::number(mRenderManager->texWidth()));
    stream.writeEndElement();
    stream.writeStartElement("height");
    stream.writeCharacters(QString::number(mRenderManager->texHeight()));
    stream.writeEndElement();
    stream.writeEndElement();

    QRectF sceneRect = mGraphWidget->mapToScene(mGraphWidget->viewport()->rect()).boundingRect();

    stream.writeStartElement("scene");
    stream.writeStartElement("x");
    stream.writeCharacters(QString::number(sceneRect.x()));
    stream.writeEndElement();
    stream.writeStartElement("y");
    stream.writeCharacters(QString::number(sceneRect.y()));
    stream.writeEndElement();
    stream.writeStartElement("width");
    stream.writeCharacters(QString::number(sceneRect.width()));
    stream.writeEndElement();
    stream.writeStartElement("height");
    stream.writeCharacters(QString::number(sceneRect.height()));
    stream.writeEndElement();
    stream.writeEndElement();

    stream.writeStartElement("output_node");
    stream.writeCharacters(mNodeManager->outputId().toString());
    stream.writeEndElement();

    stream.writeEndElement();
}



void ConfigurationParser::writeSeedNode(QUuid id, Seed* seed, QXmlStreamWriter &stream)
{
    stream.writeStartElement("seed_node");

    stream.writeAttribute("id", id.toString());

    stream.writeStartElement("type");
    stream.writeCharacters(QString::number(seed->type()));
    stream.writeEndElement();

    stream.writeStartElement("fixed");
    stream.writeCharacters(QString::number(seed->fixed()));
    stream.writeEndElement();

    stream.writeStartElement("image_filename");
    stream.writeCharacters(seed->imageFilename());
    stream.writeEndElement();

    QPointF position = mGraphWidget->nodePosition(id);

    stream.writeStartElement("position");

    stream.writeStartElement("x");
    stream.writeCharacters(QString::number(position.x()));
    stream.writeEndElement();

    stream.writeStartElement("y");
    stream.writeCharacters(QString::number(position.y()));
    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndElement();
}



void ConfigurationParser::writeOperationNode(ImageOperationNode* node, QXmlStreamWriter &stream)
{
    stream.writeStartElement("operation_node");

    stream.writeAttribute("id", node->id().toString());

    OperationParser opParser;

    opParser.writeOperation(node->operation(), stream, true);

    stream.writeStartElement("inputs");

    for (auto [id, inData]: node->inputs().asKeyValueRange())
    {
        stream.writeStartElement("input");

        stream.writeAttribute("id", id.toString());

        stream.writeStartElement("type");
        stream.writeCharacters(QString::number(static_cast<int>(inData->type())));
        stream.writeEndElement();

        stream.writeStartElement("blendfactor");
        stream.writeCharacters(QString::number(inData->blendFactor()->value()));
        stream.writeEndElement();

        stream.writeEndElement();
    }

    stream.writeEndElement();

    QPointF position = mGraphWidget->nodePosition(node->id());

    stream.writeStartElement("position");

    stream.writeStartElement("x");
    stream.writeCharacters(QString::number(position.x()));
    stream.writeEndElement();

    stream.writeStartElement("y");
    stream.writeCharacters(QString::number(position.y()));
    stream.writeEndElement();

    stream.writeEndElement();

    stream.writeEndElement();
}



void ConfigurationParser::writeMidiData(QXmlStreamWriter& stream)
{
    stream.writeStartElement("midi");

    // Links

    stream.writeStartElement("links");

    stream.writeAttribute("multi_link", QString::number(mMidiLinkManager->multiLink()));

    for (auto [portName, links]: mMidiLinkManager->floatLinks().asKeyValueRange())
    {
        for (auto [key, number]: links.asKeyValueRange())
        {
            stream.writeStartElement("link");
            stream.writeAttribute("type", "float");
            stream.writeAttribute("port_name", portName);
            stream.writeAttribute("key", QString::number(key));
            stream.writeAttribute("number_id", number->id().toString());
            stream.writeEndElement();
        }
    }

    for (auto [portName, links]: mMidiLinkManager->intLinks().asKeyValueRange())
    {
        for (auto [key, number]: links.asKeyValueRange())
        {
            stream.writeStartElement("link");
            stream.writeAttribute("type", "int");
            stream.writeAttribute("port_name", portName);
            stream.writeAttribute("key", QString::number(key));
            stream.writeAttribute("number_id", number->id().toString());
            stream.writeEndElement();
        }
    }

    for (auto [portName, links]: mMidiLinkManager->uintLinks().asKeyValueRange())
    {
        for (auto [key, number]: links.asKeyValueRange())
        {
            stream.writeStartElement("link");
            stream.writeAttribute("type", "uint");
            stream.writeAttribute("port_name", portName);
            stream.writeAttribute("key", QString::number(key));
            stream.writeAttribute("number_id", number->id().toString());
            stream.writeEndElement();
        }
    }

    stream.writeEndElement();

    stream.writeEndElement();
}



void ConfigurationParser::read(QString filename)
{
    QFile inFile(filename);
    if (inFile.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader mStream;
        mStream.setDevice(&inFile);

        OperationParser opParser;

        if (mStream.readNextStartElement() && mStream.name() == "fosforo")
        {
            mFactory->clear();

            QUuid outputNodeId;
            int width = mRenderManager->texWidth();
            int height = mRenderManager->texHeight();

            while (mStream.readNextStartElement())
            {
                if (mStream.name() == "nodes")
                {
                    QMap<QUuid, QMap<QUuid, InputData*>> connections;

                    while (mStream.readNextStartElement())
                    {
                        if (mStream.name() == "seed_node") {
                            readSeedNode(mStream);
                        }
                        else if (mStream.name() == "operation_node") {
                            readOperationNode(connections, mStream);
                        }
                        else {
                            mStream.skipCurrentElement();
                        }
                    }

                    mNodeManager->connectOperations(connections);
                    mNodeManager->sortOperations();
                }
                else if (mStream.name() == "display") {
                    readDisplay(width, height, outputNodeId, mStream);
                }
                else if (mStream.name() == "midi") {
                    readMidiData(mStream);
                }
                else {
                    mStream.skipCurrentElement();
                }
            }

            if (!outputNodeId.isNull()) {
                mNodeManager->setOutput(outputNodeId);
            }

            emit newImageSizeRead(width, height);
        }

        if (mStream.tokenType() == QXmlStreamReader::Invalid) {
            mStream.readNext();
        }

        if (mStream.hasError()) {
            mStream.raiseError();
        }

        inFile.close();
    }
}



void ConfigurationParser::readDisplay(int &width, int &height, QUuid &outputNodeId, QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "size")
        {
            while (stream.readNextStartElement())
            {
                if (stream.name() == "width") {
                    width = stream.readElementText().toInt();
                }
                else if (stream.name() == "height") {
                    height = stream.readElementText().toInt();
                }
                else {
                    stream.skipCurrentElement();
                }
            }
        }
        else if (stream.name() == "scene") {

            QRectF sceneRect;

            while (stream.readNextStartElement())
            {
                if (stream.name() == "x") {
                    sceneRect.setX(stream.readElementText().toFloat());
                }
                else if (stream.name() == "y") {
                    sceneRect.setY(stream.readElementText().toFloat());
                }
                else if (stream.name() == "width") {
                    sceneRect.setWidth(stream.readElementText().toFloat());
                }
                else if (stream.name() == "height") {
                    sceneRect.setHeight(stream.readElementText().toFloat());
                }
                else {
                    stream.skipCurrentElement();
                }
            }

            mGraphWidget->fitInView(sceneRect);
        }
        else if (stream.name() == "output_node") {
            outputNodeId = QUuid(stream.readElementText());
        }
        else {
            stream.skipCurrentElement();
        }
    }
}



void ConfigurationParser::readSeedNode(QXmlStreamReader& stream)
{
    QUuid id = QUuid(stream.attributes().value("id").toString());
    int type = 0;
    bool fixed = false;
    QString imageFilename;
    QPointF position(0.0, 0.0);

    OperationParser opParser;

    while (stream.readNextStartElement())
    {
        if (stream.name() == "type") {
            type = stream.readElementText().toInt();
        }
        else if (stream.name() == "fixed") {
            fixed = stream.readElementText().toInt();
        }
        else if (stream.name() == "image_filename") {
            imageFilename = stream.readElementText();
        }
        else if (stream.name() == "position")
        {
            while (stream.readNextStartElement())
            {
                if (stream.name() == "x") {
                    position.setX(stream.readElementText().toFloat());
                }
                else if (stream.name() == "y") {
                    position.setY(stream.readElementText().toFloat());
                }
                else {
                    stream.skipCurrentElement();
                }
            }
        }
        else {
            stream.skipCurrentElement();
        }
    }

    Seed* seed = new Seed(type, fixed, imageFilename);
    mFactory->addSeed(id, seed);
    mGraphWidget->setNodePosition(id, position);
}



void ConfigurationParser::readOperationNode(QMap<QUuid, QMap<QUuid, InputData*>> &connections, QXmlStreamReader& stream)
{
    QUuid id = QUuid(stream.attributes().value("id").toString());
    ImageOperation* operation = new ImageOperation();
    QMap<QUuid, InputData*> inputs;
    QPointF position(0.0, 0.0);

    OperationParser opParser;

    while (stream.readNextStartElement())
    {
        if (stream.name() == "operation") {
            opParser.readOperation(operation, stream, true);
        }
        else if (stream.name() == "inputs")
        {
            while (stream.readNextStartElement())
            {
                if (stream.name() == "input")
                {
                    QUuid srcId = QUuid(stream.attributes().value("id").toString());
                    InputType type = InputType::Normal;
                    float blendFactor = 1.0;

                    while (stream.readNextStartElement())
                    {
                        if (stream.name() == "type") {
                            type = static_cast<InputType>(stream.readElementText().toInt());
                        }
                        else if (stream.name() == "blendfactor") {
                            blendFactor = stream.readElementText().toFloat();
                        }
                        else {
                            stream.skipCurrentElement();
                        }
                    }

                    inputs.insert(srcId, new InputData(type, nullptr, blendFactor));
                }
            }

            connections.insert(id, inputs);
        }
        else if (stream.name() == "position")
        {
            while (stream.readNextStartElement())
            {
                if (stream.name() == "x") {
                    position.setX(stream.readElementText().toFloat());
                }
                else if (stream.name() == "y") {
                    position.setY(stream.readElementText().toFloat());
                }
                else {
                    stream.skipCurrentElement();
                }
            }
        }
    }

    mFactory->addOperation(id, operation, position);
}



void ConfigurationParser::readMidiData(QXmlStreamReader& stream)
{
    mMidiLinkManager->clearLinks();

    while (stream.readNextStartElement())
    {
        if (stream.name() == "links")
        {
            bool multiLink = false;
            if (stream.attributes().hasAttribute("multi_link")) {
                multiLink = stream.attributes().value("multi_link").toInt();
            }
            mMidiLinkManager->setMultiLink(multiLink);

            stream.readNextStartElement();

            while (stream.name() == "link")
            {
                QString type = stream.attributes().value("type").toString();
                QString portName = stream.attributes().value("port_name").toString();
                int key = stream.attributes().value("key").toInt();
                QUuid number_id = QUuid(stream.attributes().value("number_id").toString());

                if (type == "float") {
                    Number<float>* number = mFactory->number<float>(number_id);
                    if (number) {
                        mMidiLinkManager->setupMidiLink(portName, key, number);
                    }
                }
                else if (type == "int") {
                    Number<int>* number = mFactory->number<int>(number_id);
                    if (number) {
                        mMidiLinkManager->setupMidiLink(portName, key, number);
                    }
                }
                else if (type == "uint") {
                    Number<unsigned int>* number = mFactory->number<unsigned int>(number_id);
                    if (number) {
                        mMidiLinkManager->setupMidiLink(portName, key, number);
                    }
                }

                stream.readNextStartElement();
            }
        }
        else {
            stream.skipCurrentElement();
        }
    }
}
