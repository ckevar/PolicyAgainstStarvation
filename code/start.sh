for I in $(seq 0 4)
 do
  xterm -e ./lpd $I &
 done
./manager
