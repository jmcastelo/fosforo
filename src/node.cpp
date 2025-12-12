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



#include "node.h"
#include "edge.h"
//#include "generator.h"

#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsScene>
//#include <QGraphicsSceneMouseEvent>
//#include <QPainter>
#include <QStyleOption>
//#include <QFileDialog>
//#include <QWidgetAction>
//#include <QLabel>
//#include <QActionGroup>



Node::Node(QUuid id, QWidget *widget, QGraphicsItem* parent) :
    QGraphicsWidget { parent },
    mId { id },
    mWidget { widget }
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setAcceptedMouseButtons(Qt::AllButtons);
    setCacheMode(DeviceCoordinateCache);

    setZValue(-1);

    mWidget->installEventFilter(this);

    mProxyWidget = new QGraphicsProxyWidget(this);
    mProxyWidget->setWidget(mWidget);
    mProxyWidget->setPos(QPointF(0, 0));

    resize(mProxyWidget->size());
}



/*Node::~Node()
{
    // delete mProxyWidget;
    qDebug() << this;
}*/


/*void Node::closeEvent(QCloseEvent* event)
{
    mWidget->close();
    QGraphicsWidget::closeEvent(event);
}*/



void Node::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    mProxyWidget->resize(event->newSize());

    foreach (Edge *edge, edgeList)
        edge->adjust();

    QGraphicsWidget::resizeEvent(event);
}



/*QSizeF Node::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::PreferredSize)
        return preferredSize();

    return QGraphicsWidget::sizeHint(which, constraint);
}*/



bool Node::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == mWidget && event->type() == QEvent::Resize)
    {
        QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
        QSizeF newSize = resizeEvent->size();

        resize(newSize);

        return false;
    }
    return QGraphicsWidget::eventFilter(obj, event);
}



/*Node::Node(GraphWidget* graphWidget, QString text) :
    name { text },
    graph { graphWidget }
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
}*/



//Node::~Node(){}


/*
void Node::copy()
{
    graph->copyNode(this);
}



void Node::remove()
{
    graph->removeNode(this);
}



void Node::selectToConnect()
{
    graph->selectNodeToConnect(this);
}
*/


void Node::addEdge(Edge *edge)
{
    edgeList.append(edge);
}



void Node::removeEdge(Edge *edge)
{
    edgeList.removeOne(edge);
}



QVector<Edge*> Node::edges() const
{
    return edgeList;
}



bool Node::connectedTo(Node *node)
{
    foreach (Edge* edge, edgeList)
        if (edge->destNode() == node)
            return true;

    return false;
}



void Node::centerBetween(QPointF src, QPointF dst)
{
    setPos(src + 0.5 * (dst - src) - QPointF(0.5 * mWidget->width(), 0.5 * mWidget->height()));
}


/*void Node::setAsOutput()
{
    graph->generator->setOutput(id);
    graph->updateNodes();
}



QRectF Node::textBoundingRect() const
{
    if (scene())
    {
        QFontMetricsF fontMetrics(scene()->font());
        return fontMetrics.boundingRect(name);
    }
    else
        return QRectF();
}
*/


QRectF Node::boundingRect() const
{
    return mProxyWidget->boundingRect();
}



QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addRoundedRect(mWidget->rect().toRectF(), 20.0, 20.0);
    return path;
}



