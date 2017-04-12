This is a parser for the log file "iotop.log" output by the "iotop" command:
"sudo iotop -botqqqk -d 10 > ./iotop.log"

The input and output parameters are described at beginning of the python file.

===========
Exc command example to run this parser:
"python ./iotop-log-file-parser.py ./iotop.log ./parsed-iotop.log 10171"

Explain: 
./iotop.log is the output log file of the "iotop" command above;
./parsed-iotop.log is the output file of this parser;
10171 is the pid a process "build-tetrartree".
===========


Related links about iotop:
http://www.binarytides.com/monitor-disk-io-iotop-cron/
https://linux.die.net/man/1/iotop
http://man.linuxde.net/iotop