# ARP-Assignment
Assignment for Advanced Robot Programming Course

The purpose of this assignment is to create a multi process system that works in a network of identical systems that are on the same LAN network. They are connected through socket and exchange a data token at a specific speed. 

There are four posix processes in the system. The main process is in the Assignment1.c file. This computes the data token and will send it to the next machine. It will send data to a process called L in order to log it in a log file. The main process receives tokens from a process called G, which is given by the an adjacent machine and recieves tokens from that machine via socket. The final process is called S and takes posix signals from the terminal and changes the behaviour of P accordingly. 