/*void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QRectF textRect = textBoundingRect();

    QLinearGradient gradient(textRect.topLeft(), textRect.bottomRight());
    if (graph->generator->isOutput(id))
    {
        if (option->state & QStyle::State_Sunken || option->state & QStyle::State_Selected) {
            gradient.setColorAt(0, QColor(Qt::yellow));
            gradient.setColorAt(1, QColor(Qt::red));
        } else {
            gradient.setColorAt(0, Qt::darkYellow);
            gradient.setColorAt(1, Qt::darkRed);
        }
    }
    else
    {
        if (option->state & QStyle::State_Sunken || option->state & QStyle::State_Selected) {
            gradient.setColorAt(0, QColor(Qt::cyan));
            gradient.setColorAt(1, QColor(Qt::blue));
        } else {
            gradient.setColorAt(0, Qt::darkCyan);
            gradient.setColorAt(1, Qt::darkBlue);
        }
    }
    painter->setBrush(gradient);

    if (marked)
        painter->setPen(QPen(Qt::cyan, penSize));
    else
        painter->setPen(QPen(Qt::lightGray, penSize));
    painter->drawEllipse(textRect.adjusted(-ellipseMargin, -ellipseMargin, ellipseMargin, ellipseMargin));

    painter->setFont(scene()->font());
    painter->setPen(QPen(Qt::white, 0));
    painter->drawText(textRect, Qt::AlignCenter, name);
}*/

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget)

    if (option->state & QStyle::State_Sunken || option->state & QStyle::State_Selected)
    {
        painter->setPen(QPen(QColor(128, 128, 164), 2));

        QRectF rect = mWidget->rect().toRectF();
        painter->drawRect(rect);
    }
}



QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange && scene())
    {
        QPointF newPos = value.toPointF();

        // Keep within scene

        QRectF rect = scene()->sceneRect();
        if (!rect.contains(newPos))
        {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return newPos;
        }
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        foreach (Edge* edge, edgeList) {
            edge->adjust();
        }

        // Draw graph's background

        // graph->resetCachedContent();
    }
    else if (change == QGraphicsItem::ItemSelectedChange)
    {
        if (value.toBool() == true)
        {
            setZValue(0);

            foreach (auto edge, edgeList)
            {
                if (edge->destNode() == this)
                {
                    edge->setZValue(-1);
                    edge->setWidgetVisible(true);
                    edge->adjust();
                }
            }
        }
        else
        {
            setZValue(-1);

            foreach (auto edge, edgeList)
            {
                if (edge->destNode() == this)
                {
                    edge->setZValue(-2);
                    edge->setWidgetVisible(false);
                }
            }
        }
    }

    return QGraphicsWidget::itemChange(change, value);
}



/*void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsObject::mouseReleaseEvent(event);
}*/



