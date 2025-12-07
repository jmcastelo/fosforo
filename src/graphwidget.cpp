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



#include "graphwidget.h"
#include "node.h"
#include "edge.h"
#include "edgewidget.h"
#include "cycle.h"
#include "cyclesearch.h"

#include <QKeyEvent>
#include <QRandomGenerator>
#include <QMenu>
#include <QAction>
#include <QPainterPath>
#include <QPointer>



GraphWidget::GraphWidget(Factory *factory, NodeManager* nodeManager, QWidget *parent) :
    QGraphicsView { parent },
    mFactory { factory },
    mNodeManager { nodeManager }
{
    // Graphics scene

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setFont(QFont("Arial", 8, QFont::Light));
    scene->setSceneRect(0.0, 0.0, 10'000.0, 10'000.0);
    setScene(scene);

    connect(scene, &QGraphicsScene::selectionChanged, this, &GraphWidget::newSelectedNodes);

    setDragMode(RubberBandDrag);
    setRubberBandSelectionMode(Qt::ContainsItemShape);

    // setOptimizationFlags(DontAdjustForAntialiasing | DontSavePainterState);

    setCacheMode(CacheBackground);
    // setViewportUpdateMode(BoundingRectViewportUpdate);
    setViewportUpdateMode(FullViewportUpdate);
    // setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setRenderHints(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::ClickFocus);

    scale(qreal(1.0), qreal(1.0));
    center = sceneRect().center();
    centerOn(sceneRect().center());

    // Menu

    mMainMenu = new QMenu(this);

    mAddSeedAction = new QAction("Add seed");
    mMainMenu->addActions({mAddSeedAction});

    mAvailOpsMenu = new QMenu("Add operation");
    populateAvailOpsMenu();
    mMainMenu->addMenu(mAvailOpsMenu);

    mMainMenu->addSeparator();

    mBuildNewOpAction = new QAction("Build new operation");
    mMainMenu->addActions({mBuildNewOpAction});

    mPasteNodesAction = new QAction("Paste nodes");

    // Connections

    connect(mFactory, QOverload<QUuid, OperationWidget*>::of(&Factory::newOpWidgetCreated), this, &GraphWidget::addNewNode);
    connect(mFactory, QOverload<QUuid, OperationWidget*, QPointF>::of(&Factory::newOpWidgetCreated), this, &GraphWidget::addNewNodeOnPos);
    connect(mFactory, &Factory::newSeedWidgetCreated, this, &GraphWidget::addNewNode);
    connect(mFactory, &Factory::cleared, this, &GraphWidget::clearScene);

    connect(mNodeManager, &NodeManager::nodesConnected, this, &GraphWidget::connectNodes);
    connect(mNodeManager, &NodeManager::removeNodes, this, &GraphWidget::removeNodes);
    connect(mNodeManager, &NodeManager::nodeRemoved, this, &GraphWidget::removeNode);
    connect(mNodeManager, &NodeManager::nodesDisconnected, this, &GraphWidget::removeEdge);
    connect(mNodeManager, &NodeManager::nodeInserted, this, &GraphWidget::centerNodeBetween);
    connect(mNodeManager, &NodeManager::nodesCopied, this, &GraphWidget::setCopiedNodes);

    connect(mMainMenu, &QMenu::triggered, this, &GraphWidget::onActionTriggered);
}



GraphWidget::~GraphWidget()
{
    //disconnect(scene(), &QGraphicsScene::selectionChanged, this, &GraphWidget::newSelectedNodes);

    delete mAddSeedAction;
    delete mBuildNewOpAction;
    qDeleteAll(mAvailOpsActions);
    delete mAvailOpsMenu;
    delete mMainMenu;

    scene()->clear();
}



void GraphWidget::populateAvailOpsMenu()
{
    mAvailOpsMenu->clear();

    qDeleteAll(mAvailOpsActions);
    mAvailOpsActions.clear();

    mFactory->scan();

    foreach (QString opName, mFactory->availableOperationNames()){
        mAvailOpsActions.append(new QAction(opName));
    }

    mAvailOpsMenu->addActions(mAvailOpsActions);
}



void GraphWidget::enablePasteAction()
{
    if (!mCopiedOperations.isEmpty() || !mCopiedSeeds.isEmpty()) {
        mMainMenu->addAction(mPasteNodesAction);
    }
    else {
        mMainMenu->removeAction(mPasteNodesAction);
    }
}



