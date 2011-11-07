/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       PlantGL: Modeling Plant Geometry
 *
 *       Copyright 2000-2009 - Cirad/Inria/Inra - Virtual Plant Team
 *
 *       File author(s): F. Boudon (frederic.boudon@cirad.fr) et al.
 *
 *       Development site : https://gforge.inria.fr/projects/openalea/
 *
 *  ----------------------------------------------------------------------------
 *
 *                      GNU General Public Licence
 *
 *       This program is free software; you can redistribute it and/or
 *       modify it under the terms of the GNU General Public License as
 *       published by the Free Software Foundation; either version 2 of
 *       the License, or (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public
 *       License along with this program; see the file COPYING. If not,
 *       write to the Free Software Foundation, Inc., 59
 *       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  ----------------------------------------------------------------------------
 */

#ifndef __dijkstra_h__
#define __dijkstra_h__

#include "../algo_config.h"
#include <plantgl/scenegraph/container/indexarray.h>
#include <plantgl/tool/util_array.h>

#include <queue>
#include <memory>

PGL_BEGIN_NAMESPACE

/**
  my priority queue for comparing node
*/

#define DIJKSTRA_WITH_SORTED_VECTOR

template<class DistArray>
struct nodecompare {
       const DistArray& distances;
       nodecompare(const DistArray& _distances) : distances(_distances) {}
       bool operator()(const uint32_t& a,const uint32_t& b) { return distances[a] > distances[b]; }
};
	
template<class DistArray>
class node_priority_queue {
public:


#ifdef DIJKSTRA_WITH_SORTED_VECTOR
    std::vector<uint32_t> c;
#else
    std::deque<uint32_t> c;
#endif
    struct nodecompare<DistArray> comp;

    node_priority_queue(const DistArray& distances): comp(distances) { }

	bool empty() const { return (c.empty()); }
	size_t size() const { return (c.size()); }

	void push(uint32_t _Val)
		{	
#ifdef DIJKSTRA_WITH_SORTED_VECTOR
            c.insert(lower_bound(c.begin(),c.end(),_Val,comp),_Val);
#else
            c.push_back(_Val);
		    push_heap(c.begin(), c.end(), comp);
#endif
		}

	uint32_t pop()
	{
#ifdef DIJKSTRA_WITH_SORTED_VECTOR
        uint32_t topvalue = c.back(); 
        c.pop_back();
#else
        uint32_t topvalue = c.front(); 
		pop_heap(c.begin(), c.end(), comp);
		c.pop_back();
#endif
        return topvalue;
	}

    void update() { 
#ifdef DIJKSTRA_WITH_SORTED_VECTOR
        sort(c.begin(),c.end(), comp); 
#else
        sort_heap(c.begin(),c.end(), comp); 
#endif
    }
};

struct Node {
    uint32_t id;
    uint32_t parent;
    real_t distance;
    Node(uint32_t _id, uint32_t _parent, real_t _distance) 
        : id(_id), parent(_parent), distance(_distance) {}
} ;

typedef std::vector<Node> NodeList;

template<class EdgeWeigthEvaluation>
NodeList  dijkstra_shortest_paths_in_a_range(const IndexArrayPtr& connections, 
                                   uint32_t root, 
                                   EdgeWeigthEvaluation& distevaluator,
                                   real_t maxdist = REAL_MAX)
 {

     NodeList result;

     size_t nbnodes = connections->size();
     real_t * distances = new real_t [nbnodes];
     for(real_t * it = distances; it != distances+nbnodes; ++it) *it = REAL_MAX;
     distances[root] = 0;
     uint32_t * parents = new uint32_t[nbnodes];
     parents[root] = root;

     std::vector<bool> colored(nbnodes,false);

     node_priority_queue<real_t *> Q(distances);
     Q.push(root);

     while(!Q.empty()){
         uint32_t current = Q.pop();
         result.push_back(Node(current,parents[current],distances[current])); 
         const Index& nextchildren = connections->getAt(current);
         for (Index::const_iterator itchildren = nextchildren.begin();
             itchildren != nextchildren.end(); ++itchildren)
         {
             uint32_t v = *itchildren;
             real_t weigthuv = distevaluator(current,v); 
             real_t distance = weigthuv+distances[current];
             if (distance < distances[v] && distance < maxdist){
                 distances[v] = distance;
                 parents[v] = current;
                 if (!colored[v]){
                     colored[v] = true;
                     Q.push(v);
                 }
                 else {
                     Q.update();
                 }
             }
         }
     }
     delete [] parents;
	 delete [] distances;
     return result;
 }

template<class EdgeWeigthEvaluation>
std::pair<TOOLS(Uint32Array1Ptr),TOOLS(RealArrayPtr)>  dijkstra_shortest_paths(const IndexArrayPtr& connections, 
                                   uint32_t root, 
                                   EdgeWeigthEvaluation& distevaluator)
 {


     size_t nbnodes = connections->size();
     TOOLS(RealArrayPtr) distances(new TOOLS(RealArray)(nbnodes,REAL_MAX));
     distances->setAt(root,0);
     TOOLS(Uint32Array1Ptr) parents(new TOOLS(Uint32Array1)(nbnodes,root));

     std::vector<bool> colored(nbnodes,false);

     node_priority_queue<RealArray> Q(*distances);
     Q.push(root);

     while(!Q.empty()){
         uint32_t current = Q.pop();
         const Index& nextchildren = connections->getAt(current);
         for (Index::const_iterator itchildren = nextchildren.begin();
             itchildren != nextchildren.end(); ++itchildren)
         {
             uint32_t v = *itchildren;
             real_t weigthuv = distevaluator(current,v); 
             real_t distance = weigthuv+distances->getAt(current);
             if (distance < distances->getAt(v)){
                 distances->setAt(v,distance);
                 parents->setAt(v,current);
                 if (!colored[v]){
                     colored[v] = true;
                     Q.push(v);
                 }
                 else {
                     Q.update();
                 }
             }
         }
     }
     return std::pair<TOOLS(Uint32Array1Ptr),TOOLS(RealArrayPtr)>(parents,distances);
 }
 /*
 DIJKSTRA(G, s, w)
  for each vertex u in V
    d[u] := infinity 
    p[u] := u 
    color[u] := WHITE
  end for
  color[s] := GRAY 
  d[s] := 0 
  INSERT(Q, s)
  while (Q != �)
    u := EXTRACT-MIN(Q)
    S := S U { u }
    for each vertex v in Adj[u]
      if (w(u,v) + d[u] < d[v])
        d[v] := w(u,v) + d[u]
        p[v] := u 
        if (color[v] = WHITE) 
          color[v] := GRAY
          INSERT(Q, v) 
        else if (color[v] = GRAY)
          DECREASE-KEY(Q, v)
      else
        ...
    end for
    color[u] := BLACK
  end while
  return (d, p)
  */

PGL_END_NAMESPACE

#endif