// Operation node
/*
OperationNode::OperationNode(GraphWidget* graphWidget, QString name) : Node(graphWidget, name)
{
    id = graph->generator->addOperation(name);
}



OperationNode::OperationNode(GraphWidget* graphWidget, QString name, QUuid uuid, QPointF pos) : Node(graphWidget, name)
{
    id = uuid;
    setPos(pos);
}



OperationNode::OperationNode(const OperationNode& node) : Node(node.graph, node.name)
{
    id = graph->generator->copyOperation(node.id);
}



OperationNode::~OperationNode()
{
     graph->onDestroyOperationNode(id);
}



bool OperationNode::hasInputs()
{
    foreach (Edge* edge, edgeList)
        if (edge->destNode() == this)
            return true;

    return false;
}



void OperationNode::renameOperation(QString newName)
{
    name = newName;

    foreach (Edge *edge, edgeList)
    {
        edge->setBlendFactorGroupBoxTitle();
        edge->adjust();
    }

    update();
    scene()->update();
}



void OperationNode::setOperation(QAction *action)
{
    graph->generator->setOperation(id, action->text());

    name = action->text();

    foreach (Edge *edge, edgeList)
        edge->adjust();

    update();
    scene()->update();

    graph->updateOperation(id);
}



void OperationNode::enableOperation(bool checked)
{
    graph->generator->enableOperation(id, checked);
    emit graph->operationEnabled(id, checked);
    if (isSelected())
        graph->newNodeSelected(this);
}



void OperationNode::equalizeBlendFactors()
{
    graph->generator->equalizeBlendFactors(id);

    foreach (Edge* edge, edgeList)
    {
        edge->update();
        edge->updateBlendFactor();
    }
}



void OperationNode::clear()
{
    graph->generator->clearOperation(id);
}



void OperationNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (graph->nodeSelectedToConnect())
            graph->connectNodes(this);
        else
            graph->deselectNodeToConnect();
    }

    update();
    QGraphicsObject::mousePressEvent(event);
}



void OperationNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    if (isSelected() && (graph->seedNodesSelected() > 0 || graph->operationNodesSelected() > 1 || graph->nodesSelected()))
    {
        if (graph->seedNodesSelected() > 0)
        {
            menu.addAction(graph->seedNodesSelected() > 1 ? "Draw seeds" : "Draw seed", graph, &GraphWidget::drawSelectedSeeds);
            menu.addSeparator();
        }

        if (graph->operationNodesSelected() > 1)
        {
            menu.addAction("Enable", graph, &GraphWidget::enableSelectedOperations);
            menu.addAction("Disable", graph, &GraphWidget::disableSelectedOperations);
            if (graph->selectedOperationNodesHaveInputs())
                menu.addAction("Equalize blend factors", graph, &GraphWidget::equalizeSelectedBlendFactors);
            menu.addSeparator();
            menu.addAction("Clear", graph, &GraphWidget::clearSelectedOperationNodes);
            menu.addSeparator();

            if (graph->twoOperationNodesSelected())
            {
                menu.addAction("Swap", graph, &GraphWidget::swapSelectedOperationNodes);
                menu.addSeparator();
            }
        }

        if (graph->nodesSelected())
        {
            menu.addAction("Copy", graph, &GraphWidget::makeNodeSnapshot);
            menu.addAction("Remove", graph, &GraphWidget::removeSelectedNodes);
        }
    }
    else
    {
        QMenu *operationsMenu = menu.addMenu("Set operation");

        foreach (QString opName, graph->generator->availableOperations)
            operationsMenu->addAction(opName);

        connect(operationsMenu, &QMenu::triggered, this, &OperationNode::setOperation);

        QAction* enableAction = menu.addAction("Enabled", this, &OperationNode::enableOperation);
        enableAction->setCheckable(true);
        enableAction->setChecked(graph->generator->isOperationEnabled(id));

        if (hasInputs())
            menu.addAction("Equalize blend factors", this, &OperationNode::equalizeBlendFactors);

        menu.addSeparator();

        menu.addAction("Set as output", this, &OperationNode::setAsOutput);

        if (graph->moreThanOneNode())
            menu.addAction("Connect to", this, &OperationNode::selectToConnect);

        menu.addSeparator();

        menu.addAction("Clear", this, &OperationNode::clear);

        menu.addSeparator();

        menu.addAction("Copy", this, &OperationNode::copy);
        menu.addAction("Remove", this, &OperationNode::remove);
    }

    // Backup scene's selected items
    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

    connect(&menu, &QMenu::aboutToHide, this, [selectedItems]()
    {
        // Restore the selection state
        for (QGraphicsItem* item : selectedItems) {
            item->setSelected(true);
        }
    });

    menu.exec(event->screenPos());
}



QVariant OperationNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == QGraphicsItem::ItemSelectedChange && value.toBool() == true) || change == QGraphicsItem::ItemPositionChange)
    {
        foreach (Edge* edge, edgeList)
        {
            if (edge->destNode() == this)
            {
                edge->drawBlendFactor(true);
                edge->update();
            }
        }
    }
    else if (change == QGraphicsItem::ItemSelectedChange && value.toBool() == false)
    {
        graph->drawBlendFactors(false);
    }

    return Node::itemChange(change, value);
}
*/


