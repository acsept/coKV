#include<iostream>
#include <stdio.h>
#include "thread.h"

// using namespace std;

void text(){
    std::cout<<"text--------"<<std::endl;
}

int main(){
    server::Thread th(text);

    getchar();


    return 0;
}