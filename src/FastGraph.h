#include <unordered_map>
#include <vector>
#include <atomic>

#include "utils.h"


typedef unsigned int vertex;
typedef std::pair<vertex, vertex> edge;


class FastGraph {
 private:
    std::vector< vertex > vertices;
    std::unordered_map< vertex, int > renumerationTable;
    std::vector< int > indices;
    std::vector< vertex > csr;
    std::vector< double > scores;
    globalArgs_t * const args = NULL;

    std::vector< std::vector< double > > threadScores;

    void threadFunction(unsigned int id, unsigned int startIndex, unsigned int finishIndex, std::atomic<unsigned int>& counter);

 public:
    FastGraph(globalArgs_t *_args):
        args(_args) {
    }

    void loadGraph();
    void serialBrandes();
    void threadedBrandes();
    void saveResult();
};
