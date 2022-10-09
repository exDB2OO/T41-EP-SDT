#ifndef BEENHERE
#include "SDT.h"
#endif

/*****
  Purpose: Error message if Select button pressed with no Menu active
<<<<<<< Updated upstream

  Parameter list:
    void

=======

  Parameter list:
    void

>>>>>>> Stashed changes
  Return value;
    void
*****/
void NoActiveMenu()
{
    tft.setFontScale( (enum RA8875tsize) 1);    
    tft.setTextColor(RA8875_RED);
    tft.setCursor(10, 0);
    tft.print("No menu selected");  
}
/*****
  Purpose: Function is designed to route program control to the proper execution point in response to
           a button press.

  Parameter list:
    int vsl               the value from analogRead in loop()

  Return value;
    void
*****/
void ExecuteButtonPress(int val)
{
  if (val == MENU_OPTION_SELECT && menuStatus == NO_MENUS_ACTIVE) {          // Pressed Select with no primary/secondary menu selected
    NoActiveMenu();
    return;
  } else {
    menuStatus = PRIMARY_MENU_ACTIVE;
  }
  switch (val) {
    case MENU_OPTION_SELECT:                                     // 0
      /*                                    Useful comment in understanding how menues align:

          const char *topMenus[]    = { "CW",  "Spectrum Set", "AGC", "NR Set",
                                        "IQ Manual", "EQ Rec Set", "EQ Xmt Set", "Mic Comp", "Freq Cal",
                                        "NB Set", "RF Set", "Audio Post Proc", "VFO Select", "EEPROM"
                                      };
         int (*functionPtr[])()     = {&CWOptions, &SpectrumOptions, &AGCOptions, &NROptions,
                                         &IQOptions, &EqualizerRecOptions, &EqualizerXmtOptions, &MicOptions, &FrequencyOptions,
                                         &NBOptions, &RFOptions, &PostProcessorAudio, &VFOSelect, &EEPROMOptions
                                         };
      */

      if (secondaryMenuIndex == -1) {                             // Doing primary menu
        secondaryMenuChoiceMade = functionPtr[mainMenuIndex]();   // These are processed in MenuProcessing.cpp
        menuStatus = SECONDARY_MENU_ACTIVE;
        secondaryMenuIndex = -1;                                  // Reset secondary menu
      }
      EraseMenus();
      break;

    case MAIN_MENU_UP:                                            // 1
      ButtonMenuIncrease();
      if (secondaryMenuIndex == -1) {           // Doing primary menu
        ShowMenu(&topMenus[mainMenuIndex], PRIMARY_MENU);
      }
      break;

    case BAND_UP:
      ShowSpectrum();                                           //Now calls ProcessIQData and Encoders calls
      digitalWrite(bandswitchPins[currentBand], LOW);
      ButtonBandIncrease();
      digitalWrite(bandswitchPins[currentBand], HIGH);
      BandInformation();
      NCO_FREQ = 0L;
      CenterFilterOverlay();
      break;

    case ZOOM:                                                    // 3
      ButtonZoom();
      break;

    case MAIN_MENU_DN:                                            // 4
      ButtonMenuDecrease();
      if (secondaryMenuIndex == -1) {           // Doing primary menu
        ShowMenu(&topMenus[mainMenuIndex], PRIMARY_MENU);
      }
      break;

    case BAND_DN:                                                 // 5
      //filterWidth = FILTER_WIDTH;
      ShowSpectrum();                                           //Now calls ProcessIQData and Encoders calls
      digitalWrite(bandswitchPins[currentBand], LOW);
      ButtonBandDecrease();
      digitalWrite(bandswitchPins[currentBand], HIGH);
      BandInformation();
      NCO_FREQ = 0L;
      CenterFilterOverlay();
      break;

    case FILTER:                                                  // 6
      ButtonFilter();
      break;

    case DEMODULATION:                                            // 7
      ButtonDemodMode();
      break;

    case SET_MODE:                                                //  8
      ButtonMode();
      break;

    case NOISE_REDUCTION:                                         // 9
      ButtonNR();
      break;

    case NOTCH_FILTER:                                            // 10
      ButtonNotchFilter();
      UpdateNotchField();
      break;
    /*
        case DISPLAY_OPTIONS:                                         //  11
          ButtonDisplayOptions();
          break;
    */
    case INCREMENT:                                               // 12
      ButtonFreqIncrement();
      break;

    case NOISE_FLOOR:                                             // 13
      ButtonSetNoiseFloor();
      break;

    case UNUSED_1:                                                // 14
      if (stepFT == 50) {                                                       // AFP 06-22-22 Fine tune step toggle
        stepFT = 500;
      }
      else {
        if (stepFT == 500) {
          stepFT = 50;
        }
      }
      UpdateIncrementField();
      break;

    case UNUSED_2:
      ResetHistograms();     // JJP 1/28/22
      break;

    default:
      break;
  }
}

