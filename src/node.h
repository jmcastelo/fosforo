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



#ifndef NODE_H
#define NODE_H



#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QUuid>
//#include <QMenu>
//#include <QAction>




//class GraphWidget;
class Edge;
//class NodeManager;



// Abstract node

class Node : public QGraphicsWidget
{
    Q_OBJECT

public:
    //QString name;

    //bool marked = false;

    //Node(GraphWidget* graphWidget, QString name);
    explicit Node(QUuid id, QWidget* widget, QGraphicsItem* parent = nullptr);
    ~Node();

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QUuid id() const { return mId; }

    void addEdge(Edge *edge);
    void removeEdge(Edge *edge);
    QList<Edge*> edges() const;

    bool connectedTo(Node *node);

    void centerBetween(QPointF src, QPointF dst);

    //QRectF textBoundingRect() const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

//public slots:
    //void setAsOutput();
    //void selectToConnect();
    //void copy();
    //void remove();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event) override;
    //QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    // void closeEvent(QCloseEvent* event) override;

    //GraphWidget* graph;
    QList<Edge*> edgeList;
    //bool menuOpen = false;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    //void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    //void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    //qreal ellipseMargin = 10.0;
    //qreal penSize = 2.0;
    QUuid mId;
    QWidget* mWidget;
    QGraphicsProxyWidget* mProxyWidget;
};



// Operation node

/*class OperationNode : public Node
{
    Q_OBJECT

public:
    OperationNode(GraphWidget* graphWidget, QString name);
    OperationNode(GraphWidget* graphWidget, QString name, QUuid uuid, QPointF pos);
    OperationNode(const OperationNode& node);
    ~OperationNode();

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    bool hasInputs();

    void renameOperation(QString newName);

public slots:
    void setOperation(QAction *action);
    void enableOperation(bool checked);
    void equalizeBlendFactors();
    void clear();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};*/



// Seed node

/*class SeedNode : public Node
{
    Q_OBJECT

public:
    SeedNode(GraphWidget* graphWidget, QString name = "Seed");
    SeedNode(GraphWidget* graphWidget, QUuid uuid, QPointF pos, QString name = "Seed");
    SeedNode(const SeedNode& node);
    ~SeedNode();

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

public slots:
    void draw();
    void setType(QAction* action);
    void loadImage();
    void setFixed(bool fixed);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
};*/



#endif // NODE_H
