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



#ifndef CYCLESEARCH_H
#define CYCLESEARCH_H



#include "node.h"
#include <QList>



struct SCCResult
{
    QList<QList<int>> adjacencyList;
    int lowestNodeId;

    SCCResult(QList<QList<int>> adjList, int id) :
        adjacencyList { adjList },
        lowestNodeId { id }
    {}
};



class ElementaryCyclesSearch
{
public:
    ElementaryCyclesSearch(QList<QList<bool>> matrix, QList<Node*> nodes);
    QList<QList<Node*>> getElementaryCycles();

private:
    QList<QList<Node*>> cycles;
    QList<QList<int>> adjacencyList;
    QList<Node*> graphNodes;
    QList<bool> blocked;
    QList<QList<int>> B;
    QList<int> stack;

    QList<QList<int>> getAdjacencyList(QList<QList<bool>> matrix);
    bool findCycles(int v, int s, QList<QList<int>> adjList);
    void unblock(int node);
};



class StrongConnectedComponents
{
public:
    StrongConnectedComponents(QList<QList<int>> adjList);
    SCCResult* getAdjacencyList(int node);

private:
    QList<QList<int>> adjacencyListOriginal;
    QList<QList<int>> adjacencyList;
    QList<bool> visited;
    QList<int> stack;
    QList<int> lowlink;
    QList<int> number;
    int sccCounter = 0;
    QList<QList<int>> currentSCCs;

    void makeAdjListSubgraph(int node);
    QList<int> getLowestIdComponent();
    QList<QList<int>> getAdjList(QList<int> nodes);
    void getStrongConnectedComponents(int root);
};



#endif // CYCLESEARCH_H
