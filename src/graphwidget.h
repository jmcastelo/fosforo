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

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H



//#include "blendfactorwidget.h"
#include "factory.h"
#include "nodemanager.h"

#include <QGraphicsView>
#include <QUuid>
#include <QMap>
#include <QPointF>



class Node;
class Edge;
class EdgeWidget;
// class NodeManager;
//struct InputData;


class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    GraphWidget(Factory* factory, NodeManager* nodeManager, QWidget *parent = nullptr);
    ~GraphWidget();

    //Node* getNode(QUuid id);
    //void selectNode(QUuid id, bool selected);
    //void copyNode(Node* node);
    //void removeNode(Node* node);
    //void removeEdge(Edge* edge);
    //void selectNodeToConnect(Node* node);
    //void deselectNodeToConnect();
    //void connectNodes(Node* node);
    //void insertNodeBetween(QAction* action, Edge* edge);

    //bool nodeSelectedToConnect();
    //bool moreThanOneNode();

    //void newNodeSelected(Node*);
    //bool nodesSelected();
    //bool singleOperationNodeSelected();
    //bool isOperationNodeSelected(QUuid id);
    //int operationNodesSelected();
    //bool twoOperationNodesSelected();
    //int seedNodesSelected();
    //bool selectedOperationNodesHaveInputs();

    //void updateOperation(QUuid id);

    //void onDestroyOperationNode(QUuid id);
    //void onDestroySeedNode(QUuid id);

    //void updateNodes();

    //void drawBlendFactors(bool draw);

    QPointF nodePosition(QUuid id);
    void setNodePosition(QUuid id, QPointF position);

    //void clearScene();
    //void loadSeedNode(QUuid id, QPointF position);
    //void loadOperationNode(QUuid id, QString name, QPointF position);
    //void connectNodes(QMap<QUuid, QMap<QUuid, InputData*>> connections);

    //void closeWidgets();

public slots:
    void clearScene();
    void markNodes(QList<QUuid> ids);
    //void drawSelectedSeeds();
    //void enableSelectedOperations();
    //void disableSelectedOperations();
    //void equalizeSelectedBlendFactors();
    //void clearSelectedOperationNodes();
    //void swapSelectedOperationNodes();
    //void makeNodeSnapshot();
    //void removeSelectedNodes();


signals:
    void selectedNodesChanged(QList<QUuid> selNodeIds);
    /*void singleNodeSelected(Node*);
    void operationNodeSelected(QUuid id);
    void multipleNodesSelected();
    void noOperationNodesSelected();
    void showOperationParameters(QUuid id);
    void removeOperationParameters(QUuid id);
    void updateOperationParameters(QUuid id);
    void blendFactorWidgetCreated(BlendFactorWidget* widget);
    void blendFactorWidgetToggled(BlendFactorWidget* widget);
    void blendFactorWidgetDeleted(BlendFactorWidget* widget);
    void operationEnabled(QUuid id, bool enabled);*/

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    Factory* mFactory;
    NodeManager* mNodeManager;
    //Node *selectedNode = nullptr;

    //QVector<OperationNode*> selectedOperationNodes;
    //QVector<SeedNode*> selectedSeedNodes;

    //QMap<QUuid, Node*> copiedNodes[2];
    //QVector<Edge*> newEdges;

    QPointF prevPos;
    QPointF center;
    qreal scaleFactor = 1.0;

    QPointF mClickPoint;

    QMenu* mMainMenu;
    QMenu* mAvailOpsMenu;

    QAction* mAddSeedAction;
    QAction* mBuildNewOpAction;
    QList<QAction*> mAvailOpsActions;
    QAction* mPasteNodesAction;

    QMap<QUuid, ImageOperation*> mCopiedOperations;
    QMap<QUuid, Seed*> mCopiedSeeds;
    QMap<QUuid, QPointF> mCopiedNodePositions;

    void populateAvailOpsMenu();
    void enablePasteAction();

    //void copyNodes(bool connectionA);
    bool pointIntersectsItem(QPointF point);
    void reconnectNodes(Node* node);

    void searchElementaryCycles();

    Node* getNode(QUuid id);
    Edge* getEdge(QUuid srcId, QUuid dstId);

    void pasteNodes();

private slots:
    void onActionTriggered(QAction* action);
    void newSelectedNodes();
    //void addOperationNodeUnderCursor(QAction* action);
    // void addSeedNode();
    //void pasteCopiedNodes();
    void addNewNode(QUuid id, QWidget* widget);
    void addNewNodeOnPos(QUuid id, QWidget* widget, QPointF pos);
    // void addNewOperationNode(OperationWidget* widget);
    void connectNodes(QUuid srcId, QUuid dstId, InputType type, EdgeWidget* widget);
    void removeNode(QUuid id);
    void removeEdge(QUuid srcId, QUuid dstId);
    void centerNodeBetween(QUuid srcId, QUuid dstId, QUuid opId);
    void setCopiedNodes(QMap<QUuid, ImageOperation*> opsMap, QMap<QUuid, Seed*> seedsMap);
};



#endif // GRAPHWIDGET_H
