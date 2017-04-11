# drone-video-index


###
# comment for building TetraR-tree 
#
make clean
make build-tetrartree
./run.sh


###
# comment for monitoring memory usage 
#
./memusg.sh ./run.sh


###
# comment for monitoring i/o costs 
#
$Terminal1: ./run.sh
$Terminal2: sudo iotop -botqqqk -d 10 > /var/www/html/YingSpace/drone-video-index/aerial-fov-tetrartree/iotop.log 
python ./iotop-log-file-parser.py iotop.log parsed-iotop.log 29591 # inputLogFilename, outputFilename, pID
