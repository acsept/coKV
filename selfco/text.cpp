#include "selfco.h"
#include <iostream>


void text(){
    std::cout<<"text"<<std::endl;
}

void texttow(){
    std::cout<<"text2"<<std::endl;
}

int main(){
    server::Selfco::getThis();
    server::Selfco::ptr cur(new server::Selfco(text));
    cur->resuem();
    server::Selfco::ptr cu(new server::Selfco(texttow));
    cu->resuem();


}