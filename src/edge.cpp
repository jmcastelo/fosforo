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



#include "edge.h"
#include "node.h"
//#include "cycle.h"
//#include "graphwidget.h"
//#include "generator.h"
#include "edgewidget.h"

#include <QPainter>
// #include <QGraphicsScene>
//#include <QtMath>
//#include <QGraphicsSceneContextMenuEvent>
//#include <QMenu>
//#include <QDoubleValidator>
//#include <QSlider>
//#include <QFormLayout>
//#include <QGroupBox>
//#include <QPushButton>



/*Edge::Edge(GraphWidget *graphWidget, Node *sourceNode, Node *destNode)
    : graph { graphWidget },
    source { sourceNode },
    dest { destNode }
{
    setAcceptedMouseButtons(Qt::NoButton);
    source->addEdge(this);
    dest->addEdge(this);

    adjust();

    foreach (Edge* edge, dest->edges())
        if (edge->destNode() == source)
            edge->adjust();

    connect(this, &Edge::blendFactorWidgetCreated, graph, &GraphWidget::blendFactorWidgetCreated);
    connect(this, &Edge::blendFactorWidgetToggled, graph, &GraphWidget::blendFactorWidgetToggled);
    connect(this, &Edge::blendFactorWidgetDeleted, graph, &GraphWidget::blendFactorWidgetDeleted);

    if (graph->generator->isNode(source->id) && graph->generator->isNode(dest->id))
        constructBlendFactorWidget();
}*/

Edge::Edge(Node* sourceNode, Node* destNode, bool predge, EdgeWidget *widget, QGraphicsItem* parent) :
    QGraphicsWidget { parent },
    mWidget { widget },
    source { sourceNode },
    dest { destNode }
{
    setAcceptedMouseButtons(Qt::NoButton);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-2);

    // Set proxy

    mProxyWidget = new QGraphicsProxyWidget(this);
    mProxyWidget->setWidget(mWidget);

    // Add this edge to source and dest nodes

    source->addEdge(this);
    dest->addEdge(this);

    setWidgetVisible(false);

    // Predge?

    setPredge(predge);

    adjust();

    foreach (Edge* edge, dest->edges()) {
        if (edge->destNode() == source) {
            edge->adjust();
        }
    }

    // Connections

    connect(mWidget, &EdgeWidget::typeActionToggled, this, &Edge::setPredge);
}



/*Edge::~Edge()
{
    qDebug() << this;
}*/


/*void Edge::remove()
{
    source->removeEdge(this);
    dest->removeEdge(this);

    foreach (Edge* edge, dest->edges())
        if (edge->destNode() == source)
            edge->adjust();

    emit blendFactorWidgetDeleted(blendFactorWidget);
    blendFactorWidget->deleteLater();

    graph->removeEdge(this);
}*/



Node* Edge::sourceNode() const
{
    return source;
}



Node* Edge::destNode() const
{
    return dest;
}



void Edge::setWidgetVisible(bool visible)
{
    mWidgetVisible = visible;
    mWidget->setVisible(visible);
}



void Edge::setPredge(bool predge)
{
    mPredge = predge;
    mWidget->toggleTypeAction(predge);
    update();
}


void Edge::togglePredge(bool predge)
{
    mPredge = predge;
    update();
}


/*void Edge::setPredge(bool set)
{
    predge = set;

    if (predge)
        graph->generator->setOperationInputType(source->id, dest->id, InputType::Blit);
    else
        graph->generator->setOperationInputType(source->id, dest->id, InputType::Normal);

    update();
}*/



