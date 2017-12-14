//
//  Index.cpp
//  
//
//  Created by David Harris on 2017-11-29.
//
//

#include "Index.h"

uint16_t Index::hashcalc(void* m, uint16_t s) {
    uint16_t hash = 0;
    for(int i=0; i<s; i++) hash += ((uint8_t*)m)[i];
    return hash;
}
void Index::set(uint16_t i, void* m, uint16_t s) {
    index = i;
    hash = hashcalc(m,s);
}
int Index::sortCompare(const void* aa, const void* bb) {
            //Serial.print(F("\nIn hashCompare:"));
    Index* a = (Index*)aa;
    Index* b = (Index*)bb;
            //Serial.print(F(" a=")); Serial.print(a->hash,HEX);
            //Serial.print(F(" b=")); Serial.print(b->hash,HEX);
    if(a->hash>b->hash) return 1;
    if(a->hash<b->hash) return -1;
    return 0;
}
int Index::findCompare(const void* aa, const void* bb) {
            //Serial.print(F("\nIn indexCompare:"));
    int a = *(int*)aa;
    Index* b = (Index*)bb;
            //Serial.print(F(" a=")); Serial.print(a,HEX);
            //Serial.print(F(" b=")); Serial.print(b->hash,HEX);
    if(a>b->hash) return 1;
    if(a<b->hash) return -1;
    return 0;
}

Index* Index::findIndex(void* ms, uint16_t s, uint16_t is, Index* start) {
                //Serial.print(F("\nIn Index::findIndex"));
                //Serial.print(F("\nis=")); Serial.print(is);
    Index* p = start;
    if(start>(this+is)) return nullptr;
    Index hh;
    hh.hash = hashcalc(ms,s);
                //Serial.print(F("\nms=")); Serial.print(ms);
                //Serial.print(F("\nhh=")); Serial.print(hh,HEX);
    if(start==0) {
        p = (Index*)bsearch(&hh, this, is, sizeof(Index), findCompare);
        if(!p) return nullptr;
                //Serial.print(F("\ni=")); Serial.print(p->index);
                //Serial.print(F("\nBackup"));
        while((p-1)>=this && (p-1)->hash==hh.hash) {p--;}
                //Serial.print(F("\np=")); Serial.print(p->index);
    } else {
                //Serial.print(F("\nnext:"));
        p++;
        if(start>(this+is)) return nullptr;
                //Serial.print(F("\np=")); Serial.print(p->index);
        if(p->hash==hh.hash) return p;
        else return nullptr;
    }
                //Serial.print(F("\np->hash=")); Serial.print(p->hash);
    if(hh.hash==p->hash) return p;
    return nullptr;
}
/*
 Index* Index::findIndexOfMsg(Msg* ms, uint16_t s, uint16_t is, Index* start) {
 Serial.print(F("\nIn findIndexOfMsg:"));
 Serial.print(F("\nms=")); Serial.print(ms);
 Serial.print(F("\ns=")); Serial.print(s);
 Serial.print(F("\nis=")); Serial.print(is);
 Serial.print(F("\nstart.hash=")); Serial.print(start->hash);
 Index* i = this->findIndex(ms, s, is, start);
 if(i!=nullptr) return i;
 return nullptr;
 }
 */
 /*
 int Index::findIndexOfEventID(EventID* eid, uint16_t is, uint16_t start) {
   //int i = this->findIndex(eid, sizeof(*eid), is, start);
   //if(i<0) return -1;
   //if(equal(eid,channels[i].eventid,8)) return i;
   //return -1;
     
     Index* i = this->findIndex(eid, sizeof(*eid), is, start);
     return i->index;
 }
 */

void Index::print() {
    Serial.print(F("["));Serial.print(hash,HEX);
    Serial.print(F(","));Serial.print(index,HEX);
    Serial.print(F("]"));
}

void Index::print(uint16_t n) {
    Serial.print(F("\nIndex::"));
    for(int i=0;i<n;i++) {
        Serial.print("\n");Serial.print(i);Serial.print(" ");
        (this+i)->print();
    }
}
void Index::sort(uint16_t n) {
    Serial.print(F("\nsize="));Serial.print(sizeof(this));
    qsort(this, n, sizeof(*this), Index::sortCompare);
}

//EventID Index::getEID() {}

/*  continuation of subclassing
 void Index2::print() {
 Serial.print(F("["));Serial.print(hash);
 Serial.print(F(","));Serial.print(index);
 Serial.print(F(","));Serial.print(index2);
 Serial.print(F("]"));
 }
 
 void Index2::print(uint16_t n) {
 Serial.print(F("\nIndex2::"));
 for(int i=0;i<n;i++) {
 Serial.print("\n");Serial.print(i);Serial.print(" ");
 (this+i)->print();
 }
 }
 void Index2::sort(uint16_t n) {
 Serial.print(F("\nsize="));Serial.print(sizeof(this));
 qsort(this, n, sizeof(*this), Index::hashCompare);
 }
 */