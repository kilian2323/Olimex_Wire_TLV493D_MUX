/**
 * Kilian Ochs, 07.04.2021
 * Center for Biorobotics, TALTECH, Tallinn
 * Robominers experimental setup
 * 
 * This sketch is used to interface magnetic Hall sensors of type TLV493D
 * using several multiplexers of type TCA9548A. The number of sensors per multiplexer must be the
 * same for all multiplexers.
 * 
 * Continuously reads the sensors one by one, prints all sensor readings to Console.         
 * When reading the Console outputs during manual verification, the format mf = PlainText should be used.
 *
 * The format mf = Compressed packs each sensor float value into a 16bit integer. The result is stored in
 * txString, which can be used for data acquisition purposes in the future (send to ROS or via Serial).
 * 
 * To be compiled for a Linux-based system.
 * This is a ported version of the library made for the  * Adafruit Feather M0
 * (see gitHub: kilian2323/3dMagSensorsVisual).
 */

#include "Sensor_MUX.h"
#include "./olimex/conio.h"

#define MUX_STARTADDR 0x70  // [0x70] Address of the first multiplexer; the others must be consecutive
#define NUM_MUX 1           // Number of multiplexers
#define NUM_SENSORS 2       // Number of sensors per multiplexer (max. 8)
#define MAXBUF 1000         // Maximum char length of an output message



const bool fastMode = true;		       // [true] if false, the sensor readings come more slowly, but might be more accurate

const unsigned char endSignature[2] = {'\r','\n'};     // for Compressed message format: end of message  
const bool sendPolarRadius = false;    // [false] if true, the polar radius will be sent as the third value in Spherical mode using Compressed message format,
                                       //         otherwise it will be omitted (only two values will be sent)
const int multiplier = 100;            // [100] used for Compressed message format. Higher value: better accuracy, but lower range of values to send

const Type t = Spherical;              // [Cartesian] type of representation of sensor data: Cartesian or Spherical
const MessageFormat mf = PlainText;    // [PlainText] format for serial messages: PlainText or Compressed




//////////// End of user-defined constants /////////////



Tlv493d *Tlv493dMagnetic3DSensor[NUM_MUX][NUM_SENSORS]; // 2D array of pointers to Tlv493d Object
//unsigned long lastLoop = 0;
//float loopFrequency;
unsigned char * txString = new unsigned char[MAXBUF];
uint8_t txIndex = 0;
unsigned char encodeResult[2];
float data[NUM_MUX][NUM_SENSORS][3];
bool init = true;

using namespace std;


int main()
{    
    setup();
    loop();    
    Wire.end();
    debug("\n>>>> Good-bye!\n\n");
	return 0;     
}




void setup()
{
	debug(">>>> RESET\n");	
	
	debug(">>>> Wire.begin(1)\n");
    Wire.begin((uint8_t)1); // /dev/i2c-1
    
    debug(">>>> Constructing objects of TLV493D\n");
	for(uint8_t m=0; m<NUM_MUX; m++)
	{
		for(uint8_t i=0; i<NUM_SENSORS; i++)
		{
			Tlv493dMagnetic3DSensor[m][i] = new Tlv493d();
		}
	}
	
	debug(">>>> Initializing %d sensor(s) TLV493D\n",NUM_SENSORS);
	for(uint8_t m=MUX_STARTADDR; m<MUX_STARTADDR+NUM_MUX; m++)
	{
		debug("     on MUX address 0x%02X\n",m);
		muxDisablePrevious(m); 

		for(uint8_t i=0; i<NUM_SENSORS; i++)
		{
			tcaSelect(m,i);
			initSensor(m-MUX_STARTADDR,i);			
		}
	}	
	testAndReinitialize(); // This is some dirty hack to avoid "badly initialized" (?) sensors in Linux
	init = false;
	#ifdef DEBUG
	debug(">>>> Setup ended. Waiting for some seconds...\n");
	delay(5000);
	#endif
}

void loop()
{
	while(true)
    {    
		char key = '\0';
		if(getInput(&key))
		{
			if(key == 'q')
			{
				return;
			}
		}
		
		
		for(uint8_t m=MUX_STARTADDR; m<MUX_STARTADDR+NUM_MUX; m++)
		{
			// Deselect all channels of the previous multiplexer    
			muxDisablePrevious(m);
			
			for(uint8_t i=0; i<NUM_SENSORS; i++)
			{ 
			  // Switch to next multiplexed port  			  
			  tcaSelect(m,i);   			  
		  
			  // Read sensor with selected type of representation
			  readSensor(m-MUX_STARTADDR,i); 		  
			}    
		}
		// Print readings
		printOut();
	}
}


