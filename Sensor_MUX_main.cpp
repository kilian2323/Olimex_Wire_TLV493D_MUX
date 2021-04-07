
#include "Sensor_MUX.h"


using namespace std;

Tlv493d sensor;


int main()
{    
    printf(">>>> Wire.begin(1)\n");
    Wire.begin((uint8_t)1); // /dev/i2c-1
    
    printf(">>>> tcaSelect(0)\n");
    tcaSelect(0);
    
    printf(">>>> sensor.begin()\n");
    sensor.begin();   
        
    printf(">>>> sensor.setAccessMode(...)\n");
    sensor.setAccessMode(sensor.FASTMODE);
                
    printf(">>>> sensor.disableTemp()\n");
    sensor.disableTemp();
    
    delay(5000);
    
    while(true)
    {
    
		int dly = sensor.getMeasurementDelay();
		delay(dly);
		
		Tlv493d_Error_t err = sensor.updateData();
		
		if(err == TLV493D_NO_ERROR)
		{
			float x = sensor.getX();
			float y = sensor.getY();		
			float z = sensor.getZ();	
			if(z < 0)
			{
				x *= -1;
				y *= -1;
				z *= -1;
			}	
			printf("%.2f\t%.2f\t%.2f\n",x,y,z);
		}
	}
	return 0;
     
}

void tcaSelect(uint8_t i) 
{
  Wire.beginTransmission(0x70);
  Wire.write(1 << i);
  Wire.endTransmission();   
}

