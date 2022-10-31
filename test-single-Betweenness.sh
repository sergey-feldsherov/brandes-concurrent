#!/bin/bash

DATASET=$1
DATASETEXTENSION=$2

mkdir -p $HOME/data/$DATASET/networkit-scores

python3.7 ./launcher.py \
	--algorithm=Betweenness \
	--input=$HOME/data/$DATASET/graph/$DATASET$DATASETEXTENSION \
	--output=$HOME/data/$DATASET/networkit-scores/$DATASET$DATASETEXTENSION.networkit.Betweenness.scores \
	--directed \
#	--renumerate \