Node* GraphWidget::getNode(QUuid id)
{
    const QList<QGraphicsItem*> items = scene()->items();

    foreach (QGraphicsItem* item, items)
    {
        if (Node* node = qgraphicsitem_cast<Node*>(item))
        {
            if (node->id() == id) {
                return node;
            }
        }
    }

    return nullptr;
}



Edge* GraphWidget::getEdge(QUuid srcId, QUuid dstId)
{
    const QList<QGraphicsItem*> items = scene()->items();

    foreach (QGraphicsItem* item, items)
    {
        if (Edge* edge = qgraphicsitem_cast<Edge*>(item))
        {
            if (edge->sourceNode()->id() == srcId && edge->destNode()->id() == dstId) {
                return edge;
            }
        }
    }

    return nullptr;
}



void GraphWidget::onActionTriggered(QAction* action)
{
    if (action == mAddSeedAction) {
        mFactory->createNewSeed();
    }
    else if (action == mBuildNewOpAction) {
        mFactory->createNewOperation();
    }
    else if (mAvailOpsActions.contains(action))
    {
        int index = mAvailOpsActions.indexOf(action);
        mFactory->addAvailableOperation(index);
    }
    else if (action == mPasteNodesAction)
    {
        pasteNodes();
    }
}



void GraphWidget::closeEvent(QCloseEvent* event)
{
    /*QList<QPointer<Node>> watchers;

    for (auto* item : scene()->items()) {
        if (Node* node = qgraphicsitem_cast<Node*>(item)) {
            watchers.append(QPointer<Node>(node));
        }
    }

    foreach (QPointer<Node> wptr, watchers) {
        if (!wptr.isNull()) {
            wptr->close();
        }
    }*/

    foreach (auto* item, scene()->items()) {
        if (Node* node = qgraphicsitem_cast<Node*>(item)) {
            node->close();
        }
    }

    QGraphicsView::closeEvent(event);
}



/*void GraphWidget::closeWidgets()
{
    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
        if (Edge* edge = qgraphicsitem_cast<Edge*>(item))
            edge->closeBlendFactorWidget();
}*/



/*void GraphWidget::newSelectedNodes()
{
    selectedOperationNodes.clear();
    selectedSeedNodes.clear();

    const QList<QGraphicsItem*> items = scene()->selectedItems();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            selectedOperationNodes << node;
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            selectedSeedNodes << node;
    }

    if (selectedSeedNodes.size() + selectedOperationNodes.size() > 1)
        emit multipleNodesSelected();
    else if (selectedSeedNodes.size() == 1 && selectedOperationNodes.empty())
        emit singleNodeSelected(selectedSeedNodes.front());
    else if (selectedOperationNodes.size() == 1 && selectedSeedNodes.empty())
    {
        emit singleNodeSelected(selectedOperationNodes.front());
        emit operationNodeSelected(selectedOperationNodes.front()->id);
    }
    else if (selectedSeedNodes.empty() && selectedOperationNodes.empty())
        emit singleNodeSelected(nullptr);

    if (selectedOperationNodes.empty())
        emit noOperationNodesSelected();
}*/

void GraphWidget::newSelectedNodes()
{
    QList<QUuid> selNodeIds;

    const QList<QGraphicsItem*> items = scene()->selectedItems();

    for (QGraphicsItem* item : items) {
        if (Node* node = qgraphicsitem_cast<Node*>(item)) {
            selNodeIds.append(node->id());
        }
    }

    emit selectedNodesChanged(selNodeIds);
}


/*void GraphWidget::addOperationNodeUnderCursor(QAction *action)
{
    Node *node = new OperationNode(this, action->whatsThis());
    scene()->addItem(node);
    node->setPos(mapToScene(mapFromGlobal(action->data().toPoint())));
    emit showOperationParameters(node->id);
}*/



void GraphWidget::addNewNode(QUuid id, QWidget* widget)
{
    Node* node = new Node(id, widget);
    scene()->addItem(node);
    node->setPos(mClickPoint);

    //QGraphicsProxyWidget *proxyWidget = scene()->addWidget(mNodeManager->addNewOperation());
    //proxyWidget->setPos(mapToScene(mapFromGlobal(data.toPoint())));

    /*Node* proxyWidget = new Node(mNodeManager->addNewOperation());
    scene()->addItem(proxyWidget);
    proxyWidget->setPos(mapToScene(mapFromGlobal(data.toPoint())));*/
}



