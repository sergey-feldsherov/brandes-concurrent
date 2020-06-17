#include <cassert>
#include <unordered_set>
#include <unordered_map>
#include <list>

#include "utils.h"
#include "ProgressBar.h"


typedef unsigned int vertex;
typedef std::pair<vertex, vertex> edge;


class Graph {
 private:
    std::unordered_map< vertex, std::unordered_set< vertex > > edges;
    std::unordered_map< vertex, double > ranking;

    globalArgs_t * const args = NULL;

    void addEdge(vertex u0, vertex u1) {
        assert(u0 != u1);
        edges[u0].insert(u1);
    }


 public:
    Graph(globalArgs_t *_args):
        args(_args) {
    }

    void loadGraph();
    void computeBrandes();
    void saveBrandesData();
};


