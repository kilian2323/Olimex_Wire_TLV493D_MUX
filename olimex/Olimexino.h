#ifndef OLIMEXINO_H
#define OLIMEXINO_H

#include <cstdint>
#include <unistd.h>
#include <string>
#include <math.h>

#define PI M_PI

using namespace std;


void delay(uint32_t ms);
void print(string s);
void println(string s);
void print(int i);
void println(int i);
void print(float f);
void println(float f);


#endif