void GraphWidget::addNewNodeOnPos(QUuid id, QWidget* widget, QPointF pos)
{
    Node* node = new Node(id, widget);
    scene()->addItem(node);
    node->setPos(pos);

    //QGraphicsProxyWidget *proxyWidget = scene()->addWidget(mNodeManager->addNewOperation());
    //proxyWidget->setPos(mapToScene(mapFromGlobal(data.toPoint())));

    /*Node* proxyWidget = new Node(mNodeManager->addNewOperation());
    scene()->addItem(proxyWidget);
    proxyWidget->setPos(mapToScene(mapFromGlobal(data.toPoint())));*/
}



/*void GraphWidget::addSeedNode()
{
    QAction* action = qobject_cast<QAction*>(sender());
    QVariant data = action->data();

    Node *node = new Node(mNodeManager->addSeed());
    scene()->addItem(node);
    node->setPos(mapToScene(mapFromGlobal(data.toPoint())));
}*/



void GraphWidget::connectNodes(QUuid srcId, QUuid dstId, InputType type, EdgeWidget* widget)
{
    Node* srcNode = getNode(srcId);
    Node* dstNode = getNode(dstId);

    if (srcNode && dstNode && srcNode != dstNode)
    {
        Edge* edge = new Edge(srcNode, dstNode, type == InputType::Blit, widget);
        scene()->addItem(edge);
        searchElementaryCycles();
    }
}



void GraphWidget::reconnectNodes(Node* node)
{
    if (node->edges().size() == 2)
    {
        Edge* inEdge = nullptr;
        Edge* outEdge = nullptr;

        // Identify input and output edges

        if (node->edges().constFirst()->destNode() == node && node->edges().constLast()->sourceNode() == node)
        {
            inEdge = node->edges().constFirst();
            outEdge = node->edges().constLast();
        }
        else if (node->edges().constLast()->destNode() == node && node->edges().constFirst()->sourceNode() == node)
        {
            inEdge = node->edges().constLast();
            outEdge = node->edges().constFirst();
        }

        // Avoid connecting node with itself

        if (inEdge && outEdge && (inEdge->sourceNode() != outEdge->destNode()))
        {
            // Avoid connecting already connected nodes

            foreach (Edge* edge, inEdge->sourceNode()->edges())
            {
                if (edge->destNode() == outEdge->destNode())
                    return;
            }

            // Reconnect nodes

            QUuid srcId = inEdge->sourceNode()->id();
            QUuid dstId = outEdge->destNode()->id();
            float blendFactor = mNodeManager->blendFactor(inEdge->sourceNode()->id(), node->id())->value();

            mNodeManager->connectOperations(srcId, dstId, blendFactor);
        }
    }
}



/*void GraphWidget::selectNode(QUuid id, bool selected)
{
    const QList<QGraphicsItem*> items = scene()->items();

    foreach (QGraphicsItem* item, items)
    {
        if (OperationNode *node = qgraphicsitem_cast<OperationNode*>(item))
            if (node->id == id)
                node->setSelected(selected);
    }
}*/



void GraphWidget::removeNodes(QList<QUuid> ids)
{
    foreach (QUuid id, ids) {
        removeNode(id);
    }
}


void GraphWidget::removeNode(QUuid id)
{
    Node* node = getNode(id);

    if (node)
    {
        reconnectNodes(node);

        foreach (Edge *edge, node->edges())
        {
            if (edge->sourceNode()) {
                edge->sourceNode()->removeEdge(edge);
            }
            if (edge->destNode()) {
                edge->destNode()->removeEdge(edge);
            }

            scene()->removeItem(edge);
            delete edge;
        }

        scene()->removeItem(node);
        node->deleteLater();
        // delete node;

        searchElementaryCycles();
    }
}



void GraphWidget::removeEdge(QUuid srcId, QUuid dstId)
{
    Edge* edge = getEdge(srcId, dstId);

    if (edge)
    {
        edge->sourceNode()->removeEdge(edge);
        edge->destNode()->removeEdge(edge);

        scene()->removeItem(edge);
        edge->deleteLater();

        searchElementaryCycles();
    }
}


