#include <unordered_map>
#include <vector>
#include <atomic>
#include <string>

#include "utils.h"


typedef unsigned int vertex;


class FastGraph {
 private:
    std::vector< vertex > vertices;
    std::unordered_map< vertex, int > renumerationTable;//Converts from new indices to old
    std::vector< int > indices;
    std::vector< vertex > csr;
    std::vector< double > scores;
    globalArgs_t * const args = NULL;

    std::vector< std::vector< double > > threadScores;

    void threadFunction(unsigned int id, unsigned int finishIndex, std::atomic<unsigned int>& counter, std::atomic<bool>& shouldBeRunning, std::atomic<unsigned int>& runningThreads);

    void threadFunction_noAutosave(unsigned int id, unsigned int m_startID, unsigned int m_finishID, std::vector<unsigned int>& progressVector);

 public:
    FastGraph(globalArgs_t *_args):
        args(_args) {
    }

    void loadGraph();
    void serialBrandes();
    void threadedBrandes();
    void threadedBrandes_noAutosave();
    void saveResult(std::string = "", bool noPrinting = false);
};
