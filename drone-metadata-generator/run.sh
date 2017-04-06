#!/bin/bash

LATITUDE="34.021223"
LONGITUDE="-118.287164"
HEIGHT="160"
AZIMUTH="0"
PITCH="20"
ROLL="20"
VIEWABLE_ANGLE="80"

### extended:
#MIN_LAT="47.313494"
#MIN_LNG="7.999397"
#MAX_LAT="48.014538"
#MAX_LNG="9.100777"

### raw:
MIN_LAT="47.597"
MIN_LNG="8.5239 "
MAX_LAT="47.608"
MAX_LNG="8.5403"

MIN_RADIUS="0.1"
MAX_RADIUS="1.0"
MIN_DIR="0.0"
MAX_DIR="359.0"


RAW_DATA_FILE="./data/raw-data.csv" #raw-data file
EXTENDED_DATA_FILE="./data/extended-data.csv"
QUATRILATERAL_FILE="./data/raw-quatrilaterals.csv"



BATCH_SIZE="130"
#./data-extension.exe ${RAW_DATA_FILE} ${EXTENDED_DATA_FILE} ${BATCH_SIZE}




#./calc-quatrilaterals.exe ${RAW_DATA_FILE} ${QUATRILATERAL_FILE}
#./calc-quatrilaterals.exe ${LATITUDE} ${LONGITUDE} ${HEIGHT} ${AZIMUTH} ${PITCH} ${ROLL} ${VIEWABLE_ANGLE}



DEAD_SPACE_AREA_FILE="./data/dead_space_ratio.csv"
#./area-comparison.exe ${QUATRILATERAL_FILE} ${DEAD_SPACE_AREA_FILE}




QUERY_NUM="10000"
QUERIES_FILE="./queries/raw-point-queries.txt"
QUERY_TYPE="point_query"
./generate-queries.exe ${QUERIES_FILE} ${QUERY_TYPE} ${MIN_LAT} ${MIN_LNG} ${MAX_LAT} ${MAX_LNG} ${QUERY_NUM}

QUERIES_FILE="./queries/raw-range-queries.txt"
QUERY_TYPE="range_query"
./generate-queries.exe ${QUERIES_FILE} ${QUERY_TYPE} ${MIN_LAT} ${MIN_LNG} ${MAX_LAT} ${MAX_LNG} ${QUERY_NUM} ${MIN_RADIUS} ${MAX_RADIUS}

QUERIES_FILE="./queries/raw-directional-queries.txt"
QUERY_TYPE="directional_query"
./generate-queries.exe ${QUERIES_FILE} ${QUERY_TYPE} ${MIN_LAT} ${MIN_LNG} ${MAX_LAT} ${MAX_LNG} ${QUERY_NUM} ${MIN_DIR} ${MAX_DIR}