/*void GraphWidget::removeEdge(Edge *edge)
{
    generator->disconnectOperations(edge->sourceNode()->id, edge->destNode()->id);

    scene()->removeItem(edge);
    delete edge;

    searchElementaryCycles();
}



void GraphWidget::selectNodeToConnect(Node *node)
{
    selectedNode = node;
}



void GraphWidget::deselectNodeToConnect()
{
    selectedNode = nullptr;
}



bool GraphWidget::nodeSelectedToConnect()
{
    return selectedNode != nullptr;
}*/



/*bool GraphWidget::moreThanOneNode()
{
    QVector<Node*> nodes;

    const QList<QGraphicsItem*> items = scene()->items();

    foreach (QGraphicsItem* item, items)
    {
        if (OperationNode *node = qgraphicsitem_cast<OperationNode*>(item))
            nodes << node;
        else if (SeedNode *node = qgraphicsitem_cast<SeedNode*>(item))
            nodes << node;
    }

    return nodes.size() > 1;
}*/



/*void GraphWidget::newNodeSelected(Node* node)
{
    emit singleNodeSelected(node);
}*/



/*bool GraphWidget::nodesSelected()
{
    return selectedOperationNodes.size() + selectedSeedNodes.size() > 1;
}*/



/*bool GraphWidget::singleOperationNodeSelected()
{
    return (selectedOperationNodes.size() == 1) && (selectedSeedNodes.size() == 0);
}*/



/*bool GraphWidget::isOperationNodeSelected(QUuid id)
{
    foreach (OperationNode* node, selectedOperationNodes)
        if (node->id == id)
            return true;
    return false;
}*/



/*int GraphWidget::operationNodesSelected()
{
    return selectedOperationNodes.size();
}*/



/*bool GraphWidget::twoOperationNodesSelected()
{
    return selectedOperationNodes.size() == 2;
}*/



/*int GraphWidget::seedNodesSelected()
{
    return selectedSeedNodes.size();
}*/



/*void GraphWidget::drawSelectedSeeds()
{
    foreach (SeedNode* node, selectedSeedNodes)
        generator->drawSeed(node->id);
}*/


