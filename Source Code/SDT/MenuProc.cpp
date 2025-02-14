#ifndef BEENHERE
#include "SDT.h"
#endif

/*****
  Purpose: Present the CW options available and return the selection

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int CWOptions()
{
  const char *cwChoices[]   = {"WPM", "Key Type", "Paddle Flip", "Cancel"};   // Add code practice oscillator
  int CWChoice        = 0;

  CWChoice = SubmenuSelect(cwChoices, 4, 0);

  switch (CWChoice) {
    case 0:                                 // WPM
      EEPROMData.wordsPerMinute = SetWPM();
      break;

    case 1:                                 // Type of key:
      SetKeyType();                         // Straight key or keyer?
      UpdateWPMField();
      break;

    case 2:                                 // Flip paddles
      DoPaddleFlip();
      break;

    default:                                // Cancel
      break;
  }
  return CWChoice;
}

/*****
  Purpose: Show the list of scales for the spectrum divisions

  Parameter list:
    void

  Return value
    int           an index into displayScale[] array, or -1 on cancel
*****/
int SpectrumOptions()
{
  const char *spectrumChoices[] = {"20 dB/unit", "10 dB/unit", "5 dB/unit", "2 dB/unit", "1 dB/unit", "Cancel"};
  int spectrumSet = 1;

  spectrumSet = SubmenuSelect(spectrumChoices, 6, spectrumSet);
  if (spectrumSet == -1)                                        // Did they make a choice?
    return currentScale;                                        // Nope.

  currentScale = spectrumSet;                                   // Yep...
  EEPROMData.currentScale = currentScale;
  EEPROMWrite();
  ShowSpectrumdBScale();
  return spectrumSet;
}
/*****
  Purpose: Present the bands available and return the selection

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int AGCOptions()
{
  const char *AGCChoices[]  = {"Off", "Slow", "Medium", "Fast", "Cancel"};

  AGCMode = SubmenuSelect(AGCChoices, 5, AGCMode);

  switch (AGCMode) {                                          // The opted for AGC
    case 0:                                                   // Off
      tft.fillRect(AGC_X_OFFSET, AGC_Y_OFFSET, 30, tft.getFontHeight(), RA8875_BLACK);
      AGCPrep();
      return AGCMode;
      
    case 1:                                                   // Slow
      tft.setTextColor(RA8875_WHITE);
      break;
      
    case 2:                                                   // Medium
      tft.setTextColor(ORANGE);
      break;
      
    case 3:                                                   // Fast
      tft.setTextColor(RA8875_GREEN);
      break;
      
    default:
      break;
  }
  tft.setCursor(AGC_X_OFFSET, AGC_Y_OFFSET);    // Where to print
  tft.print("AGC");

  AGCPrep();
  return AGCMode;
}

/*****
  Purpose: Present the noise reduction options

  Parameter list:
    void

  Return value
    int           an index into the band array, 0 for off, -1 for cancel
*****/
int SetFilterValue()
{
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, 250, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X, 1);
  tft.print(" Value, ");
  
  switch (nrOptionSelect) {
    case 0:                                 // Off
      return 0;
      break;

    case 1:                                 // Kim
      tft.print("Kim: ");
      break;

    case 2:                                 // LMS 1
      tft.print("LMS 1: ");
      break;

    case 3:                                 // LMS 2
      tft.print("LMS 2: ");
      break;

    case 4:                                 // Cancel
      NR_Filter_Value = -1;
      break;

  }
  return GetEncoderValue(5, 100, 50, 5, (char *) " Value:  "); 
}
/*****
  Purpose: Present the noise reduction options

  Parameter list:
    void

  Return value
    int           an index into the band array, 0 for off, -1 for cancel
*****/
int NROptions()
{
  const char *NRChoices[]  = {"Off", "Kim", "Spectral", "LMS", "Cancel"};
  int kimValue, lms1, lms2;
  int passBack = 0;

  lms1 = lms2 = 0;
  nrOptionSelect = SubmenuSelect(NRChoices, 5, nrOptionSelect);

  switch (nrOptionSelect) {
    case 0:                                 // Off
      break;

    case 1:                                 // Kim
      SetFilterValue();
      kimValue = NR_Filter_Value;
      NR_Choice = NR_Kim = 1;
      passBack = kimValue;
      break;

    case 2:                                 // Spectral
      SetFilterValue();
      NR_Choice = NR_LMS = 2;
      passBack = lms1;
      break;

    case 3:                                 // LMS
      SetFilterValue();
      NR_Choice = NR_LMS = 3;
      passBack = lms2;
      break;

    case 4:                                 // Cancel
      NR_Choice = passBack = -1;
      break;
  }
//  tft.fillRect(0, 0, 580, CHAR_HEIGHT, RA8875_BLACK);         // Erase menu choices
  EraseMenus();
  UpdateNoiseField();
  return passBack;
}

