#ifndef SENSOR_MUX_MAIN_H
#define SENSOR_MUX_MAIN_H

#include "./src/Tlv493d.h"
#include "./src/util/RegMask.h"
#include "./src/util/BusInterface2.h"

void tcaSelect(uint8_t,uint8_t);
void tcaDisable(uint8_t);
void muxDisablePrevious(uint8_t);
void initSensor(uint8_t, uint8_t);
void readSensor(uint8_t, uint8_t);
void encode(float);
float radToDeg(float);
void writeTx(unsigned char);
void printOut();
bool getInput(char *);
void testAndReinitialize();

void setup();
void loop();


enum Type {  
  Cartesian,
  Spherical
};

enum MessageFormat {  
  PlainText,        // PlainText: Full string, in Cartesian up to 1424 bytes in total for 8x8
  Compressed        // Compressed: Cartesian: 3 values per sensor,                               3 x 2 bytes per sensor, 388 bytes in total for 8x8
                    //             Spherical: 2 values per sensor (if sendPolarRadius is false), 2 x 2 bytes per sensor, 260 bytes in total for 8x8
};



#endif