/*
void GraphWidget::enableSelectedOperations()
{
    foreach (OperationNode* node, selectedOperationNodes)
    {
        generator->enableOperation(node->id, true);
        emit operationEnabled(node->id, true);
    }
}



void GraphWidget::disableSelectedOperations()
{
    foreach (OperationNode* node, selectedOperationNodes)
    {
        generator->enableOperation(node->id, false);
        emit operationEnabled(node->id, false);
    }
}



bool GraphWidget::selectedOperationNodesHaveInputs()
{
    foreach (OperationNode* node, selectedOperationNodes)
        if (node->hasInputs())
            return true;
    return false;
}



void GraphWidget::equalizeSelectedBlendFactors()
{
    foreach (OperationNode* node, selectedOperationNodes)
        generator->equalizeBlendFactors(node->id);

    updateNodes();
}



void GraphWidget::clearSelectedOperationNodes()
{
    foreach (OperationNode* node, selectedOperationNodes)
        generator->clearOperation(node->id);

}



void GraphWidget::swapSelectedOperationNodes()
{
    if (selectedOperationNodes.size() == 2)
    {
        generator->swapTwoOperations(selectedOperationNodes[0]->id, selectedOperationNodes[1]->id);

        QString name = selectedOperationNodes[0]->name;
        selectedOperationNodes[0]->renameOperation(selectedOperationNodes[1]->name);
        selectedOperationNodes[1]->renameOperation(name);

        emit updateOperationParameters(selectedOperationNodes[0]->id);
        emit updateOperationParameters(selectedOperationNodes[1]->id);
    }
}



void GraphWidget::removeSelectedNodes()
{
    // Operate on copies since selected nodes change as they are removed

    QVector<OperationNode*> opNodes = selectedOperationNodes;

    for (OperationNode* node : opNodes)
        removeNode(node);

    QVector<SeedNode*> seedNodes = selectedSeedNodes;

    for (SeedNode* node : seedNodes)
        removeNode(node);
}



void GraphWidget::copyNode(Node *node)
{
    copiedNodes[0].clear();
    copiedNodes[0].insert(node->id, node);
    copyNodes(true);
}



void GraphWidget::makeNodeSnapshot()
{
    copiedNodes[0].clear();

    foreach (OperationNode* node, selectedOperationNodes)
        copiedNodes[0].insert(node->id, node);

    foreach (SeedNode* node, selectedSeedNodes)
        copiedNodes[0].insert(node->id, node);

    copyNodes(true);
}



void GraphWidget::copyNodes(bool connectionA)
{
    // Compute coordinates relative to center of copied nodes

    QPointF center(0.0, 0.0);

    foreach (Node* node, copiedNodes[0])
        center += node->pos();

    center /= copiedNodes[0].size();

    QMap<QUuid, QPointF> relativeCoords;

    foreach (Node* node, copiedNodes[0])
        relativeCoords.insert(node->id, node->pos() - center);

    // Copy-construct new nodes
    // Isomorphism relates copied and new nodes

    QMap<QUuid, QUuid> isomorphism;

    copiedNodes[1].clear();

    foreach (Node* srcNode, copiedNodes[0])
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(srcNode))
        {
            OperationNode* dstNode  = new OperationNode(*node);
            dstNode->setPos(relativeCoords.value(node->id));
            isomorphism.insert(node->id, dstNode->id);
            copiedNodes[1].insert(dstNode->id, dstNode);
        }
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(srcNode))
        {
            SeedNode* dstNode  = new SeedNode(*node);
            dstNode->setPos(relativeCoords.value(node->id));
            isomorphism.insert(node->id, dstNode->id);
            copiedNodes[1].insert(dstNode->id, dstNode);
        }
    }

    // Link new nodes according to isomorphism

    newEdges.clear();

    foreach (Node* node, copiedNodes[0])
    {
        foreach (Edge* edge, node->edges())
        {
            QUuid sourceId = edge->sourceNode()->id;
            QUuid destId = edge->destNode()->id;

            if (copiedNodes[0].contains(sourceId) && copiedNodes[0].contains(destId))
            {
                QUuid newSourceId = isomorphism.value(sourceId);
                QUuid newDestId = isomorphism.value(destId);

                if (connectionA)
                    generator->connectCopiedOperationsA(sourceId, destId, newSourceId, newDestId);
                else
                    generator->connectCopiedOperationsB(sourceId, destId, newSourceId, newDestId);

                Edge* newEdge = new Edge(this, copiedNodes[1].value(newSourceId), copiedNodes[1].value(newDestId));
                newEdge->setPredge(edge->isPredge());
                newEdges.push_back(newEdge);
            }
        }
    }
}



void GraphWidget::pasteCopiedNodes()
{
    generator->pasteOperations();

    // Get cursor position in scene coordinate system

    QAction* action = qobject_cast<QAction*>(sender());
    QVariant data = action->data();
    QPoint cursorPos = data.toPoint();
    QPointF origin = mapToScene(mapFromGlobal(cursorPos));

    // Add items to scene

    foreach (Node* node, copiedNodes[1])
    {
        scene()->addItem(node);
        node->setPos(origin + node->pos());

        if (OperationNode* opNode = qgraphicsitem_cast<OperationNode*>(node))
            emit showOperationParameters(opNode->id);
    }

    foreach (Edge* edge, newEdges)
    {
        scene()->addItem(edge);
        edge->adjust();
        edge->constructBlendFactorWidget();
    }

    copiedNodes[0] = copiedNodes[1];

    copyNodes(false);

    searchElementaryCycles();
}



void GraphWidget::connectNodes(Node *node)
{
    if (!selectedNode->connectedTo(node))
    {
        generator->connectOperations(selectedNode->id, node->id, 1.0f);
        scene()->addItem(new Edge(this, selectedNode, node));
        searchElementaryCycles();
    }

    selectedNode = nullptr;
}



void GraphWidget::insertNodeBetween(QAction* action, Edge* edge)
{
    Node *node = new OperationNode(this, action->text());

    emit showOperationParameters(node->id);

    generator->connectOperations(edge->sourceNode()->id, node->id, generator->blendFactor(edge->sourceNode()->id, edge->destNode()->id));
    generator->connectOperations(node->id, edge->destNode()->id, generator->blendFactor(edge->sourceNode()->id, edge->destNode()->id));

    scene()->addItem(node);
    node->setPos(mapToScene(mapFromGlobal(action->data().toPoint())));

    scene()->addItem(new Edge(this, edge->sourceNode(), node));

    Edge* output = new Edge(this, node, edge->destNode());
    if (edge->isPredge())
        output->setPredge(true);

    scene()->addItem(output);

    edge->remove();
}



void GraphWidget::updateOperation(QUuid id)
{
    emit updateOperationParameters(id);
}



void GraphWidget::onDestroyOperationNode(QUuid id)
{
    generator->removeOperation(id);
    emit removeOperationParameters(id);
}



void GraphWidget::onDestroySeedNode(QUuid id)
{
    generator->removeSeed(id);
}



void GraphWidget::updateNodes()
{
    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            node->update();
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            node->update();
    }
}



void GraphWidget::drawBlendFactors(bool draw)
{
    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (Edge* edge = qgraphicsitem_cast<Edge*>(item))
        {
            edge->drawBlendFactor(draw);
            edge->update();
        }
    }
}
*/