/*****
  Purpose: set the calibration factors for receive and transmit IQ,

  Parameter list:
    void

  Return value
    int               1 done for consistency of definition

  CAUTION:  Sub menu selection for Rec IQ and transmit IQ
    Each set of adjustment first does IG gain factor and then IQ phase factor
    To use:
    1. Press Menu+ to get to "IQ Manual" Press "select" to get to submenu Press "Select" to enter Receive cal
    2. Adjust Gain factor with Filter Encoder, then press "Select" to move to Phase factor.Adjust Phase
    3. Press Select again to Exit.
    4. To calibrate Transmit IQ cal,press Menu+ get to "IQ Manual" , press ",Select", then "Menu+" to get to Transmit
    5. Use "Select" Button againto proceed as above
*****/
int IQOptions()
{
  static long RecIQGainRead    = 0, last_RecIQGainRead   = 0;
  static long RecIQPhaseRead   = 0, last_RecIQPhaseRead  = 0;
  static long XmitIQGainRead   = 0, last_XmitIQGainRead  = 0;
  static long XmitIQPhaseRead  = 0, last_XmitIQPhaseRead = 0;
  const char *IQOptions[] = {"Receive", "Transmit", "Cancel"};
  int val;

  spectrum_zoom = 0;

  ZoomFFTPrep();
  UpdateZoomField();
  DrawFrequencyBarValue();
  ShowBandwidth();

  IQChoice = SubmenuSelect(IQOptions, 3, IQChoice);
  switch (IQChoice) {
    case 0:
      IQ_RecCalFlag = 1;
      SetFreq();

      //== == == == == == == == == == == == == == == == == = Receive Gain == == == == == == == == ==
      tft.fillRect(251, 0, 250, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setFontScale(1);
      tft.setTextColor(RA8875_WHITE);
      tft.setCursor(252, 1);
      tft.print("R Gain:");
      tft.setCursor(400, 1);
      tft.print(IQ_amplitude_correction_factor);
      while (true) {
        if (RecIQGainRead != last_RecIQGainRead) {
          IQ_amplitude_correction_factor = IQ_amplitude_correction_factor - RecIQGainRead * 0.001;
          tft.fillRect(350, 0, 150, CHAR_HEIGHT, RA8875_MAGENTA);
          tft.setTextColor(RA8875_WHITE);
          tft.setCursor(400, 1);
          tft.print(IQ_amplitude_correction_factor);
        }
        last_RecIQGainRead = RecIQGainRead;

        val = ReadSelectedPushButton();
        if (val != BOGUS_PIN_READ) {                        // Any button press??
          val = ProcessButtonPress(val);                    // Use ladder value to get menu choice
          if (val == MENU_OPTION_SELECT) {                  // Yep. Make a choice??
            EEPROMWrite();
            break;
          }
        }
        IQCalFlag = 1;
        ShowSpectrum();
      }
      //== == == == == == == == == == == == == == == == == = Receive Phase == == == == == == == == ==
      tft.fillRect(251, 0, 250, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setTextColor(RA8875_WHITE);
      tft.setCursor(252, 1);
      tft.print("R Phase:");
      tft.setCursor(400, 1);
      tft.print(IQ_phase_correction_factor);
      while (true) {
        if (RecIQPhaseRead != last_RecIQPhaseRead) {
          IQ_phase_correction_factor = IQ_phase_correction_factor - RecIQPhaseRead * 0.001;
          tft.fillRect(350, 0, 150, CHAR_HEIGHT, RA8875_MAGENTA);
          tft.setTextColor(RA8875_WHITE);
          tft.setCursor(400, 1);
          tft.print(IQ_phase_correction_factor);
        }
        last_RecIQPhaseRead = RecIQPhaseRead;

        val = ReadSelectedPushButton();
        if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
          val = ProcessButtonPress(val);                    // Use ladder value to get menu choice
          if (val == 0) {                  // Make a choice??
            IQCalFlag = 0;
            IQ_RecCalFlag = 0; //  AFP 04-17-22
            SetFreq();         //  AFP 04-17-22
            EEPROMWrite();
            ShowSpectrum();
            break;
          }
        }
        IQCalFlag = 1;
        ShowSpectrum();
        IQ_RecCalFlag = 0;
        SetFreq();
      }
    case 1:                                 // Transmit
      //== == == == == == == == == == == == == == == == == = Xmit Gain == == == == == == == == ==
      tft.fillRect(251, 0, 250, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setTextColor(RA8875_WHITE);
      tft.setCursor(252, 1);
      tft.print("X Gain:");
      tft.setCursor(400, 1);
      tft.print(IQ_Xamplitude_correction_factor);

      while (true) {
        if (digitalRead(PTT) == LOW) {
          mute = 1;
          digitalWrite(RXTX, HIGH);
          si5351.output_enable(SI5351_CLK2, 0);  //AFP 12-27-21
          modeSelectInR.gain(0, 1.0); //Selects Ex
          modeSelectInR.gain(1, 0.0); //Selects Ex
          modeSelectInL.gain(0, 1.0); //Selects Ex
          modeSelectInL.gain(1, 0.0); //Selects Ex

          modeSelectOutL.gain(0, 0);
          modeSelectOutR.gain(0, 0);
          modeSelectOutExL.gain(0, 1.0);
          modeSelectOutExR.gain(0, 1.0);
          ExciterIQData();
        }
        if (XmitIQGainRead != last_XmitIQGainRead) {
          IQ_Xamplitude_correction_factor = IQ_Xamplitude_correction_factor - XmitIQGainRead * 0.01;
          tft.fillRect(350, 0, 150, CHAR_HEIGHT, RA8875_MAGENTA);
          tft.setTextColor(RA8875_WHITE);
          tft.setCursor(400, 1);
          tft.print(IQ_Xamplitude_correction_factor);
        }
        last_XmitIQGainRead = XmitIQGainRead;

        val = ReadSelectedPushButton();
        if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
          val = ProcessButtonPress(val);                    // Use ladder value to get menu choice
          if (val == 0) {                  // Make a choice??
            EEPROMWrite();
            break;
          }
        }
      }
      //== == == == == == == == == == == == == == == == == = Xmit Phase == == == == == == == == ==

      tft.fillRect(251, 0, 250, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setTextColor(RA8875_WHITE);
      tft.setCursor(252, 1);
      tft.print("X Phase:");
      tft.setCursor(400, 1);
      tft.print(IQ_Xphase_correction_factor);
      while (true) {
        if (digitalRead(PTT) == LOW) {
          mute = 1;
          digitalWrite(RXTX, HIGH);
          si5351.output_enable(SI5351_CLK2, 0);
          modeSelectInR.gain(0, 1.0); //Selects Ex
          modeSelectInR.gain(1, 0.0); //Selects Ex
          modeSelectInL.gain(0, 1.0); //Selects Ex
          modeSelectInL.gain(1, 0.0); //Selects Ex

          modeSelectOutL.gain(0, 0);
          modeSelectOutR.gain(0, 0);
          modeSelectOutExL.gain(0, 1.0);
          modeSelectOutExR.gain(0, 1.0);
          ExciterIQData();
        }
        if (XmitIQPhaseRead != last_XmitIQPhaseRead) {
          IQ_Xphase_correction_factor = IQ_Xphase_correction_factor - XmitIQPhaseRead * 0.01;

          tft.fillRect(350, 0, 150, CHAR_HEIGHT, RA8875_MAGENTA);
          tft.setTextColor(RA8875_WHITE);
          tft.setCursor(400, 1);
          tft.print(IQ_Xphase_correction_factor);
        }
         last_XmitIQPhaseRead = XmitIQPhaseRead;

        val = ReadSelectedPushButton();
        if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
          val = ProcessButtonPress(val);                    // Use ladder value to get menu choice
          if (val == 0) {                  // Make a choice??
            IQCalFlag = 0;
            EEPROMWrite();
            break;
          }
        }
        IQCalFlag = 1;
      }
    case 2:                                 //Cancel
      break;
  }
  IQ_RecCalFlag = 0;
  return 1;                                 // To keep function pointer happy
}
/*****
  Purpose: To process the graphics for the 14 chan equalizar otpion

  Parameter list:
    int array[]         the peoper array to fill in
    char *title             the equalizer being set
  Return value
    void
*****/
void ProcessEqualizerChoices(int array[], char *title)
{
  const char *eqFreq[] = {" 200", " 250", " 315", " 400", " 500", " 630", " 800",
                          "1000", "1250", "1600", "2000", "2500", "3150", "4000"
                         };
  int yLevel[EQUALIZER_CELL_COUNT];

  int columnIndex;
  int iFreq;
  int newValue;
  int xOrigin  = 50;
  int yOrigin  = 50;
  int wide     = 700;
  int high     = 300;
  int barWidth = 46;
  int barTopY;
  int barBottomY;
  int val;

  for (iFreq = 0; iFreq < EQUALIZER_CELL_COUNT; iFreq++) {
    yLevel[iFreq] = array[iFreq];                           // Could be Rec or Xmt.
  }
  tft.writeTo(L2);
  tft.clearMemory();
  tft.writeTo(L1);
  tft.fillWindow(RA8875_BLACK);

  tft.fillRect(xOrigin - 50, yOrigin - 25, wide + 50, high + 50, RA8875_BLACK); // Clear data area
  tft.setTextColor(RA8875_GREEN);
  tft.setFontScale( (enum RA8875tsize) 1);
  tft.setCursor(200, 0);
  tft.print(title);

  tft.drawRect(xOrigin - 4, yOrigin, wide + 4, high, RA8875_BLUE);
  tft.drawFastHLine(xOrigin - 4, yOrigin + (high / 2), wide + 4, RA8875_RED); // Print center zero line center
  tft.setFontScale( (enum RA8875tsize) 0);

  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + tft.getFontHeight());
  tft.print("+12");
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + (high / 2) - tft.getFontHeight());
  tft.print(" 0");
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + high - tft.getFontHeight() * 2);
  tft.print("-12");
  tft.drawFastHLine(xOrigin + tft.getFontWidth() * 2, yOrigin + high - tft.getFontHeight() * 2 - 10, wide - tft.getFontWidth() * 2, RA8875_WHITE);    // Clear hole in display center

  barTopY   = yOrigin + (high / 2);                   // 50 + (300 / 2) = 200
  barBottomY = barTopY + DEFAULT_EQUALIZER_BAR;       // Default 200 + 100

  for (iFreq = 0; iFreq < EQUALIZER_CELL_COUNT; iFreq++) {
    tft.fillRect(xOrigin  + (barWidth + 4) * iFreq , barTopY - (yLevel[iFreq] - DEFAULT_EQUALIZER_BAR), barWidth, yLevel[iFreq], RA8875_CYAN);
    tft.setCursor(xOrigin + (barWidth + 4) * iFreq , yOrigin + high - tft.getFontHeight() * 2);
    tft.print(eqFreq[iFreq]);
    tft.setCursor(xOrigin + (barWidth + 4) * iFreq + tft.getFontWidth() * 1.5 , yOrigin + high + tft.getFontHeight() * 2);
    tft.print(yLevel[iFreq]);
  }

  columnIndex = 0;                                // Get ready to set values for columns
  while (columnIndex < EQUALIZER_CELL_COUNT) {
    while (true) {
      newValue = yLevel[columnIndex];                            // Get current value
      if (filterEncoderMove != 0) {
        tft.fillRect(xOrigin  + (barWidth + 4) * columnIndex ,   // Indent to proper bar...
                     barBottomY - newValue - 1,                  // Start at red line
                     barWidth,                                   // Set bar width
                     newValue + 1,                               // Erase old bar
                     RA8875_BLACK);
        newValue += (PIXELS_PER_EQUALIZER_DELTA * filterEncoderMove); // Find new bar height. OK since filterEncoderMove equals 1 or -1       
        tft.fillRect(xOrigin  + (barWidth + 4) * columnIndex ,   // Indent to proper bar...
                     barBottomY - newValue,                      // Start at red line
                     barWidth,                                   // Set bar width
                     newValue,                                   // Draw new bar
                     RA8875_GREEN);
        yLevel[columnIndex] = newValue;
        tft.fillRect(xOrigin + (barWidth + 4) * columnIndex + tft.getFontWidth() * 1.5 - 1, yOrigin + high + tft.getFontHeight() * 2,
                     barWidth, CHAR_HEIGHT, RA8875_BLACK);
        tft.setCursor(xOrigin + (barWidth + 4) * columnIndex + tft.getFontWidth() * 1.5 , yOrigin + high + tft.getFontHeight() * 2);
        tft.print(yLevel[columnIndex]);
        if (newValue < DEFAULT_EQUALIZER_BAR) {                   // Repaint red center line if erased
          tft.drawFastHLine(xOrigin - 4, yOrigin + (high / 2), wide + 4, RA8875_RED);; // Clear hole in display center
        }
      }
      filterEncoderMove = 0;
      MyDelay(200L);

      val = ReadSelectedPushButton();                     // Read the ladder value

      if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
        val = ProcessButtonPress(val);                    // Use ladder value to get menu choice
        MyDelay(100L);

        tft.fillRect(xOrigin  + (barWidth + 4) * columnIndex ,   // Indent to proper bar...
                     barBottomY - newValue,                        // Start at red line
                     barWidth,                                     // Set bar width
                     newValue,                                     // Draw new bar
                     ORANGE);
        array[columnIndex] = newValue;
        filterEncoderMove = 0;
        columnIndex++;
        break;
      }      
    }
  }
}


