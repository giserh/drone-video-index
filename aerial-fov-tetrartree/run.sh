#!/bin/bash

### Index Building

TXT_OBJECT_QUATRILATERAL="../drone-metadata-generator/data/extended-quatrilaterals.csv"

INDEX_FILE="./extended-4KB-correct-opt-tetrartree.index"
BUILD_INDEX_LOG="./build-index.log"
QUERY_LOG="./query.log"

#./memusg.sh
./build-tetrartree.exe ${TXT_OBJECT_QUATRILATERAL} ${INDEX_FILE} ${BUILD_INDEX_LOG} 1





### Point query with TetraR-tree

INDEX_ROOT_ID="207"
QUERY_LAT="47.687851"
QUERY_LNG="8.699875"

#./pointQuery-with-tetrartree.exe ${INDEX_FILE} ${INDEX_ROOT_ID} ${QUERY_LOG} ${QUERY_LAT} ${QUERY_LNG}


QUERY_FILE="../drone-metadata-generator/queries/extended-point-queries-sample1K.txt"
#./pointQuery-with-tetrartree.exe ${INDEX_FILE} ${INDEX_ROOT_ID} ${QUERY_LOG} ${QUERY_FILE}