QPointF GraphWidget::nodePosition(QUuid id)
{
    QPointF position;

    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (Node* node = qgraphicsitem_cast<Node*>(item))
        {
            if (node->id() == id)
            {
                position = node->pos();
                break;
            }
        }
    }

    return position;
}



void GraphWidget::setNodePosition(QUuid id, QPointF position)
{
    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (Node* node = qgraphicsitem_cast<Node*>(item))
        {
            if (node->id() == id)
            {
                node->setPos(position);
                break;
            }
        }
    }
}



void GraphWidget::centerNodeBetween(QUuid srcId, QUuid dstId, QUuid opId)
{
    Edge* edge = getEdge(srcId, dstId);
    Node* node = getNode(opId);

    if (edge && node) {
        node->centerBetween(edge->srcPoint(), edge->dstPoint());
    }
}



void GraphWidget::setCopiedNodes(QMap<QUuid, ImageOperation*> opsMap, QMap<QUuid, Seed*> seedsMap)
{
    mCopiedOperations = opsMap;
    mCopiedSeeds = seedsMap;

    mCopiedNodePositions.clear();

    QPointF center;
    foreach (QUuid id, mCopiedOperations.keys()) {
        center += nodePosition(id);
    }
    foreach (QUuid id, mCopiedSeeds.keys()) {
        center += nodePosition(id);
    }
    center /= (mCopiedOperations.size() + mCopiedSeeds.size());

    foreach (QUuid id, mCopiedOperations.keys()) {
        mCopiedNodePositions.insert(id, nodePosition(id)- center);
    }
    foreach (QUuid id, mCopiedSeeds.keys()) {
        mCopiedNodePositions.insert(id, nodePosition(id) - center);
    }
}



void GraphWidget::pasteNodes()
{
    QMap<QUuid, QUuid> isomorphism;

    for (auto [id, operation] : mCopiedOperations.asKeyValueRange())
    {
        QUuid newId = QUuid::createUuid();
        ImageOperation* newOperation = new ImageOperation(*operation);
        mFactory->addOperation(newId, newOperation, mClickPoint + mCopiedNodePositions.value(id));
        isomorphism.insert(id, newId);
    }
    for (auto [id, seed] : mCopiedSeeds.asKeyValueRange())
    {
        QUuid newId = QUuid::createUuid();
        Seed* newSeed = new Seed(seed->type(), seed->fixed(), seed->imageFilename());
        mFactory->addSeed(newId, newSeed);
        newSeed->draw();
        setNodePosition(newId, mClickPoint + mCopiedNodePositions.value(id));
        isomorphism.insert(id, newId);
    }

    mNodeManager->connectPastedNodes(isomorphism);
}


void GraphWidget::clearScene()
{
    scene()->clear();
}


/*
void GraphWidget::loadSeedNode(QUuid id, QPointF position)
{
    Node *node = new SeedNode(this, id, position);
    scene()->addItem(node);
}



void GraphWidget::loadOperationNode(QUuid id, QString name, QPointF position)
{
    Node *node = new OperationNode(this, name, id, position);
    scene()->addItem(node);
}



Node* GraphWidget::getNode(QUuid id)
{
    Node* found = nullptr;

    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
        {
            if (node->id == id)
            {
                found = node;
                break;
            }
        }
        if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
        {
            if (node->id == id)
            {
                found = node;
                break;
            }
        }
    }

    return found;
}



void GraphWidget::connectNodes(QMap<QUuid, QMap<QUuid, InputData*>> connections)
{
    QMap<QUuid, QMap<QUuid, InputData*>>::iterator dst = connections.begin();

    while (dst != connections.end())
    {
        Node* dstNode = getNode(dst.key());

        if (dstNode)
        {
            QMap<QUuid, InputData*>::iterator src = dst.value().begin();

            while (src != dst.value().constEnd())
            {
                Node* srcNode = getNode(src.key());

                if (srcNode)
                {
                    Edge* edge = new Edge(this, srcNode, dstNode);

                    edge->setPredge(src.value()->type == InputType::Blit);

                    scene()->addItem(edge);
                }

                src++;
            }
        }

        dst++;
    }

    searchElementaryCycles();
}
*/