/*QPointF Edge::intersectionPoint(Node *node, Node *other, QPointF offset, QLineF line)
{
    QPointF center = node->mapToScene(node->boundingRect().center());

    QPointF nodePoint = node->mapToScene(node->boundingRect().center() + offset);
    QPointF otherPoint = other->mapToScene(other->boundingRect().center() + offset);

    QRectF ellipseRect = node->mapRectToScene(node->boundingRect());
    qreal halfWidth = 0.5 * ellipseRect.width();
    qreal halfHeight = 0.5 * ellipseRect.height();

    if (!qFuzzyIsNull(line.x2() - line.x1()))
    {
        qreal slope = (line.y2() - line.y1()) / (line.x2() - line.x1());

        QLineF vertical(0.0, -10000.0, 0.0, 10000.0);
        QPointF point0;
        line.intersects(vertical, &point0);

        qreal a = halfHeight * halfHeight + halfWidth * halfWidth * slope * slope;
        qreal b = 2.0 * (slope * halfWidth * halfWidth * (point0.y() - center.y()) - halfHeight * halfHeight * center.x());
        qreal c = halfHeight * halfHeight * (center.x() * center.x() - halfWidth * halfWidth) + halfWidth * halfWidth * (point0.y() - center.y()) * (point0.y() - center.y());

        qreal q = b * b - 4.0 * a * c;

        qreal root1 = -(b + qSqrt(q)) / (2.0 * a);
        qreal root2 = (qSqrt(q) - b) / (2.0 * a);

        QPointF intersection1(root1, slope * root1 + point0.y());
        QPointF intersection2(root2, slope * root2 + point0.y());

        if (QLineF(otherPoint, intersection1).length() < QLineF(otherPoint, intersection2).length())
            return intersection1 - nodePoint;
        else
            return intersection2 - nodePoint;
    }
    else
    {
        qreal scaled = (nodePoint.x() - center.x()) / halfWidth;
        qreal yOff = halfHeight * qSqrt(1.0 - scaled * scaled);

        QPointF intersection1(nodePoint.x(), center.y() + yOff);
        QPointF intersection2(nodePoint.x(), center.y() - yOff);

        if (QLineF(otherPoint, intersection1).length() < QLineF(otherPoint, intersection2).length())
            return intersection1 - nodePoint;
        else
            return intersection2 - nodePoint;
    }
}*/

QPointF Edge::intersectionPoint(Node* node, QLineF line)
{
    QRectF nodeRect = node->mapRectToScene(node->boundingRect());

    QPointF intersectionPoint;

    QLineF topLine(nodeRect.topLeft(), nodeRect.topRight());
    if (line.intersects(topLine, &intersectionPoint) == QLineF::BoundedIntersection)
        return intersectionPoint;

    QLineF rightLine(nodeRect.topRight(), nodeRect.bottomRight());
    if (line.intersects(rightLine, &intersectionPoint) == QLineF::BoundedIntersection)
        return intersectionPoint;

    QLineF bottomLine(nodeRect.bottomLeft(), nodeRect.bottomRight());
    if (line.intersects(bottomLine, &intersectionPoint) == QLineF::BoundedIntersection)
        return intersectionPoint;

    QLineF leftLine(nodeRect.bottomLeft(), nodeRect.topLeft());
    if (line.intersects(leftLine, &intersectionPoint) == QLineF::BoundedIntersection)
        return intersectionPoint;

    return node->mapToScene(node->boundingRect().center());
}


void Edge::adjust()
{
    if (!source || !dest)
        return;

    prepareGeometryChange();

    QLineF line(source->mapToScene(source->boundingRect().center()), dest->mapToScene(dest->boundingRect().center()));

    QPointF srcIntPoint = intersectionPoint(source, line);
    QPointF dstIntPoint = intersectionPoint(dest, line);

    if (!source->contains(source->mapFromScene(dstIntPoint)) && !dest->contains(dest->mapFromScene(srcIntPoint)))
    {
        sourcePoint = mapFromScene(srcIntPoint);
        destPoint = mapFromScene(dstIntPoint);
    }
    else
    {
        destPoint = sourcePoint;
    }

    QLineF visibleLine = QLineF(sourcePoint, destPoint);

    // Make proxy and line centers coincide

    QPointF proxyPos = visibleLine.center() - QPointF(0.5 * mWidget->width(), 0.5 * mWidget->height());
    mProxyWidget->setPos(proxyPos);

    // Edge widget visible if edge is long enough

    if (mWidgetVisible) {
        mWidget->setVisible(!mProxyWidget->boundingRect().contains(mProxyWidget->mapFromScene(visibleLine.p1())) && !mProxyWidget->boundingRect().contains(mProxyWidget->mapFromScene(visibleLine.p2())));
        mWidget->adjustAllSizes();
    }
}



/*void Edge::setLinkOffset()
{
    linkOffset = 0;

    foreach (Edge* edge, dest->edges())
    {
        if (edge->destNode() == source)
        {
            if (edge->linkOffset == 0 || edge->linkOffset == 1)
                linkOffset = -1;
            else
                linkOffset = 1;
        }
    }
}*/



/*void Edge::setAsPredge()
{
    predge = true;

    graph->generator->setOperationInputType(source->id, dest->id, InputType::Blit);

    update();
}



void Edge::setAsEdge()
{
    predge = false;

    graph->generator->setOperationInputType(source->id, dest->id, InputType::Normal);

    update();
}*/



