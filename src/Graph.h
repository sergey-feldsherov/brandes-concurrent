#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <thread>

#include "utils.h"


typedef unsigned int vertex;
typedef std::pair<vertex, vertex> edge;


class Graph {
 private:
    std::vector< vertex > vertices;
    std::unordered_map< vertex, std::unordered_set< vertex > > edges;
    std::unordered_map< vertex, double > ranking;
    std::vector< std::unordered_map< vertex, double > > concurrentRanking;
    std::vector< std::thread > workers;


    globalArgs_t * const args = NULL;

    void addEdge(vertex u0, vertex u1) {
        edges[u0].insert(u1);
    }

    void threadFuncBrandes(unsigned int id, unsigned int begin, unsigned int end);

 public:
    Graph(globalArgs_t *_args):
        args(_args) {
    }

    void concurrentBrandes();
    void loadGraph();
    void computeBrandes();
    void saveBrandesData();
};