/*****
  Purpose: Present the bands available and return the selection

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int EqualizerRecOptions()
{
  ProcessEqualizerChoices(EEPROMData.equalizerRec, (char *)"Receive Equalizer");
  EEPROMWrite();
  RedrawDisplayScreen();
  ShowName();
  return 0;
}

/*****
  Purpose: Present the bands available and return the selection

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int EqualizerXmtOptions()
{
  ProcessEqualizerChoices(EEPROMData.equalizerXmt, (char *) "Transmit Equalizer");
  EEPROMWrite();
  RedrawDisplayScreen();
  ShowName();
  return 0;
}

/*****
  Purpose: Turn mic compression on and set the level

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int MicOptions()
{
  const char *micChoices[] = {"On", "Off", "Cancel"};

  micChoice = SubmenuSelect(micChoices, 3, micChoice);
  switch (micChoice) {
    case 0:                           // On
      micChoice = 1;
      SetCompressionLevel();
      break;

    case 1:                           // Off
      micChoice = 0;
      break;

    default:                          // Cancelled choice
      micChoice = -1;
      break;

  }
  secondaryMenuIndex = -1;
  return micChoice;
}
/*****
  Purpose: Determine the frequency offset to provide more accurate frequency tuning

  Parameter list:
    void

  Return value
    int
*****/
int FrequencyOptions()
{
  int i;
  long currentDifference;
  long frequency;
  long difference = 100000L;
  long W1AWFrequencies[] = { 3581500,  3990000,  7047500,  7290000, 14047500, 14290000,
                             18097500, 18160000, 21067500, 21390000, 28067500, 28590000
                           };

  if (activeVFO == VFO_A) {                   // Which VFO are we setting??
    frequency = currentFreqA;
  } else {
    frequency = currentFreqB;
  }

  for (i = 0; i < 12; i++) {
    currentDifference = abs(W1AWFrequencies[i] - frequency);
    if (currentDifference < difference) {
      difference = W1AWFrequencies[i] - frequency;
    }
  }
  if (activeVFO == VFO_A) {                   // Which VFO are we setting??
    TxRxFreq = currentFreqA += difference;
  } else {
    TxRxFreq = currentFreqB += difference;
  }

  SetFreq();
  ShowFrequency();
  EEPROMData.frequencyOffset = difference;
  return activeVFO;
}