/*void Edge::insertNode(QAction* action)
{
    graph->insertNodeBetween(action, this);
}



void Edge::constructBlendFactorWidget()
{
    blendFactorWidget = new BlendFactorWidget(this, graph->generator->blendFactor(source->id, dest->id));
    connect(blendFactorWidget, &BlendFactorWidget::toggled, graph, &GraphWidget::blendFactorWidgetToggled);
    emit blendFactorWidgetCreated(blendFactorWidget);
}



void Edge::setBlendFactorGroupBoxTitle()
{
    blendFactorWidget->setTitle(source->name + " - " + dest->name);
}



void Edge::updateBlendFactor()
{
    blendFactorWidget->setBlendFactor(graph->generator->blendFactor(source->id, dest->id));
}



void Edge::setBlendFactor(float factor)
{
    graph->generator->setBlendFactor(source->id, dest->id, factor);
    paintBlendFactor = true;
    update();
}



void Edge::showBlendFactorWidget()
{
    blendFactorWidget->toggle(true);
}



void Edge::closeBlendFactorWidget()
{
    if (blendFactorWidget)
        blendFactorWidget->close();
}*/



QRectF Edge::boundingRect() const
{
    if (!source || !dest)
        return QRectF();

    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(), destPoint.y() - sourcePoint.y())).normalized().adjusted(-10.0, -10.0, 10.0, 10.0);
}



QPainterPath Edge::shape() const
{
    QLineF line(sourcePoint, destPoint);
    QLineF normal1 = line.normalVector();
    normal1.setLength(5.0);
    QLineF normal2(sourcePoint, 2.0 * sourcePoint - normal1.p2());

    QPainterPath path;
    path.addPolygon(QPolygonF(QList<QPointF>{ normal2.p2(), normal1.p2(), normal1.p2() + destPoint - sourcePoint, normal2.p2() + destPoint - sourcePoint }));
    path.closeSubpath();
    return path;
}



void Edge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (!source || !dest)
        return;

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.0)))
        return;

    // Draw the line itself

    if (mPredge)
        painter->setPen(QPen(Qt::lightGray, 2.0, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
    else
        painter->setPen(QPen(Qt::lightGray, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    painter->drawLine(line);

    // Draw the arrow

    if (line.length() > arrowSize)
    {
        double angle = std::atan2(-line.dy(), line.dx());

        QPointF destArrowP1 = destPoint + QPointF(sin(angle - M_PI / 2.5) * arrowSize, cos(angle - M_PI / 2.5) * arrowSize);
        QPointF destArrowP2 = destPoint + QPointF(sin(angle - M_PI + M_PI / 2.5) * arrowSize, cos(angle - M_PI + M_PI / 2.5) * arrowSize);

        painter->setPen(QPen(Qt::lightGray, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::lightGray);
        painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
    }

    // Paint the widget

    mProxyWidget->paint(painter, option, widget);

    /*if (paintBlendFactor)
    {
        QString text = QString::number(graph->generator->blendFactor(source->id, dest->id));

        QFontMetricsF fontMetrics(scene()->font());
        qreal textWidth = fontMetrics.boundingRect(text).width();

        painter->setFont(scene()->font());
        painter->drawText(line.center() - QPointF(0.5 * textWidth, 0.0), text);
    }*/
}



/*void Edge::closeEvent(QCloseEvent* event)
{
    mWidget->close();
    QGraphicsWidget::closeEvent(event);
}*/


/*void Edge::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    if (qgraphicsitem_cast<OperationNode*>(source))
    {
        if (!predge)
            menu.addAction("Set as predge", this, &Edge::setAsPredge);
        else
        {
            bool showAcion = true;

            foreach (Cycle* cycle, cycleList)
            {
                if (cycle->numPredges() == 1)
                {
                    showAcion = false;
                    break;
                }
            }

            if (showAcion)
                menu.addAction("Set as edge", this, &Edge::setAsEdge);
        }
    }

    menu.addAction("Set blend factor", this, &Edge::showBlendFactorWidget);

    QMenu *operationsMenu = menu.addMenu("Insert operation");

    foreach (QString opName, graph->generator->availableOperations)
    {
        QAction* action = operationsMenu->addAction(opName);
        action->setData(QVariant(QCursor::pos()));
    }

    connect(operationsMenu, &QMenu::triggered, this, &Edge::insertNode);

    menu.addAction("Remove", this, &Edge::remove);

    menu.exec(event->screenPos());

    //QGraphicsObject::contextMenuEvent(event);
}*/
