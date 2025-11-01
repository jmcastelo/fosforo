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



#ifndef EDGE_H
#define EDGE_H



#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QUuid>
//#include <QAction>
//#include <QGroupBox>



//class GraphWidget;
class Node;
class Cycle;
class EdgeWidget;



class Edge : public QGraphicsWidget
{
    Q_OBJECT

public:
    //Edge(GraphWidget* graphWidget, Node *sourceNode, Node *destNode);
    explicit Edge(Node* sourceNode, Node* destNode, bool predge, EdgeWidget* widget, QGraphicsItem* parent = nullptr);
    // ~Edge();

    Node* sourceNode() const;
    Node* destNode() const;

    void adjust();

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

    //int linkOffset = 0;

    QPointF srcPoint() const { return sourcePoint; }
    QPointF dstPoint() const { return destPoint; }

    bool isPredge() const { return mPredge; }

    void setWidgetVisible(bool visible);

    QList<Cycle*> cycles() const { return cycleList; }
    void addCycle(Cycle* cycle) { cycleList.push_back(cycle); }
    void clearCycles() { cycleList.clear(); }

    /*void setBlendFactor(float factor);
    void drawBlendFactor(bool draw) { paintBlendFactor = draw; }
    void updateBlendFactor();
    void closeBlendFactorWidget();
    void setBlendFactorGroupBoxTitle();*/

    //void constructBlendFactorWidget();

    QRectF boundingRect() const override;

//signals:
    //void blendFactorChanged();
    //void nodesConnected();
    //void blendFactorWidgetCreated(BlendFactorWidget* widget);
    //void blendFactorWidgetToggled(BlendFactorWidget* widget);
    //void blendFactorWidgetDeleted(BlendFactorWidget* widget);

public slots:
    //void remove();
    void setPredge(bool predge);
    void togglePredge(bool predge);

protected:
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    // void closeEvent(QCloseEvent* event) override;

private:
    //GraphWidget* graph;

    QUuid mId;

    EdgeWidget* mWidget;
    QGraphicsProxyWidget* mProxyWidget;

    Node* source;
    Node* dest;

    QPointF sourcePoint;
    QPointF destPoint;

    QList<Cycle*> cycleList;

    bool mPredge = false;

    qreal arrowSize = 20;

    bool mWidgetVisible = false;

    //bool paintBlendFactor = false;

    //BlendFactorWidget* blendFactorWidget = nullptr;

    QPointF intersectionPoint(Node *node, QLineF line);
    //void setLinkOffset();

    //void setAsPredge();
    //void setAsEdge();

    //void showBlendFactorWidget();

//private slots:
    //void insertNode(QAction* action);
};



#endif // EDGE_H