/*****
  Purpose: Present the bands available and return the selection

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int RFOptions()
{
  const char *rfOptions[] = {"Power level", "Gain",  "Cancel"};
  int rfSet = 0;
  int returnValue = 0;


  rfSet = SubmenuSelect(rfOptions, 3, rfSet);
  MyDelay(150L);

  switch (rfSet) {
    case 0:                                 // Power
      transmitPowerLevel = GetEncoderValue(1, 20, EEPROMData.powerLevel, 1, (char *) "Power: ");       // Argument: min, max, start, increment
      EEPROMData.powerLevel = transmitPowerLevel;
      EEPROMWrite();
      returnValue = transmitPowerLevel;
      BandInformation();
      break;

    case 1:                                 // Gain
      rfGain = GetEncoderValue(0, 100, rfGain, 5, (char *) "Gain: ");          // Argument: min, max, start, increment
      EEPROMData.rfGain = rfGain;
      EEPROMWrite();
      returnValue = rfGain;
      break;

    case 2:                                 // Tenuator
      attenuator = GetEncoderValue(0, 100, attenuator, 5, (char *) "Attenuation: ");          // Argument: min, max, start, increment
      EEPROMData.attenuator = attenuator;
      EEPROMWrite();
      returnValue = attenuator;
      break;
  }
  return returnValue;
}


/*****
  Purpose: This option reverses the dit and dah paddles on the keyer

  Parameter list:
    void

  Return value
    void
*****/
void DoPaddleFlip()
{
  const char *paddleState[] = {"Right paddle = dah", "Right paddle = dit"};
  int choice, lastChoice;
  int pushButtonSwitchIndex;
  int valPin;

  paddleDah = KEYER_DAH_INPUT_RING;           // Defaults
  paddleDit = KEYER_DIT_INPUT_TIP;
  choice = lastChoice = 0;

  tft.fillRect(250, 0, 320, CHAR_HEIGHT, RA8875_GREEN);
  tft.setTextColor(RA8875_BLACK);
  tft.setCursor(257, 1);
  tft.print(paddleState[choice]);                    // Show the default (right paddle = dah

  while (true) {
    MyDelay(150L);
    valPin = ReadSelectedPushButton();                      // Poll buttons
    if (valPin != -1) {                                     // button was pushed
      pushButtonSwitchIndex = ProcessButtonPress(valPin);     // Winner, winner...chicken dinner!
      if (pushButtonSwitchIndex == MAIN_MENU_UP || pushButtonSwitchIndex == MAIN_MENU_DN) {
        choice = !choice;                                     // Reverse the last choice
        tft.fillRect(250, 0, 320, CHAR_HEIGHT, RA8875_GREEN);
        tft.setCursor(257, 1);
        tft.print(paddleState[choice]);
      }

      if (pushButtonSwitchIndex == MENU_OPTION_SELECT) {                     // Made a choice??
        if (choice) {                                         // Means right-paddle dit
          paddleDit = KEYER_DAH_INPUT_RING;
          paddleDah = KEYER_DIT_INPUT_TIP;
        } else {
          paddleDit = KEYER_DIT_INPUT_TIP;
          paddleDah = KEYER_DAH_INPUT_RING;
        }
        EEPROMData.paddleDit = paddleDit;
        EEPROMData.paddleDah = paddleDah;
        tft.fillRect(250, 0, 320, CHAR_HEIGHT, RA8875_BLACK);   // Clear out menu line
        MyDelay(150L);
        return;
      }
    }
  }
}

