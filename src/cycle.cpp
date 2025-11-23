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



#include "cycle.h"
#include "edge.h"
#include "node.h"

#include <QPainter>
#include <QStyleOption>



Cycle::Cycle(QList<Node*> nodes, QGraphicsItem *parent) :
    QGraphicsItem { parent }
{
    // Construct list of edges
    // Nodes of the cycle are ordered

    bool containsPredge = false;

    for (int i = 0; i < nodes.size(); i++)
    {
        foreach (Edge* edge, nodes[i]->edges())
        {
            if (edge->destNode() == nodes[(i + 1) % nodes.size()])
            {
                edges.push_back(edge);
                edge->addCycle(this);
                containsPredge |= edge->isPredge();
                break;
            }
        }
    }

    // Cycle must contain at least one predge

    if (!containsPredge) {
        edges.back()->setPredge(true);
    }

    setAcceptedMouseButtons(Qt::NoButton);
    // setCacheMode(DeviceCoordinateCache);
    setZValue(-3);
}



int Cycle::numPredges()
{
    int n = 0;

    foreach (Edge* edge, edges) {
        if (edge->isPredge()) {
            n++;
        }
    }

    return n;
}



QRectF Cycle::boundingRect() const
{
    QRectF whole;

    foreach (Edge* edge, edges) {
        whole = whole.united(edge->boundingRect());
    }

    return whole;
}



QPainterPath Cycle::shape() const
{
    QPolygonF boundary;

    for (int i = 0; i < edges.size(); i++)
    {
        boundary << edges[i]->srcPoint() << edges[i]->dstPoint();
        boundary << edges[(i + 1) % edges.size()]->srcPoint();
    }

    QPainterPath path;
    path.addPolygon(boundary);
    path.closeSubpath();
    return path;
}



void Cycle::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(16, 64, 255, 32));

    QPolygonF boundary;

    for (int i = 0; i < edges.size(); i++)
    {
        boundary << edges[i]->srcPoint() << edges[i]->dstPoint();
        boundary << edges[(i + 1) % edges.size()]->srcPoint();
    }

    painter->drawPolygon(boundary);
}