// Seed node
/*
SeedNode::SeedNode(GraphWidget* graphWidget, QString name) : Node(graphWidget, name)
{
    id = graph->generator->addSeed();
}



SeedNode::SeedNode(GraphWidget* graphWidget, QUuid uuid, QPointF pos, QString name) : Node(graphWidget, name)
{
    id = uuid;
    setPos(pos);
}



SeedNode::SeedNode(const SeedNode& node) : Node(node.graph, node.name)
{
    id = graph->generator->copySeed(node.id);
}



SeedNode::~SeedNode()
{
    graph->onDestroySeedNode(id);
}



void SeedNode::draw()
{
    graph->generator->drawSeed(id);
}



void SeedNode::setType(QAction* action)
{
    graph->generator->setSeedType(id, action->data().toInt());

    if (isSelected())
        graph->newNodeSelected(this);
}



void SeedNode::loadImage()
{
    QString filename = QFileDialog::getOpenFileName(graph, "Load image", QDir::homePath(), "Images (*.bmp *.jpeg *.jpg *.png *.tif *.tiff)");

    if (!filename.isEmpty())
    {
        graph->generator->loadSeedImage(id, filename);
    }
}



void SeedNode::setFixed(bool fixed)
{
    graph->generator->setSeedFixed(id, fixed);

    if (isSelected())
        graph->newNodeSelected(this);
}



void SeedNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        graph->deselectNodeToConnect();
    }

    update();
    QGraphicsObject::mousePressEvent(event);
}



void SeedNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    if (isSelected() && (graph->seedNodesSelected() > 0 || graph->operationNodesSelected() > 1 || graph->nodesSelected()))
    {
        if (graph->seedNodesSelected() > 0)
        {
            menu.addAction(graph->seedNodesSelected() > 1 ? "Draw seeds" : "Draw seed", graph, &GraphWidget::drawSelectedSeeds);
            menu.addSeparator();
        }

        if (graph->operationNodesSelected() > 1)
        {
            menu.addAction("Enable", graph, &GraphWidget::enableSelectedOperations);
            menu.addAction("Disable", graph, &GraphWidget::disableSelectedOperations);
            menu.addAction("Equalize blend factors", graph, &GraphWidget::equalizeSelectedBlendFactors);
            menu.addSeparator();
            menu.addAction("Clear", graph, &GraphWidget::clearSelectedOperationNodes);
            menu.addSeparator();
        }

        if (graph->nodesSelected())
        {
            menu.addAction("Copy", graph, &GraphWidget::makeNodeSnapshot);
            menu.addAction("Remove", graph, &GraphWidget::removeSelectedNodes);
        }
    }
    else
    {
        menu.addAction("Draw", this, &SeedNode::draw);

        QMenu *typeMenu = menu.addMenu("Type");

        QAction* colorAction = new QAction("Random: color");
        colorAction->setCheckable(true);
        colorAction->setData(QVariant(0));

        QAction* grayscaleAction = new QAction("Random: grayscale");
        grayscaleAction->setCheckable(true);
        grayscaleAction->setData(QVariant(1));

        QAction* imageAction = new QAction("Image");
        imageAction->setCheckable(true);
        imageAction->setData(QVariant(2));

        QActionGroup* type = new QActionGroup(this);
        type->addAction(colorAction);
        type->addAction(grayscaleAction);
        type->addAction(imageAction);

        typeMenu->addAction(colorAction);
        typeMenu->addAction(grayscaleAction);
        typeMenu->addAction(imageAction);

        connect(typeMenu, &QMenu::triggered, this, &SeedNode::setType);

        colorAction->setChecked(graph->generator->getSeedType(id) == 0);
        grayscaleAction->setChecked(graph->generator->getSeedType(id) == 1);
        imageAction->setChecked(graph->generator->getSeedType(id) == 2);

        menu.addAction("Load image", this, &SeedNode::loadImage);

        QAction* fixedAction = menu.addAction("Fixed", this, &SeedNode::setFixed);
        fixedAction->setCheckable(true);
        fixedAction->setChecked(graph->generator->isSeedFixed(id));

        menu.addSeparator();

        menu.addAction("Set as output", this, &SeedNode::setAsOutput);

        if (graph->moreThanOneNode())
            menu.addAction("Connect to", this, &SeedNode::selectToConnect);

        menu.addSeparator();

        menu.addAction("Copy", this, &SeedNode::copy);
        menu.addAction("Remove", this, &SeedNode::remove);
    }

    // Backup scene's selected items
    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();

    connect(&menu, &QMenu::aboutToHide, this, [selectedItems]()
    {
        // Restore the selection state
        for (QGraphicsItem* item : selectedItems) {
            item->setSelected(true);
        }
    });

    menu.exec(event->screenPos());
}
*/