/*****
  Purpose: Allows user to set the sidetone for the CPO. Sidetone for listening is done with frequency offset.

  Parameter list:
    void

  Return value
    void
*****/
void SetKeyerSidetone()
{
  int lastTone     = cwSidetone;
  int val;

  tft.setFontScale( (enum RA8875tsize) 1);

  tft.fillRect(250, 0, 130, CHAR_HEIGHT, RA8875_GREEN);
  tft.setTextColor(RA8875_BLACK);
  tft.setCursor(252, 1);
  tft.print(cwSidetone);

  while (true) {
    if (filterEncoderMove != 0) {                         // encoder was changed
      lastTone += filterEncoderMove;                      // Add change, + or -, to existing value
      if (lastTone < MIN_TONE) {
        lastTone = MIN_TONE;
      } else {
        if (lastTone > MAX_TONE) {
          lastTone = MAX_TONE;
        }
      }
      tft.fillRect(250, 0, 130, CHAR_HEIGHT, RA8875_GREEN);
      tft.setCursor(252, 1);
      tft.print(lastTone);
    }
    val = ReadSelectedPushButton();                     // Read the ladder value
    MyDelay(150L);

    if (val != -1  && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
      val = ProcessButtonPress(val);
      if (val == MENU_OPTION_SELECT) {                   // Make a choice??
        cwSidetone = lastTone;
        EEPROMData.keyerSidetone = lastTone;
        EEPROMWrite();
        break;
      }
    }
  }
}


