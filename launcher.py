import networkit as nk
import argparse
import time
import random


timeStart = time.time()

parser = argparse.ArgumentParser(description='Run Networkit\'s KADABRA implementation.')

allowedAlgorithms = ['Betweenness', 'ApproxBetweenness', 'EstimateBetweenness', 'KadabraBetweenness']

parser.add_argument("-i", "--input", type=str, default="./input.txt", help='input file in SNAP format')
parser.add_argument("-o", "--output", type=str, default="./output.txt",  help='output file with BC scores')
parser.add_argument("--err", type=float, default=0.001, help='algorithm hyperparameter where applicable')
parser.add_argument("--epsilon", type=float, default=0.01, help='algorithm hyperparameter where applicable')
parser.add_argument("--delta", type=float, default=0.1, help='algorithm hyperparameter where applicable')
parser.add_argument("--nSamples", type=int, default=100, help='algorithm hyperparameter where applicable')
parser.add_argument("--parallel", action='store_true', help='use parallel implementation where applicable')
parser.add_argument("--top", type=int, default=10000, help='number of top vertices to compute where applicable')
parser.add_argument("--directed", action='store_true', help='treat input graph as directed')
parser.add_argument("--normalized", action='store_true', help='return normalized scores where applicable')
parser.add_argument("--renumerate", action='store_true', help='renumerate input graph before computation')
parser.add_argument("--algorithm", type=str, default="ApproxBetweenness", choices=allowedAlgorithms, help='an algorithm to use for betweenness computations')

args = parser.parse_args()


print("Input graph treated as directed: {}".format(args.directed))
print("Input will be renumerated: {}".format(args.renumerate))

print("Reading graph from: ", args.input)
myReader = nk.graphio.SNAPGraphReader(directed=args.directed, remapNodes=args.renumerate)
#myReader = nk.graphio.EdgeListReader(separator=" ", continuous=True, directed=False)
G = myReader.read(args.input)
print("Graph reading finished")
print("Vertices: ", G.numberOfNodes())
print("Edges: ", G.numberOfEdges())

if(args.algorithm == 'Betweenness'):
	print("Running Betweenness")
	bc = nk.centrality.Betweenness(G)
elif(args.algorithm == 'KadabraBetweenness'):
	print("Running KadabraBetweenness(KADABRA by Borassi, Natale) with delta={}, err={}, k={}".format(args.delta, args.err, args.top))
	nk.setSeed(42, True) # setting the seed is required for KADABRA
	bc = nk.centrality.KadabraBetweenness(G, err=args.err, delta=args.delta, k=args.top)
elif(args.algorithm == 'ApproxBetweenness'):
	print("Running ApproxBetweenness(Fast Approximation by Riondato, Kornaropoulos) with delta={}, epsilon={}".format(args.delta, args.epsilon))
	bc = nk.centrality.ApproxBetweenness(G, epsilon=args.epsilon, delta=args.delta)
elif(args.algorithm == 'EstimateBetweenness'):
	print('Running EstimateBetweenness(Better Approximation by Geisberger et al.) with nSamples={}'.format(args.nSamples))
	print('Parallel: {}', args.parallel)
	print('Normalized: {}', args.normalized)
	bc = nk.centrality.EstimateBetweenness(G, args.nSamples, parallel=args.parallel, normalized=args.normalized)

bc.run()

bcScores = bc.ranking()
print("Writing scores to: ", args.output)
f = open(args.output, 'w')
for v, c in bcScores:
    f.write("{} {}\n".format(v, c))

f.close()

timeEnd = time.time()
print("Total execution time: ", timeEnd - timeStart)

