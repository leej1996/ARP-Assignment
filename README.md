# ARP-Assignment
Assignment for Advanced Robot Programming Course

The purpose of this assignment was too create a multi process system that works in a network of identical systems that are on the same LAN network. They are connected through socket and exchange a data token at a specific speed. Due to Covid-19, the parameters were changed so that the project would work on one computer instead. 

The main process is in the Assignment1.c file. This creates several branches, with the main branch being called process P. This computes the data token and will send it to the G process via socket. P will also send data to a process called L in order to log it in a log file. P receives tokens from G as well via pipe. The final process is called S and takes posix signals from the terminal and changes the behaviour of P accordingly. 

To set up the system, you must update the config file accordingly. Please look at the howto.txt file for instructions on this. 
