/**********************************************************************************************
 * This is another PID test implementation based on the great work of
 *
 * * Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 * * Arduino PID Library - Version 1.2.1
 * * This Library is licensed under the MIT License
 *
 * all credits go to him. (Tobias <medlor@web.de>)
 **********************************************************************************************/

#ifndef PIDBias_h
#define PIDBias_h
#define LIBRARY_VERSION 0.0.1

//#include "rancilio-pid.h"
#include <stdio.h>

class PIDBias {
  public:
    // Constants used in some of the functions below
    #define AUTOMATIC 1
    #define MANUAL 0

    // commonly used functions **************************************************************************
    PIDBias(float*, double*, float*, float*, unsigned long*, unsigned int*, float**, float, float, float, const unsigned int);

    void SetMode(int Mode); // * sets PIDBias to either Manual (0) or Auto (non-0)

    int Compute(float, float); // * performs the PIDBias calculation.  it should be
                  //   called every time loop() cycles. ON/OFF and
                  //   calculation frequency can be set using SetMode
                  //   SetSampleTime respectively
                  // returns:
                  // 1 := when compute is run
                  // 0 := compute not run yet
                  // 2 := time for compute to run, but pid is disabled

    void SetOutputLimits(float, float); // * clamps the output to a specific range. 0-255 by default, but
                                          //   it's likely the user will want to change this depending on
                                          //   the application

    // available but not commonly used functions ********************************************************
    void SetTunings(float, float, // * While most users will set the tunings once in the
        float); //   constructor, this function gives the user the option
                //   of changing tunings during runtime for Adaptive control

    void SetSampleTime(int); // * sets the frequency, in Milliseconds, with which
                            //   the PIDBias calculation is performed.  default is 100

    // Display functions ****************************************************************
    float GetKp(); // These functions query the pid for interal values.
    float GetKi(); //  they were created mainly for the pid front-end,
    float GetKd(); // where it's important to know what is actually
    int GetMode(); //  inside the PIDBias.

    float GetOutputP();
    float GetOutputI();
    float GetSumOutputI();
    float GetOutputD();
    double GetLastOutput();
    void SetSumOutputI(float);
    void SetFilterSumOutputI(float);
    float GetFilterSumOutputI();
    // void SetSteadyPowerOffset(float);
    // float GetSteadyPowerOffset();
    void SetAutoTune(bool);
    void SetSteadyPowerDefault(float);
    float GetSteadyPowerOffset();
    // void UpdateSteadyPowerOffset(unsigned long, unsigned long);
    float CalculateSteadyPowerOffset();
    float GetSteadyPowerOffsetCalculated();

  private:
    void Initialize();

    float dispKp; // * we'll hold on to the tuning parameters in user-entered
    float dispKi; //   format for display purposes
    float dispKd; //

    float kp; // * (P)roportional Tuning Parameter
    float ki; // * (I)ntegral Tuning Parameter
    float kd; // * (D)erivative Tuning Parameter

    float outputP;
    float outputI;
    float outputD;
    float sumOutputD;
    float sumOutputI;
    float filterSumOutputI;
    float steadyPowerDefault;
    float* mySteadyPowerOffset;
    unsigned long* mySteadyPowerOffset_Activated;
    unsigned int* mySteadyPowerOffset_Time;
    float steadyPowerOffsetCalculated;
    bool steadyPowerAutoTune;

    float* myInput; // * Pointers to the Input, Output, and Setpoint variables
    double* myOutput; //   This creates a hard link between the variables and the
    float** mySetpoint; //   PIDBias, freeing the user from having to constantly tell us
                        //   what these values are.  with pointers we'll just know.
    float* mySteadyPower;
    unsigned int myWindowSize;
    unsigned long lastTime;
    unsigned long lastTrigger, lastTrigger2, lastTriggerCrossingSetPoint;
    float lastError;
    double lastOutput;
    unsigned long SampleTime;
    float outMin, outMax;
    bool inAuto;
};
#endif
