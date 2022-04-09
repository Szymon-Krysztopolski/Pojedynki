#include "duel.h"

Player::Player(int tid, int size){
    this->tid=tid;
    this->size=size;

    this->vector_timer=new int[size];
    this->vector_modes=new int[size];
    this->lamport_timer=0;

    this->ready=1; //(mode==0) ready4fight || ready4help (mode==1)
}

Veteran::Veteran(int tid, int size)
:Player(tid,size)
{
    ;
}

Support::Support(int tid, int size)
:Player(tid,size)
{
    ;
}