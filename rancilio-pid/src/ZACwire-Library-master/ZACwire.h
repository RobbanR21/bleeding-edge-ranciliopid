/*  ZACwire - Library for reading temperature sensors TSIC 206/306/506
  created by Adrian Immer in 2020
  v1.3.1
*/

#ifndef ZACwire_h
#define ZACwire_h

#include "Arduino.h"

template <uint8_t pin>
class ZACwire {
  
  public:
  
	ZACwire(int Sensortype = 306, byte defaultBitWindow = 125, bool core = 1){
      _Sensortype = Sensortype;
      _defaultBitWindow = defaultBitWindow + (range >> 1);	//expected BitWindow in µs, depends on sensor & temperature
      _core = core;									//only ESP32: choose cpu0 or cpu1
    }
  
	bool begin() {									//start collecting data, needs to be called 100+ms before the first getTemp()
		pinMode(pin, INPUT);
		if (!pulseInLong(pin, LOW)) return false;	//check if there is an incoming signal
		isrPin = digitalPinToInterrupt(pin);
		if (isrPin == -1) return false;
		bitWindow = _defaultBitWindow;				//change from 0 to defaultBitWindow to give the getTemp the info begin() was already executed
		#ifdef ESP32
		xTaskCreatePinnedToCore(attachISR_ESP32,"attachISR_ESP32",800,NULL,1,NULL,_core); //freeRTOS
		#else
		attachInterrupt(isrPin, read, RISING);
		#endif
		return true;
    }
  
	float getTemp() {								//give back temperature in °C
		static bool misreading;
		uint16_t temp[2];
		if ((unsigned int)millis() - lastISR > 255) {	//check wire connection for the last 255ms
			if (bitWindow) return 221;				// temp=221 if sensor not connected
			else {									// w/o bitWindow, begin() wasn't called before
				begin();
				delay(110);
			}
		}
		if (BitCounter == 19) {
			byte newBitWindow = ((ByteTime << 1) + ByteTime >> 5) + (range >> 1);  //divide by around 10.5 and add half range
			bitWindow < newBitWindow ? ++bitWindow : --bitWindow;	//adjust bitWindow
		}
		else misreading = true;						//use misreading-backup when newer reading is incomplete
		bool _backUP = backUP^misreading;
		temp[0] = rawTemp[0][_backUP];				//get high significant bits from ISR
		temp[1] = rawTemp[1][_backUP];				//get low   ''    ''
		misreading = !misreading;					//reset misreading after use
		
		bool parity = true;
		for (byte j=0; j<2; ++j) {
			for (byte i=0; i<9; ++i) parity ^= temp[j] & 1 << i;	//check parity
			temp[j] >>= 1;							//delete parity bit
		}
		if (parity) { 								//check for failure
			temp[1] |= temp[0] << 8;				//join high and low significant figures
			misreading = false;
			if (_Sensortype < 400) return float((temp[1] * 250L >> 8) - 499) / 10;  //calculate °C
			else return float((temp[1] * 175L >> 9) - 99) / 10;
		}
		else if (misreading) return getTemp();		//restart with backUP raw temperature
		return 222;									//temp=222 if reading failed
	}

  
	void end() {
		detachInterrupt(isrPin);
	}

  private:  
  
	#ifdef ESP32
	static void attachISR_ESP32(void *arg){			//attach ISR in freeRTOS because arduino can't attachInterrupt() inside of template class
		gpio_pad_select_gpio((gpio_num_t)isrPin);
		gpio_set_direction((gpio_num_t)isrPin, GPIO_MODE_INPUT);
		gpio_set_intr_type((gpio_num_t)isrPin, GPIO_INTR_POSEDGE);
		gpio_install_isr_service(0);
		gpio_isr_handler_add((gpio_num_t)isrPin, read, NULL);
		vTaskDelete(NULL);
	}
	static void IRAM_ATTR read(void *arg) {
	#elif defined(ESP8266)
	static void ICACHE_RAM_ATTR read() {
	#else
	static void read() {							//get called with every rising edge
	#endif
		if (++BitCounter > 4) {						//first 4 bits always =0
			static bool ByteNr;
			unsigned int microtime = micros();
			static unsigned int deltaTime;
			deltaTime = microtime - deltaTime;		//measure time to previous rising edge
			if (deltaTime >> 10) {					//true at start bit
				ByteTime = microtime;				//for measuring Tstrobe/bitWindow
				backUP = !backUP;
				BitCounter = 0;
				lastISR = millis();					//for checking wire connection
			}
			else if (BitCounter == 6) rawTemp[ByteNr][backUP] = ByteNr = 0;
			else if (BitCounter == 10) {			//after stop bit
				ByteTime = microtime - ByteTime;
				rawTemp[ByteNr = 1][backUP] = 0;
			}
			rawTemp[ByteNr][backUP] <<= 1;      
			if (deltaTime > bitWindow);				//Logic 0
			else if (rawTemp[ByteNr][backUP] & 2 || deltaTime < bitWindow - (range >> (BitCounter==11))) rawTemp[ByteNr][backUP] |= 1;  //Logic 1
			deltaTime = microtime;
		}
	}
  
    static int isrPin;
    int _Sensortype;								//either 206, 306 or 506
    byte _defaultBitWindow;							//expected BitWindow in µs, according to datasheet 125
    bool _core;
    static volatile byte BitCounter;
    static volatile unsigned int ByteTime;
    static volatile uint16_t rawTemp[2][2];
    static byte bitWindow;
    static const byte range = 62;
    static volatile bool backUP;
    static volatile unsigned int lastISR;
};

template<uint8_t pin>
volatile byte ZACwire<pin>::BitCounter = 20;
template<uint8_t pin>
volatile unsigned int ZACwire<pin>::ByteTime;
template<uint8_t pin>
volatile bool ZACwire<pin>::backUP;
template<uint8_t pin>
volatile uint16_t ZACwire<pin>::rawTemp[2][2];
template<uint8_t pin>
int ZACwire<pin>::isrPin;
template<uint8_t pin>
byte ZACwire<pin>::bitWindow;
template<uint8_t pin>
volatile unsigned int ZACwire<pin>::lastISR;

#endif
