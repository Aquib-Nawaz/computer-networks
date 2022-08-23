#!/bin/bash
sudo ifconfig lo mtu 1500
$(yes | head -c 5MB > send.txt)
$(gcc -o receiver receiver.c -lm)
$(gcc -o sender sender.c -lm)
echo congestion_control,loss,delay,avg_throughput,std_dev > resx.csv
echo loss,delay,avg_throughput,std_dev
for algo in cubic reno
do
sudo sysctl net.ipv4.tcp_congestion_control=$algo          #setting tcp varaint from outside the code 
sudo tc qdisc del dev lo root
sudo tc qdisc add dev lo root netem loss 0.1%
for loss in 0.1% 0.5% 1% 
do
for delay in 10ms 50ms 100ms
do
sudo tc qdisc change dev lo root netem delay $delay loss $loss           #setting delay and loss parameters
touch res.txt
./receiver 1 recv1.txt $(($1+1)) >> res.txt & ./sender send.txt 127.0.0.1 $(($1+1)) &
./receiver 1 recv2.txt $(($1+2)) >> res.txt & ./sender send.txt 127.0.0.2 $(($1+2)) &
./receiver 1 recv3.txt $(($1+3)) >> res.txt & ./sender send.txt 127.0.0.3 $(($1+3)) &
./receiver 1 recv4.txt $(($1+4)) >> res.txt & ./sender send.txt 127.0.0.4 $(($1+4)) &
./receiver 1 recv5.txt $(($1+5)) >> res.txt & ./sender send.txt 127.0.0.5 $(($1+5)) &
./receiver 1 recv6.txt $(($1+6)) >> res.txt & ./sender send.txt 127.0.0.6 $(($1+6)) &
./receiver 1 recv7.txt $(($1+7)) >> res.txt & ./sender send.txt 127.0.0.7 $(($1+7)) &
./receiver 1 recv8.txt $(($1+8)) >> res.txt & ./sender send.txt 127.0.0.8 $(($1+8)) &
./receiver 1 recv9.txt $(($1+9)) >> res.txt & ./sender send.txt 127.0.0.9 $(($1+9)) &
./receiver 1 recv10.txt $(($1+10)) >> res.txt & ./sender send.txt 127.0.0.10 $(($1+10)) &
./receiver 1 recv11.txt $(($1+11)) >> res.txt & ./sender send.txt 127.0.0.11 $(($1+11)) &
./receiver 1 recv12.txt $(($1+12)) >> res.txt & ./sender send.txt 127.0.0.12 $(($1+12)) &
./receiver 1 recv13.txt $(($1+13)) >> res.txt & ./sender send.txt 127.0.0.13 $(($1+13)) &
./receiver 1 recv14.txt $(($1+14)) >> res.txt & ./sender send.txt 127.0.0.14 $(($1+14)) &
./receiver 1 recv15.txt $(($1+15)) >> res.txt & ./sender send.txt 127.0.0.15 $(($1+15)) &
./receiver 1 recv16.txt $(($1+16)) >> res.txt & ./sender send.txt 127.0.0.16 $(($1+16)) &
./receiver 1 recv17.txt $(($1+17)) >> res.txt & ./sender send.txt 127.0.0.17 $(($1+17)) &
./receiver 1 recv18.txt $(($1+18)) >> res.txt & ./sender send.txt 127.0.0.18 $(($1+18)) &
./receiver 1 recv19.txt $(($1+19)) >> res.txt & ./sender send.txt 127.0.0.19 $(($1+19)) &
./receiver 1 recv20.txt $(($1+20)) >> res.txt & ./sender send.txt 127.0.0.20 $(($1+20)) &
wait
v=$(awk '{for(i=1;i<=NF;i++) {sum[i] += $i; sumsq[i] += ($i)^2}} 
          END {for (i=1;i<=NF;i++) {
          printf "%f,%f \n", sum[i]/NR, sqrt((sumsq[i]-sum[i]^2/NR)/NR)}
         }' res.txt) #calculate the std_deviation value through script by running each code only once instead of 10 times
rm res.txt
echo $loss,$delay,$v
echo $algo,$loss,$delay,$v >> resx.csv
done
done 
done 
rm recv1.txt recv2.txt recv3.txt recv4.txt recv5.txt recv6.txt recv7.txt recv8.txt recv9.txt recv10.txt
rm recv11.txt recv12.txt recv13.txt recv14.txt recv15.txt recv16.txt recv17.txt recv18.txt recv19.txt recv20.txt
