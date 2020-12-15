#include <unordered_map>
#include <vector>
#include <atomic>
#include <string>

#include "utils.h"


typedef unsigned long long vertex;


class FastGraph {
 private:
    bool weighted;
    std::vector< vertex > vertices;
    std::unordered_map< vertex, unsigned long long > renumerationTable;//Converts from new indices to old
    std::vector< unsigned long long > indices;
    std::vector< vertex > csr;
    std::vector< double > weights;
    std::vector< double > scores;
    globalArgs_t * const args = NULL;

    std::vector< std::vector< double > > threadScores;

    void threadFunction(unsigned int id, unsigned long long int finishIndex, std::atomic<unsigned long long>& counter, std::atomic<unsigned long long>& finishedTasks, std::atomic<bool>& shouldBeRunning, std::atomic<unsigned int>& runningThreads);

 public:
    FastGraph(globalArgs_t *_args):
        args(_args) {
    }

    void loadGraph();
    void serialBrandes();
    void threadedBrandes();
    void saveResult(std::string = "", bool noPrinting = false);
};
