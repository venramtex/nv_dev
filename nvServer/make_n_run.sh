# make_n_run.sh
#usage: ./make_n_run.sh "executable to run"
#Ex: ./make_n_run.sh nvServer -> will make the file and run it
#    ./make_n_run.sh -> will make the file but doesn't run
#!/bin/bash
echo $# arguments $@
#if [ $# == 1 ];
#    then echo Argument $1 " passed"
#fi
make clean
#make "$@" 	#$@: all arguments supplied
make
 
if [ $# == 1 ];
    then echo Argument $1 " passed"
    ./"$1" &
fi

