// author: Michael Hautman
// for Arduino or Arduino library compatible devices  
#include "Arduino.h"
#include "Wire.h"
#include "LightHandlerSimple.h"

//scrubs int math for storage in byte
void LightHandlerSimple::Brightnesslimit(int &x) {
	if(x < 0){
		x = 0;
	} else if(x > 255){
		x = 255;
	}
}

void LightHandlerSimple::lightWaveSin() {
	int brightness;
	float phase;
	for (int cnt = 0; cnt < NumLED; cnt++) {
		if(sineMode){
			phase = 0 + (1.0 / cnt) * ((float)cnt)/Wavelength;
		} else {
			phase = 0 + phaseSeparation * ((float)cnt)*(abs(Wavelength)/Wavelength);//wavelength is used in this case simply to preserve the direction of the wave 
		}
		brightness = range * sin(PI*(phase) + PI* millis() / (1000)*speed) + midInt;
		Brightnesslimit(brightness);
		Brightness[cnt] = (byte)brightness;
	}
}

//random flashes
//follows a (-(x-1)^2 + 1) curve for growing brighter then fading.
//reaches peak brightness at blipDuration/2
void LightHandlerSimple::blip() {
	int brightness;
	unsigned long timeSinceBlipTrue;
	unsigned long currentTime = millis();
	int variance = abs(range + midInt);
	for (int cnt = 0; cnt < NumLED; cnt++) {
		bool blip = currentTime > (blipRand[cnt] + lastBlip[cnt]);
		if(blip){
			timeSinceBlipTrue = currentTime - (lastBlip[cnt] + blipRand[cnt]);
			if(timeSinceBlipTrue > blipDuration){
				Brightness[cnt] = (byte)midInt;
				lastBlip[cnt] = currentTime;
				blipRand[cnt] = random(0, TTblip);
			}else{
				brightness = variance*(-pow(((float)timeSinceBlipTrue/(blipDuration/2) - 1), 2) + 1) + midInt; //follows a (-(x-1)^2 + 1) curve for growing brighter then fading
				Brightnesslimit(brightness);
				Brightness[cnt] = (byte)brightness;	
			}
		}
	}
}

void LightHandlerSimple::setTimeToBlip(int x){
	
}

void LightHandlerSimple::lightIntensityMod(int x) {
	int temp = midInt + x;
	Brightnesslimit(temp);
	midInt = temp;
}

void LightHandlerSimple::setSineMode(bool x){
	sineMode = x;
}

void LightHandlerSimple::setPhaseSeparation(float x){
	phaseSeparation = x;
}
	
void LightHandlerSimple::setSineSpeed(float x){
	speed = x; 
}

void LightHandlerSimple::setMidIntensity(int x) {
	Brightnesslimit(x);
	midInt = (byte)x;
}

void LightHandlerSimple::setRange(int x) {
	Brightnesslimit(x);
	range = (byte)x;
}

void LightHandlerSimple::lightingSettingsPresets(int set) {
	switch (set) {
	case DEFAULT_SET: //default
		reset();
		AnimationMode = 0;
		break;
	case SIN_WAVE_FORWARD: //sin wave: move up
		sineMode = true;
		Wavelength = 1.0;
		AnimationMode = 1;
		break;
	case SIN_WAVE_REV: //sin wave: move down
		sineMode = true;
		Wavelength = -1.0;
		AnimationMode = 1;
		break;
	case SIN_PULSE_FORWARD: //sin wave: short pulse up
		sineMode = false;
		midInt = 0;
		AnimationMode = 1;
		break;
	case SIN_PULSE_REVERSE: //sin wave: short pulse up
		sineMode = false;
		midInt = 0;
		AnimationMode = 1;
		break;
	case BLIP: //blip
		AnimationMode = 2;
		break;
	case ZERO: //set to 0
		speed = 0;
		range = 0;
		midInt = 0;
		AnimationMode = 0;
		break;
	default:
		//nothing
		break;
	}
}

void LightHandlerSimple::applyState() {
	for (int cnt = 0; cnt < NumLED; cnt++) {
		analogWrite(LEDPin[cnt], Brightness[cnt]);
	}
}

void LightHandlerSimple::lightingFunc(int set) {
	switch (set) {
	case CONST_INTENSITY: //constant 
		for (int cnt = 0; cnt < NumLED; cnt++) {
			Brightness[cnt] = midInt;
		}

		break;
	case SINUSOID:
		lightWaveSin();

		break;
	case RAND_BLIP:
		blip();

		break;
	}
}

void LightHandlerSimple::getGlobalLightSetting(){
	return GlobalLightSetting;
}

void LightHandlerSimple::getAnimationMode(){
	return AnimationMode;
}

void LightHandlerSimple::setLightPreset(int x){
	GlobalLightSetting = x;
	lightingSettingsPresets(x);
	}
	
void LightHandlerSimple::setWavelength(float x){
	Wavelength = x;
}
	
void LightHandlerSimple::reset(){
		speed = 1;
		range = 35;
		midInt = 45;
		AnimationMode = 0;
}

	//constructor
LightHandlerSimple::LightHandlerSimple(const int *LEDPinArray,const int &NumLEDin) {
		unsigned long currenttime = millis();
		LEDPin = LEDPinArray;
		NumLED = NumLEDin;
		lastBlip = new unsigned long[NumLEDin];
		blipRand = new unsigned int[NumLEDin];
		Brightness = new byte[NumLEDin];
		for (int cnt = 0; cnt < NumLED; cnt++) {
			pinMode(LEDPin[cnt], OUTPUT);
			Brightness[cnt] = 0;
			lastBlip[cnt] = currenttime;
			blipRand[cnt] = random(0, TTblip);
		}
	}

LightHandlerSimple::LightHandlerSimple(const int &NumLEDin) {
		unsigned long currenttime = millis();
		NumLED = NumLEDin;
		lastBlip = new unsigned long[NumLEDin];
		blipRand = new unsigned int[NumLEDin];
		Brightness = new byte[NumLEDin];
		for (int cnt = 0; cnt < NumLED; cnt++) {
			pinMode(LEDPin[cnt], OUTPUT);
			Brightness[cnt] = 0;
			lastBlip[cnt] = currenttime;
			blipRand[cnt] = random(0, TTblip);
		}
	}

	//default constructor
LightHandlerSimple::LightHandlerSimple() {}

void LightHandlerSimple::execute() {
		lightingFunc(AnimationMode);
		applyState();
	}