void testAndReinitialize()
{
	for(uint8_t m=MUX_STARTADDR; m<MUX_STARTADDR+NUM_MUX; m++)
	{
		// Deselect all channels of the previous multiplexer    
		muxDisablePrevious(m);
		
		for(uint8_t i=0; i<NUM_SENSORS; i++)
		{ 
		  // Switch to next multiplexed port  			  
		  tcaSelect(m,i);   
		  debug(">>>> Checking sensor %d.%d\n",m-MUX_STARTADDR,i);
		  
		  bool ok = false;	
		  int attempts = 0;	  
		  
		  while(!ok && attempts < 10)
		  {
			  debug("     Attempt: %d/10\n",(attempts+1));
			  int readNum = 0;
			  while(readNum < 5)
			  {
				  debug("       Reading data (%d/5)\n",(readNum+1));
				  readSensor(m-MUX_STARTADDR,i); 
				  readNum++;	
				  if(data[m-MUX_STARTADDR][i][0] != 0 && data[m-MUX_STARTADDR][i][1] != 0 && data[m-MUX_STARTADDR][i][2] != 0)
				  {
					  ok = true;
					  break;
				  }
				  if(readNum == 5)
				  {
					debug("     Invalid data; reinitializing\n");
					initSensor(m-MUX_STARTADDR,i);
				  }		  
			  }
			  attempts++;
		  }		
		  if(!ok)
		  {
			  debug("     Failed to initialize this sensor.\n");
		  }	 
		  else
		  {
			  debug("     Sensor ok.\n");
		  } 
	  		    
		}    
	}
}


void tcaSelect(uint8_t m, uint8_t i) 
{
  if(init == false && NUM_MUX == 1 && NUM_SENSORS == 1)
  {
	  return;
  }
  Wire.beginTransmission(m);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void muxDisablePrevious(uint8_t m)
{
  if(NUM_MUX == 1)
  {
    return;
  }
  uint8_t muxToDisable;
  if(m == MUX_STARTADDR)
  {
    muxToDisable = MUX_STARTADDR + NUM_MUX - 1;
  }
  else
  {
    muxToDisable = m - 1;
  }
  tcaDisable(muxToDisable);
}

void tcaDisable(uint8_t addr)
{
  Wire.beginTransmission(addr);
  Wire.write(0);
  Wire.endTransmission();
  delay(5); 
}

void initSensor(uint8_t m, uint8_t i)
{
  Tlv493dMagnetic3DSensor[m][i]->begin();
  if(fastMode)
  {
	Tlv493dMagnetic3DSensor[m][i]->setAccessMode(Tlv493dMagnetic3DSensor[m][i]->FASTMODE);
  }
  Tlv493dMagnetic3DSensor[m][i]->disableTemp();  
}

void readSensor(uint8_t m, uint8_t i)
{
  int dly = Tlv493dMagnetic3DSensor[m][i]->getMeasurementDelay();
  delay(dly);
  Tlv493d_Error_t err = Tlv493dMagnetic3DSensor[m][i]->updateData();
  if(err != TLV493D_NO_ERROR)
  {
    return;
  }  

  if(t == Spherical)
  {      
    data[m][i][0] = radToDeg(Tlv493dMagnetic3DSensor[m][i]->getAzimuth());                       // angle in xy plane relative to x axis
    data[m][i][1] = radToDeg(Tlv493dMagnetic3DSensor[m][i]->getPolar());                         // inclination angle relative to x axis
    data[m][i][2] = Tlv493dMagnetic3DSensor[m][i]->getAmount();                                  // distance between magnet and sensor
  }
  else if(t == Cartesian)
  {
    data[m][i][0] = Tlv493dMagnetic3DSensor[m][i]->getX();
    data[m][i][1] = Tlv493dMagnetic3DSensor[m][i]->getY();
    data[m][i][2] = Tlv493dMagnetic3DSensor[m][i]->getZ();       
  }   
  
  // results are according to sensor reference frame if the third value is non-negative (not tested for Spherical)
  if(data[m][i][2] < 0) { 
    data[m][i][2] *= -1;
    data[m][i][1] *= -1;
    data[m][i][0] *= -1;
  }   
}

void printOut()
{
  if(mf == PlainText)
  {    
    for(uint8_t m=0;m<NUM_MUX;m++)
    {  
      printf("{");
      for(uint8_t i=0;i<NUM_SENSORS;i++)
      {
        printf("[");
        for(uint8_t j=0;j<3;j++)
        {
          printf("%.2f",data[m][i][j]);
          if(j < 2)
          {
            printf(";");    
          }
        }    
        printf("]");
      }
      printf("}");
    }
    printf("\n");
     
  } else {   

    if (mf == Compressed)
    {    
      for(uint8_t m=0;m<NUM_MUX;m++)
      {
        for(uint8_t i=0;i<NUM_SENSORS;i++)
        {
          uint8_t numValues = 3;
          if(t == Spherical && !sendPolarRadius)
          {
            numValues = 2;
          }
          for(int j=0;j<numValues;j++)
          {
            encode(data[m][i][j]);
            writeTx(encodeResult[0]);
            writeTx(encodeResult[1]);
          }
        }
      }          
    } 
    writeTx(endSignature[0]);
    writeTx(endSignature[1]);  
    
    print(txString,txIndex); 
    txIndex = 0;
  }
}

// Encoding as Little Endian
void encode(float f)
{  
  int16_t s = f * multiplier;  // multiplying the float by 100, then dropping the last 16 bits
  
  encodeResult[1] = s >> 8;
  encodeResult[0] = s & 0x00ff;     
}

float radToDeg(float rad)
{
  return rad * 180.0 / PI;
}

void writeTx(unsigned char c) 
{
  if(txIndex >= MAXBUF) { return; }
  txString[txIndex] = c;
  txIndex++;
}

bool getInput(char *c)
{
	if(kbhit())
	{
		*c = getch();
		return true;
	}
	return false;
}