void GraphWidget::markNodes(QList<QUuid> ids)
{
    const QList<QGraphicsItem*> items = scene()->items();

    for (QGraphicsItem* item : items)
    {
        if (Node* node = qgraphicsitem_cast<Node*>(item))
        {
            node->setSelected(false);

            foreach (QUuid id, ids) {
                if (node->id() == id) {
                    node->setSelected(true);
                }
            }

            node->update();
        }
    }
}



/*void GraphWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPos()))))
    {        
        QMenu menu(this);

        // Add operation

        QMenu *operationsMenu = menu.addMenu("Add operation");

        foreach (const QString &opName, generator->availableOperations)
        {
            QAction* action = operationsMenu->addAction(opName);
            action->setWhatsThis(opName);
            action->setData(QVariant(QCursor::pos()));
            operationsMenu->addAction(action);
        }

        connect(operationsMenu, &QMenu::triggered, this, &GraphWidget::addOperationNodeUnderCursor);

        // Add seed

        QAction* addSeedAction = menu.addAction("Add seed", this, &GraphWidget::addSeedNodeUnderCursor);
        addSeedAction->setData(QVariant(QCursor::pos()));

        // Paste node buffer

        if (!copiedNodes[1].empty())
        {
            menu.addSeparator();

            QAction* pasteAction = menu.addAction("Paste", this, &GraphWidget::pasteCopiedNodes);
            pasteAction->setData(QVariant(QCursor::pos()));
        }

        menu.exec(event->globalPos());
    }
    else
        QGraphicsView::contextMenuEvent(event);
}*/



void GraphWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPos()))))
    {
        //QMenu menu(this);

        // Add seed

        //QAction* addSeedAction = menu.addAction("Add seed", this, &GraphWidget::requestNewSeed);
        //addSeedAction->setData(QVariant(QCursor::pos()));

        // Build new operation

        //QAction* buildNewOpAction = menu.addAction("Build new operation", this, &GraphWidget::requestNewOperation);
        //buildNewOpAction->setData(QVariant(QCursor::pos()));

        // Available operations

        /*QMenu availOpsMenu;

        foreach (QString opName, mFactory->availableOperationNames())
        {
            QAction
        }

        menu.addMenu(&availOpsMenu);*/

        //menu.exec(event->globalPos());
        mClickPoint = mapToScene(mapFromGlobal(event->globalPos()));
        populateAvailOpsMenu();
        enablePasteAction();
        mMainMenu->exec(event->globalPos());
    }
    else
        QGraphicsView::contextMenuEvent(event);
}



bool GraphWidget::pointIntersectsItem(QPointF point)
{
    QList<Node*> nodes;
    //QVector<Edge*> edges;

    const QList<QGraphicsItem*> items = scene()->items(point);

    /*for (QGraphicsItem* item : items)
    {
        if (OperationNode* node = qgraphicsitem_cast<OperationNode*>(item))
            nodes << node;
        else if (SeedNode* node = qgraphicsitem_cast<SeedNode*>(item))
            nodes << node;
        else if (Edge* edge = qgraphicsitem_cast<Edge*>(item))
            edges << edge;
    }*/

    for (QGraphicsItem* item : items)
    {
        if (Node* node = qgraphicsitem_cast<Node*>(item))
            nodes.append(node);
    }

    //return !nodes.empty() || !edges.empty();
    return !nodes.empty();
}



void GraphWidget::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(2.0, -event->angleDelta().y() / 2000.0);
    scaleFactor = transform().scale(factor, factor).mapRect(QRectF(0, 0, 1, 1)).width();

    scale(factor, factor);

    center = mapToScene(rect().center());
    centerOn(center);
}



