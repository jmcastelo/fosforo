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

#include "cyclesearch.h"

// Elementary cycles search

ElementaryCyclesSearch::ElementaryCyclesSearch(QList<QList<bool>> matrix, QList<Node*> nodes)
{
    graphNodes = nodes;
    adjacencyList = getAdjacencyList(matrix);
}

QList<QList<int>> ElementaryCyclesSearch::getAdjacencyList(QList<QList<bool>> matrix)
{
     QList<QList<int>> list;

     for (int i = 0; i < matrix.size(); i++)
     {
         QList<int> v;

         for (int j = 0; j < matrix[i].size(); j++)
             if (matrix[i][j])
                 v.push_back(j);

         list.push_back(v);
     }

     return list;
}

QList<QList<Node*>> ElementaryCyclesSearch::getElementaryCycles()
{
    cycles.clear();
    blocked.clear();
    blocked.resize(adjacencyList.size());
    B.clear();
    B.resize(adjacencyList.size());
    stack.clear();

    StrongConnectedComponents sccs(adjacencyList);

    int s = 0;

    while (true)
    {
        SCCResult* sccResult = sccs.getAdjacencyList(s);

        if(sccResult && !sccResult->adjacencyList.empty())
        {
            QList<QList<int>> scc = sccResult->adjacencyList;
            s = sccResult->lowestNodeId;

            for (int i = 0; i < scc.size(); i++)
            {
                if (!scc[i].empty())
                {
                    blocked[i] = false;
                }
            }

            findCycles(s, s, scc);
            s++;
        }
        else
        {
            break;
        }
    }

    return cycles;
}

bool ElementaryCyclesSearch::findCycles(int v, int s, QList<QList<int>> adjList)
{
    bool f = false;
    stack.push_back(v);
    blocked[v] = true;

    for (int i = 0; i < adjList[v].size(); i++)
    {
        int w = adjList[v][i];

        if (w == s)
        {
            QList<Node*> cycle;

            for (int j = 0; j < stack.size(); j++)
                cycle.push_back(graphNodes[stack[j]]);

            cycles.push_back(cycle);

            f = true;
        }
        else if (!blocked[w])
        {
            if (findCycles(w, s, adjList))
                f = true;
        }
    }

    if (f)
    {
        unblock(v);
    }
    else
    {
        for (int i = 0; i < adjList[v].size(); i++)
        {
            int w = adjList[v][i];

            if (!B[w].contains(v))
                B[w].push_back(v);
        }
    }

    stack.remove(stack.indexOf(v));
    return f;
}

void ElementaryCyclesSearch::unblock(int node)
{
    blocked[node] = false;

    QList<int> Bnode = B[node];

    while (Bnode.size() > 0)
    {
        int w = Bnode[0];
        Bnode.remove(0);
        if (blocked[w])
            unblock(w);
    }
}

// Strong connected components

StrongConnectedComponents::StrongConnectedComponents(QList<QList<int>> adjList)
{
    adjacencyListOriginal = adjList;
}

SCCResult* StrongConnectedComponents::getAdjacencyList(int node)
{
    visited.clear();
    visited.resize(adjacencyListOriginal.size());
    lowlink.clear();
    lowlink.resize(adjacencyListOriginal.size());
    number.clear();
    number.resize(adjacencyListOriginal.size());
    visited.clear();
    visited.resize(adjacencyListOriginal.size());
    stack.clear();
    currentSCCs.clear();

    makeAdjListSubgraph(node);

    for (int i = node; i < adjacencyListOriginal.size(); i++)
    {
        if(!visited[i])
        {
            getStrongConnectedComponents(i);

            QList<int> nodes = getLowestIdComponent();

            if(!nodes.contains(node) && !nodes.contains(node + 1))
            {
                return getAdjacencyList(node + 1);
            }
            else
            {
                QList<QList<int>> adjList = getAdjList(nodes);
                if (!adjList.empty())
                    for (int j = 0; j < adjacencyListOriginal.size(); j++)
                        if (!adjList[j].empty())
                            return new SCCResult(adjList, j);
            }
        }
    }

    return nullptr;
}

void StrongConnectedComponents::makeAdjListSubgraph(int node)
{
    adjacencyList.clear();
    adjacencyList.resize(adjacencyListOriginal.size());

    for (int i = node; i < adjacencyList.size(); i++)
    {
        QList<int> successors;

        for (int j = 0; j < adjacencyListOriginal[i].size(); j++)
            if (adjacencyListOriginal[i][j] >= node)
                successors.push_back(adjacencyListOriginal[i][j]);

        if (!successors.empty())
        {
            adjacencyList[i].clear();
            adjacencyList[i].resize(successors.size());

            for (int j = 0; j < successors.size(); j++)
                adjacencyList[i][j] = successors[j];
        }
    }
}

QList<int> StrongConnectedComponents::getLowestIdComponent()
{
    int min = adjacencyList.size();

    QList<int> currScc;

    for (int i = 0; i < currentSCCs.size(); i++)
    {
        QList<int> scc = currentSCCs[i];

        for (int j = 0; j < scc.size(); j++)
        {
            int node = scc[j];
            if (node < min)
            {
                currScc = scc;
                min = node;
            }
        }
    }

    return currScc;
}

QList<QList<int>> StrongConnectedComponents::getAdjList(QList<int> nodes)
{
    QList<QList<int>> lowestIdAdjacencyList;

    if (!nodes.empty())
    {
        lowestIdAdjacencyList.resize(adjacencyList.size());

        for (int i = 0; i < nodes.size(); i++)
        {
            int node = nodes[i];

            for (int j = 0; j < adjacencyList[node].size(); j++)
            {
                int succ = adjacencyList[node][j];
                if (nodes.contains(succ))
                    lowestIdAdjacencyList[node].push_back(succ);
            }
        }
    }

    return lowestIdAdjacencyList;
}

void StrongConnectedComponents::getStrongConnectedComponents(int root)
{
    sccCounter++;
    lowlink[root] = sccCounter;
    number[root] = sccCounter;
    visited[root] = true;
    stack.push_back(root);

    for (int i = 0; i < adjacencyList[root].size(); i++)
    {
        int w = adjacencyList[root][i];

        if (!visited[w])
        {
            getStrongConnectedComponents(w);
            lowlink[root] = lowlink[root] < lowlink[w] ? lowlink[root] : lowlink[w];
        }
        else if (number[w] < number[root])
        {
            if (stack.contains(w))
                lowlink[root] = lowlink[root] < number[w] ? lowlink[root] : number[w];
        }
    }

    if (lowlink[root] == number[root] && !stack.empty())
    {
        int next = -1;
        QList<int> scc;

        do
        {
            next = stack.back();
            stack.pop_back();
            scc.push_back(next);
        }
        while (number[next] > number[root]);

        if (scc.size() > 1)
            currentSCCs.push_back(scc);
    }
}