/*****
  Purpose: To process a band decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonDecrease()
{
  if (secondaryMenuIndex == -1) {                   // We're working on main menu
    mainMenuIndex--;
    if (mainMenuIndex < 0) {                        // At last menu option, so...
      mainMenuIndex = TOP_MENU_COUNT - 1;           // ...wrap around to first menu option
    }
  } else {
    secondaryMenuIndex--;
    if (secondaryMenuIndex < 0) {                   // Same here...
      secondaryMenuIndex = subMenuMaxOptions - 1;
    }
  }
}

/*****
  Purpose: To process a band decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonIncrease()
{
  if (secondaryMenuIndex == -1) {                   // We're working on main menu
    mainMenuIndex++;
    if (mainMenuIndex == TOP_MENU_COUNT) {          // At last menu option, so...
      mainMenuIndex = 0;                            // ...wrap around to first menu option
    }
  } else {
    secondaryMenuIndex++;
    if (secondaryMenuIndex == subMenuMaxOptions) {  // Same here...
      secondaryMenuIndex = 0;
    }
  }
}

/*****
  Purpose: To process a band decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void BandDecrease()
{
  currentBand--;
  if (currentBand < FIRST_BAND) {
    currentBand = LAST_BAND; // cycle thru radio bands
  }
  if (currentBand > LAST_BAND)  {
    currentBand = FIRST_BAND;
  }
  AudioNoInterrupts();
  freq_flag[1] = 0;
  SetBand();
  ControlFilterF();
  FilterBandwidth();
  DrawSMeterContainer();
  SetFreq();
  AGCPrep();
  AudioInterrupts();
}

/*****
  Purpose: To process a band decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void BandIncrease()
{
  AudioNoInterrupts();
  currentBand++;
  if (currentBand > LAST_BAND) {
    currentBand = FIRST_BAND; // cycle thru radio bands
  }
  freq_flag[1] = 0;
  SetBand();
  UpdateIncrementField();
  FilterBandwidth();
  DrawSMeterContainer();
  AGCPrep();
  SetFreq();
  MyDelay(1L);
  AudioInterrupts();
}

/*****
  Purpose: To process a band decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonFreqIncrement()
{
  tuneIndex--;
  if (tuneIndex < 0)
    tuneIndex = 5;
  freqIncrement = incrementValues[tuneIndex];
  UpdateIncrementField();
}

/*****
  Purpose: Check for UI button press. If pressed, return the ADC value

  Parameter list:
    int vsl               the value from analogRead in loop()\

  Return value;
    int                   -1 if not valid push button, ADC value if valid
*****/
//DB2OO, 12.8.22: taken over from message 18096 but limited number of loops!
#if 0
int ReadSelectedPushButton()
{
<<<<<<< Updated upstream

  minPinRead = analogRead(BUSY_ANALOG_PIN);
 
  if (minPinRead > NOTHING_TO_SEE_HERE) {          // Value too high; not valid choice
    return -1;
  }
  if (menuStatus == NO_MENUS_ACTIVE) {

  }
=======
  minPinRead           = 0;
  int buttonReadOld = 1024;
  int maxLoops = 20;
                                                                  // do exponential averaging to smooth out the button response
  //DB2OO, 12.8.22: limit the max. number of loops
  while ((abs(minPinRead - buttonReadOld) > 3) && maxLoops-- > 0 ) {
    minPinRead      = analogRead(BUSY_ANALOG_PIN);
    buttonRead       = .1 * minPinRead + (1 - .1) * buttonReadOld;      // See expected values in next function.
    buttonReadOld = buttonRead;
    //DB2OO, 12.8.22 added 10ms delay
    MyDelay(10);
  }
  if (buttonRead > NOTHING_TO_SEE_HERE) {
    return -1;
  }
  mainMenuIndex = minPinRead = buttonRead;
  MyDelay(100L);
>>>>>>> Stashed changes
  return minPinRead;
}
#endif
#if 1
int ReadSelectedPushButton()
{

  minPinRead = analogRead(BUSY_ANALOG_PIN);
 
  if (minPinRead > NOTHING_TO_SEE_HERE) {          // Value too high; not valid choice
    return -1;
  }
  if (menuStatus == NO_MENUS_ACTIVE) {

  }
  return minPinRead;
}
#endif

/*****
  Purpose: Determine which UI button was pressed

  Parameter list:
    int valPin            the ADC value from analogRead()

  Return value;
    int                   -1 if not valid push button, index of push button if valid
*****/
int ProcessButtonPress(int valPin)
{
  int switchIndex;

  if (valPin == BOGUS_PIN_READ) {                  // Not valid press
    return -1;
  }

  if (valPin == MENU_OPTION_SELECT && menuStatus == NO_MENUS_ACTIVE) {
    NoActiveMenu();
    return -1;
  }
  for (switchIndex = 0; switchIndex < TOP_MENU_COUNT; switchIndex++)
  {
    if (abs(valPin - EEPROMData.switchValues[switchIndex]) < WIGGLE_ROOM)    // ...because ADC does return exact values every time
    {
      MyDelay(100L);
      return switchIndex;
    }
  }
  return -1;                                              // Really should never do this
}
