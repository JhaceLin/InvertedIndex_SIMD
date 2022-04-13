# test.sh
# !/bin/sh
pssh -h $PBS_NODEFILE -i "if [ ! -d \"/home/s2011283/InvertedIndex\" ];then mkdir -p \"/home/s2011283/InvertedIndex\"; fi" 1>&2
pscp -h $PBS_NODEFILE /home/s2011283/InvertedIndex /home/s2011283/InvertedIndex 1>&2
/home/s2011283/InvertedIndex/BAlgS0_ARM
