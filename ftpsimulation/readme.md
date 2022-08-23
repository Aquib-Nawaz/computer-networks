Steps to Run the Assignment:
1)Fire up Two terminal Windows
2)In the first window run $gcc -o sender sender.c
3)In the second window run $gcc -o receiver receiver.c
4)Set an appropriate network delay for the loopback interface 
5)In the second start the reciever to listen using this command $./receiver 8000 8080 0.4
6)In the above example command the receiver port is 8000, sender port is 8080 and gen_prob is 0.4
7)In the first window start the sender by this command $./sender 8080 8000 1 10
8)In the above example command the receiver port is 8000, sender port is 8080 , number of packets is 10, and retransmission timer is 1 sec
9)Both the programs halt since we are deliberately stopping the reciever by sending a hardcoded message
