#!/bin/bash
#Run this in terminal
#+ Command to compile c++ program. here i used common one
g++ Server.cpp -o server -lpthread
g++ Client.cpp -o client 
exit 0