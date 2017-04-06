#!/bin/bash

### Index Building

TXT_OBJECT_QUATRILATERAL="../drone-metadata-generator/data/raw-quatrilaterals.csv"
BINARY_OBJECT_QUATRILATERAL="../drone-metadata-generator/data/raw-binary-quatrilaterals.dat"

INDEX_FILE="./temp-rtree.index"
BUILD_INDEX_LOG="./build-index.log"


./build-rtree.exe ${TXT_OBJECT_QUATRILATERAL} ${INDEX_FILE} ${BUILD_INDEX_LOG}





#./object-file-txt2binary.exe ${TXT_OBJECT_QUATRILATERAL} ${BINARY_OBJECT_QUATRILATERAL} transfer
#./object-file-txt2binary.exe ${TXT_OBJECT_QUATRILATERAL} ${BINARY_OBJECT_QUATRILATERAL} read 1






### Point query with R-tree

INDEX_ROOT_ID="208"
QUERY_LAT="47.687851"
QUERY_LNG="8.699875"
QUERY_LOG="./query.log"

#./pointQuery-with-rtree.exe ${INDEX_FILE} ${INDEX_ROOT_ID} ${BINARY_OBJECT_QUATRILATERAL} ${QUERY_LOG} ${QUERY_LAT} ${QUERY_LNG}

QUERY_FILE="../drone-metadata-generator/queries/raw-point-queries.txt"
#./pointQuery-with-rtree.exe ${INDEX_FILE} ${INDEX_ROOT_ID} ${BINARY_OBJECT_QUATRILATERAL} ${QUERY_LOG} ${QUERY_FILE}