/*****
  Purpose: Used to change the currently active VFO

  Parameter list:
    void

  Return value
    int             // the currently active VFO, A = 1, B = 0
*****/
int VFOSelect()
{
  const char *VFOOptions[] = {"VFO A", "VFO B", "Split", "Cancel"};
  int toggle;
  int choice, lastChoice;
 
  choice = lastChoice = toggle = activeVFO;

  tft.setTextColor(RA8875_BLACK);
  tft.fillRect(250, 0, 300, CHAR_HEIGHT, RA8875_GREEN);
  tft.setCursor(257, 1);
  tft.print(VFOOptions[choice]);                    // Show the default (right paddle = dah

  splitOn = 0;                                      // Assume no split
  choice = SubmenuSelect(VFOOptions, 4 , 0);
  switch (choice) {
    case 0:                                     // VFO A
      TxRxFreq = currentFreqA;
      activeVFO = VFO_A;
      tft.fillRect(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y, 150, 20, RA8875_BLACK);      // Erase split message
      break;

    case 1:                                     // VFO B
      TxRxFreq = currentFreqB;
      activeVFO = VFO_B;
      tft.fillRect(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y, 150, 20, RA8875_BLACK);      // Erase split message
      break;

    case 2:                                     // Split
      DoSplitVFO();
      splitOn = 1;
      break;
    case 3:                                     // Cancel
      return activeVFO;
      break;
  }

  bands[currentBand].freq = TxRxFreq;
  SetFreq();
  BandInformation();
  ShowBandwidth();
  FilterBandwidth();
  EEPROMData.currentVFO = activeVFO;

  tft.fillRect(FREQUENCY_X_SPLIT, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK); // delete old digit
  tft.fillRect(FREQUENCY_X,       FREQUENCY_Y - 12, VFOA_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK); // delete old digit  tft.setFontScale( (enum RA8875tsize) 0);
  ShowFrequency();
  return activeVFO;
}