void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0, Qt::black);
    gradient.setColorAt(1, QColor(128, 128, 128));
    painter->fillRect(rect, gradient);

    QRectF bound = scene()->itemsBoundingRect();
    QRectF intersection = rect.intersected(bound);

    if (intersection != bound)
    {
        qreal arrowWidth = 40.0;
        qreal arrowHeight = 10.0;
        qreal margin = 5.0;

        // Avoid scale transformation

        QTransform t = painter->transform();
        qreal m11 = t.m11();
        qreal m22 = t.m22();
        painter->save();
        painter->setTransform(QTransform(1, t.m12(), t.m13(), t.m21(), 1, t.m23(), t.m31(), t.m32(), t.m33()));

        painter->setPen(QPen(Qt::lightGray, 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        if (intersection.top() == rect.top() || bound.bottom() < rect.top())
        {
            QPainterPath path(QPointF(m11 * rect.center().x() - 0.5 * arrowWidth, m22 * rect.top() + arrowHeight + margin));
            path.lineTo(QPointF(m11 * rect.center().x(), m22 * rect.top() + margin));
            path.lineTo(QPointF(m11 * rect.center().x() + 0.5 * arrowWidth, m22 * rect.top() + arrowHeight + margin));
            painter->drawPath(path);
        }
        if (intersection.bottom() == rect.bottom() || bound.top() > rect.bottom())
        {
            QPainterPath path(QPointF(m11 * rect.center().x() - 0.5 * arrowWidth, m22 * rect.bottom() - arrowHeight - margin));
            path.lineTo(QPointF(m11 * rect.center().x(), m22 * rect.bottom() - margin));
            path.lineTo(QPointF(m11 * rect.center().x() + 0.5 * arrowWidth, m22 * rect.bottom() - arrowHeight - margin));
            painter->drawPath(path);
        }
        if (intersection.left() == rect.left() || bound.right() < rect.left())
        {
            QPainterPath path(QPointF(m11 * rect.left() + arrowHeight + margin, m22 * rect.center().y() - 0.5 * arrowWidth));
            path.lineTo(QPointF(m11 * rect.left() + margin, m22 * rect.center().y()));
            path.lineTo(QPointF(m11 * rect.left() + arrowHeight + margin, m22 * rect.center().y() + 0.5 * arrowWidth));
            painter->drawPath(path);
        }
        if (intersection.right() == rect.right() || bound.left() > rect.right())
        {
            QPainterPath path(QPointF(m11 * rect.right() - arrowHeight - margin, m22 * rect.center().y() - 0.5 * arrowWidth));
            path.lineTo(QPointF(m11 * rect.right() - margin, m22 * rect.center().y()));
            path.lineTo(QPointF(m11 * rect.right() - arrowHeight - margin, m22 * rect.center().y() + 0.5 * arrowWidth));
            painter->drawPath(path);
        }

        painter->restore();
    }
}



void GraphWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier && event->buttons() == Qt::LeftButton)
    {
        if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPosition().toPoint()))))
        {
            QPointF delta = event->position() - prevPos;
            center -= delta / scaleFactor;
            centerOn(center);

            // Draw background

            resetCachedContent();

            prevPos = event->position();
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}



void GraphWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier && event->button() == Qt::LeftButton)
    {
        if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPosition().toPoint()))))
            prevPos = event->position();
    }

    /*if (!pointIntersectsItem(mapToScene(mapFromGlobal(event->globalPosition().toPoint()))))
    {
        deselectNodeToConnect();
        setFocus();
    }*/

    QGraphicsView::mousePressEvent(event);
}



void GraphWidget::searchElementaryCycles()
{
    QList<Node*> nodes;
    QList<Edge*> edges;

    QList<QGraphicsItem*> items = scene()->items();

    foreach (QGraphicsItem* item, items)
    {
        if (Node* node = qgraphicsitem_cast<Node*>(item))
        {
            nodes.append(node);
        }
        else if (Edge* edge = qgraphicsitem_cast<Edge*>(item))
        {
            edges.append(edge);
            edge->clearCycles();
        }
        else if (Cycle* cycle = qgraphicsitem_cast<Cycle*>(item))
        {
            scene()->removeItem(cycle);
            delete cycle;
        }
    }

    QList<bool> row(nodes.size(), false);
    QList<QList<bool>> adjacencyMatrix(nodes.size(), row);

    foreach (Edge* edge, edges)
    {
        int i = nodes.indexOf(edge->sourceNode());
        int j = nodes.indexOf(edge->destNode());
        adjacencyMatrix[i][j] = true;
    }

    ElementaryCyclesSearch ecs(adjacencyMatrix, nodes);
    QList<QList<Node*>> cycles = ecs.getElementaryCycles();

    foreach (QList<Node*> nodeCycle, cycles)
    {
        Cycle* cycle = new Cycle(nodeCycle);
        scene()->addItem(cycle);
    }
}
