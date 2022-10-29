# Brandes algorithm for Betweenness Centrality

This project implements coarse-grained parallelism for Brandes algorithm for unweighted graphs.
The shortest path computation task is split evenly across all threads.
Additionally, a data reduction step is introduced after threads are joined.

## optimize me
single-thread runs
|    dataset | renumerated? | brandes loop time |
| ---------- | ------------ | ----------------- |
| ca-AstroPh |           no |            02m30s |

## Usage

* `-u` or `--undirected` to treat graph as undirected, i.e. each edge (u -> v) from input file will correspond to two edges. Only one instance of edge is required to create an undirected edge. If not included, the graph is considered to be directed.
* `-i PATH` or `--input=PATH` to provide an input graph. Format: edgelist, any lines starting with `#` are ignored.
* `-o PATH` or `--output=PATH` to provide output file for ranking. Default value: `./output/output.ranking`. Format: `NODE SCORE`, scores are not normalized.
* `-t T` or `--threads=T` to provide how many threads should be used. If the number is negative or zero, a serial version of algorithm starts. Otherwise, T threads are allocated. Default value: 0.

## TODO

* Switch to inhouse argument-parsing solution
* Refactor ProgressBar
* Solve workarounds used for thread pausing
* Switch to thread pool with task queue for better concurrency
* Update **Usage** section and description