/*****
  Purpose: Allow user to set current EEPROM values or restore default settings

  Parameter list:
    void

  Return value
    int           the user's choice
*****/
int EEPROMOptions()
{
  const char *EEPROMOpts[] = {"Save Current", "Set Defaults", "Get Favorite", "Set Favorite", "Cancel"};
  int defaultOpt = 0;

  defaultOpt = SubmenuSelect(EEPROMOpts, 5, defaultOpt);
  switch (defaultOpt) {
    case 0:                                   // Save current values
      EEPROMWrite();
      break;

    case 1:
      EEPROMSaveDefaults();                   // Restore defaults
      break;

    case 2:
      GetFavoriteFrequency();               // Get a stored frequency and store in active VFO
      break; 
      
    case 3:
      SetFavoriteFrequency();               // Save current frequency to EEPROM
      break;

    default:
      defaultOpt = -1;                        // No choice made
      break;
  }
  return defaultOpt;
}


/*****
  Purpose: To select an option from a submenu

  Parameter list:
    char *options[]           submenus
    int numberOfChoices       choices available
    int defaultState          the starting option

  Return value
    int           an index into the band array

                                                            // Added to make easier to reference arrays below
  // int switchThreshholds[16] = {503, 517, 547, 560,
  //                              403, 431, 460, 483,
  //                              248, 296, 335, 369,
  //                                5,  76, 143, 203};
  int (*functionPtr[])() = {&CWOptions,  &SpectrumOptions, &AGCOptions,
                            &NROptions, &IQOptions, &EqualizerRecOptions, &EqualizerXmtOptions,
                            &MicOptions, &FrequencyOptions, &NBOptions, &RFOptions,
                            &EEPROMOptions
                           };

  #define MAIN_MENU_UP                503
  #define MAIN_MENU_DN                403
  #define BAND_UP                     517
  #define BAND_DN                     431
  #define SECONDARY_MENU_UP           547
  #define SECONDARY_MENU_DN           460
  #define MENU_OPTION_SELECT          560
  #define FAST_TUNE_FILTER_SELECT       5
  #define INCREMENT                   483
  #define MODE                        248
  #define FILTER                      296
  #define NOISE_FLOOR                 335
  #define DEMODULATION                369
  #define ZOOM                         76
  #define UNUSED_15                   143
  #define UNUSED_16                   203
*****/
int SubmenuSelect(const char *options[], int numberOfChoices, int defaultStart)
{
  int refreshFlag    = 0;
  int val;
  int encoderReturnValue;

  tft.setTextColor(RA8875_BLACK);
  encoderReturnValue = defaultStart;                        // Start the options using this option

  tft.setFontScale( (enum RA8875tsize) 1);
  if (refreshFlag == 0) {
    tft.fillRect(250, 0, 250, CHAR_HEIGHT, RA8875_GREEN);   // Show the option in the second field
    tft.setCursor(257, 1);
    tft.print(options[encoderReturnValue]);                             // Secondary Menu
    refreshFlag = 1;
  }
  MyDelay(150L);

  while (true) {
    val = ReadSelectedPushButton();                     // Read the ladder value
    MyDelay(150L);

    if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
      val = ProcessButtonPress(val);                    // Use ladder value to get menu choice
      MyDelay(150L);

      if (val > -1) {                                   // Valid choice?
        switch (val) {
          case MENU_OPTION_SELECT:                        // They made a choice
            tft.setTextColor(RA8875_WHITE);
//            tft.fillRect(0, 0, RIGNAME_X_OFFSET - 2, CHAR_HEIGHT, RA8875_BLACK);
            EraseMenus();
            return encoderReturnValue;
            break;

          case MAIN_MENU_UP:
            encoderReturnValue++;
            if (encoderReturnValue >= numberOfChoices)
              encoderReturnValue = 0;
            break;

          case MAIN_MENU_DN:
            encoderReturnValue--;
            if (encoderReturnValue < 0)
              encoderReturnValue = numberOfChoices - 1;
            break;

          default:
            encoderReturnValue = -1;                                    // An error selection
            break;
        }
        if (encoderReturnValue != -1) {
          tft.fillRect(250, 0, 250, CHAR_HEIGHT, RA8875_GREEN);
          tft.setTextColor(RA8875_BLACK);
          tft.setCursor(257, 1);
          tft.print(options[encoderReturnValue]);
          MyDelay(50L);
          refreshFlag = 0;
        }
      }
    }
  }
}
