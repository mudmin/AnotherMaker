//This is some test code for the following three Keyestudio sensors by AnotherMaker
//You can buy them here using my links to support the channel
//CO2 Sensor - https://amzn.to/3grX9vW
//TDS Sensor - https://amzn.to/2DCz7Qo
//Turbidity Sensor - https://amzn.to/33pjMNE
// https://youtube.com/AnotherMaker

//The CO2 sensor requires
#include <CCS811.h>
#define TdsSensorPin A1
#define VREF 5.0 // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point
int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

//CO2 sensor
/*
 * IIC address default 0x5A, the address becomes 0x5B if the ADDR_SEL is soldered.
 */
//CCS811 sensor(&Wire, /*IIC_ADDRESS=*/0x5A);
CCS811 sensor;

void setup()
{
Serial.begin(9600);
pinMode(TdsSensorPin,INPUT);

/*Wait for the chip to be initialized completely, and then exit*/
while(sensor.begin() != 0){
    Serial.println("failed to init chip, please check if the chip connection is fine");
    delay(1000);
}
/**
 * @brief Set measurement cycle
 * @param cycle:in typedef enum{
 *                  eClosed,      //Idle (Measurements are disabled in this mode)
 *                  eCycle_1s,    //Constant power mode, IAQ measurement every second
 *                  eCycle_10s,   //Pulse heating mode IAQ measurement every 10 seconds
 *                  eCycle_60s,   //Low power pulse heating mode IAQ measurement every 60 seconds
 *                  eCycle_250ms  //Constant power mode, sensor measurement every 250ms
 *                  }eCycle_t;
 */
sensor.setMeasCycle(sensor.eCycle_250ms);


}
void loop()
{
static unsigned long analogSampleTimepoint = millis();
if(millis()-analogSampleTimepoint > 40U) //every 40 milliseconds,read the analog value from the ADC
{
analogSampleTimepoint = millis();
analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //read the analog value and store into the buffer
analogBufferIndex++;
if(analogBufferIndex == SCOUNT)
analogBufferIndex = 0;
}
static unsigned long printTimepoint = millis();
if(millis()-printTimepoint > 800U)
{
printTimepoint = millis();
for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF/ 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
float compensationCoefficient=1.0+0.02*(temperature-25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
float compensationVolatge=averageVoltage/compensationCoefficient; //temperature compensation
tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
//Serial.print("voltage:");
//Serial.print(averageVoltage,2);
//Serial.print("V ");

//TDS
Serial.print("tds=");
Serial.println(tdsValue,0);
// Serial.println("ppm");

//turbidity - lower is cloudier
float value = analogRead(A0);
Serial.print("turbidity=");
Serial.println(value * (5.0 / 1024.0));

//CO2
if(sensor.checkDataReady() == true){
    Serial.print("co2=");
    Serial.println(sensor.getCO2PPM());
    //ppm


    Serial.print("tvoc=");
    Serial.println(sensor.getTVOCPPB());
    //ppb
    sensor.writeBaseLine(0x847B);
}

}
}
int getMedianNum(int bArray[], int iFilterLen)
{
int bTab[iFilterLen];
for (byte i = 0; i<iFilterLen; i++)
bTab[i] = bArray[i];
int i, j, bTemp;
for (j = 0; j < iFilterLen - 1; j++)
{
for (i = 0; i < iFilterLen - j - 1; i++)
{
if (bTab[i] > bTab[i + 1])
{
bTemp = bTab[i];
bTab[i] = bTab[i + 1];
bTab[i + 1] = bTemp;
}
}
}
if ((iFilterLen & 1) > 0)
bTemp = bTab[(iFilterLen - 1) / 2];
else
bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
return bTemp;
}
