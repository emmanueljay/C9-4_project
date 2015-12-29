for file in `ls ../data/v[0-9].dat`; do ./run  --glouton $file -L 0 -N 2; done;
