#ifndef BEENHERE
#include "SDT.h"
#endif

/*****
  Purpose: EncoderFilter

  Parameter list:
    void

  Return value;
    void
    Modified AFP21-12-15
*****/
void FilterSetSSB()
{
  long filter_change;

  // SSB
  if (filter_pos != last_filter_pos) {
    tft.fillRect((MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2 - filterWidth, SPECTRUM_TOP_Y + 17, filterWidth, SPECTRUM_HEIGHT - 20, RA8875_BLACK); // Erase old filter background
    filter_change = (filter_pos - last_filter_pos);
    if (filter_change >= 1) {
      filterWidth--;
      if (filterWidth < 10)
        filterWidth = 10;
    }
    if (filter_change <= -1) {
      filterWidth++;
      if (filterWidth > 100)
        filterWidth = 50;
    }
    last_filter_pos = filter_pos;
    if (bands[currentBand].mode == DEMOD_LSB) {
      if (switchFilterSideband == 0)      // "0" = normal, "1" means change opposite filter
      {
        bands[currentBand].FLoCut = bands[currentBand].FLoCut + filter_change * 50 * ENCODER_FACTOR;
        FilterBandwidth();
      } else if (switchFilterSideband == 1) {
        //if (abs(bands[currentBand].FHiCut) < 500) {
        bands[currentBand].FHiCut = bands[currentBand].FHiCut + filter_change * 50 * ENCODER_FACTOR;
      }
    } else if (bands[currentBand].mode == DEMOD_USB) {
      if (switchFilterSideband == 0) {
        bands[currentBand].FHiCut = bands[currentBand].FHiCut - filter_change * 50 * ENCODER_FACTOR;
        FilterBandwidth();
      } else if (switchFilterSideband == 1) {
        bands[currentBand].FLoCut = bands[currentBand].FLoCut - filter_change * 50 * ENCODER_FACTOR;
      }
    }
    ControlFilterF();
    Menu2 = MENU_F_LO_CUT;      // set Menu2 to MENU_F_LO_CUT
    FilterBandwidth();
    SetFreq();
    ShowFrequency();
  }
  notchPosOld = filter_pos;

} // end encoder2 was turned


/*****
  Purpose: Main tune frequency encoder
  Parameter list:
    void
  Return value;
    void
*****//*
  void MainTune()
  {
  unsigned char result  = tuneEncoder.process();  // Read the encoder
  switch (result) {
    case 0:                                       // Nothing happened
      mainTuneEncoder = 0;
      break;

    case DIR_CW:                                  // Turned it clockwise, 16
      mainTuneEncoder = 1;
      break;

    case DIR_CCW:                                 // Turned it counter-clockwise
      mainTuneEncoder = -1;
      break;
  }
  MyDelay(10);
  }
*/
/*****
  Purpose: EncoderTune
  Parameter list:
    void
  Return value;
    void
*****/
void EncoderTune()
{
  int audioVolumeOld   = audioVolume;
  long tuneChange      = 1L;
  long old_freq;
  long newFreq;

  unsigned char result  = tuneEncoder.process();   // Read the encoder

  if (result == 0)                                // Nothing read
    return;

  if (xmtMode == CW_MODE && decoderFlag == true) {        // No reason to reset if we're not doing decoded CW
    ResetHistograms();
  }
  old_freq = centerFreq;

  switch (result) {
    case DIR_CW:                                  // Turned it clockwise, 16
      tuneChange = 1L;
      break;

    case DIR_CCW:                                 // Turned it counter-clockwise
      tuneChange = -1L;
      break;
  }
  audioVolume    = 0;
  MyDelay(10);

  newFreq = (long)freqIncrement * tuneChange;

  centerFreq += newFreq;                // tune the master vfo and check boundaries
  if (centerFreq > F_MAX) {
    centerFreq = F_MAX;
  } else if (centerFreq < F_MIN) {
    centerFreq = F_MIN;
  }
  if (centerFreq != old_freq) {            // If the frequency has changed...
    Q_in_L.clear();
    Q_in_R.clear();
    TxRxFreq = centerFreq + NCO_FREQ;
    SetFreq();
    audioVolume = audioVolumeOld;  //Restore audio volume AFP12-17-21
    CenterFilterOverlay();
    ShowFrequency();
  }
}

/*****
  Purpose: Encoder volume control

  Parameter list:
    void

  Return value;
    int               0 means encoder didn't move; otherwise it moved
*****/
void EncoderVolume()
{
  char result;
  int adjustVol = 0;

  result = volumeEncoder.process();    // Read the encoder

  if (result == 0) {                                    // Nothing read
    return;
  }

  switch (result) {
    case DIR_CW:                          // Turned it clockwise, 16
      adjustVol = 1;
      break;

    case DIR_CCW:                         // Turned it counter-clockwise
      adjustVol = -1;
      break;
  }
  audioVolume += adjustVol;

  if (audioVolume > 100) {                // In range?
    audioVolume = 100;
  } else {
    if (audioVolume < 0) {
      audioVolume = 0;
    }
  }
  volumeChangeFlag = true;        // Need this because of unknown timing in display updating.
}

/*****
  Purpose: Use the encoder to change the value of a number in some other function

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
int GetEncoderValue(int minValue, int maxValue, int startValue, int increment, char prompt[])
{
  int currentValue = startValue;
  int val;

  tft.setFontScale( (enum RA8875tsize) 1);

  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, 280, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setCursor(SECONDARY_MENU_X + 5, MENUS_Y + 1);
  tft.print(prompt);
  tft.setCursor(SECONDARY_MENU_X + 150, MENUS_Y + 1);
  tft.print(startValue);

  while (true) {
    if (filterEncoderMove != 0) {
      currentValue += filterEncoderMove * increment;    // Bump up or down...

      if (currentValue < minValue) {
        currentValue = minValue;
      } else {
        if (currentValue > maxValue) {      
          currentValue = maxValue;
        }
      }
      if (filterEncoderMove != 0) {
        tft.fillRect(SECONDARY_MENU_X + 150, MENUS_Y + 1, 65, CHAR_HEIGHT, RA8875_MAGENTA);   // Show numeric encoder value
        tft.setCursor(SECONDARY_MENU_X + 150, MENUS_Y + 1);
        tft.print(currentValue);
      }
    }
    filterEncoderMove = 0;
    MyDelay(200L);
    
    val = ReadSelectedPushButton();                     // Read the ladder value
    MyDelay(100L);
    if (val != -1  && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
      val = ProcessButtonPress(val);                    // Use ladder value to get menu choice
      MyDelay(100L);
      if (val == MENU_OPTION_SELECT) {                  // Make a choice??
        menuStatus = NO_MENUS_ACTIVE;                   // We're done...
        return currentValue;
      }
    }
//    filterEncoderMove = 0;
  }
}

/*****
  Purpose: Allows quick setting of WPM without going through a menu

  Parameter list:
    void

  Return value;
    int           the current WPM
*****/
int SetWPM()
{
  int val;
  long lastWPM     = currentWPM;

  tft.setFontScale( (enum RA8875tsize) 1);

  tft.fillRect(251, 0, 250, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(252, 1);
  tft.print("current WPM:");
  tft.setCursor(450, 1);
  tft.print(currentWPM);

  while (true) {
    if (filterEncoderMove != 0) {                     // Changed encoder?
      currentWPM += filterEncoderMove;                // Yep
      lastWPM = currentWPM;
      if (lastWPM < 0)
        lastWPM = 5;
      else if (lastWPM > MAX_WPM)
        lastWPM = MAX_WPM;

      tft.fillRect(450, 0, 50, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(450, 1);
      tft.print(lastWPM);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();                                  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    MyDelay(150L);
    if (val == MENU_OPTION_SELECT) {                             // Make a choice??
      tft.fillRect(251, 0, 250, CHAR_HEIGHT, RA8875_BLACK);
      currentWPM = lastWPM;
      EEPROMData.wordsPerMinute = currentWPM;
      EEPROMWrite();
      UpdateWPMField();
      break;
    }
  }
  tft.fillRect(251, 0, 250, CHAR_HEIGHT, RA8875_BLACK);
  return currentWPM;
}


/*****
  Purpose: Filter fast tune control.

  Parameter list:
    void

  Return value;
    int               cannot return value
*****/
FASTRUN                   // Causes function to be allocated in RAM1 at startup for fastest performance.
void EncoderFineTune()
{
  char result;

  result = fineTuneEncoder.process();       // Read the encoder
  if (result == 0) {                        // Nothing read
    fineTuneEncoderMove = 0L;
    return;
  } else {
    if (result == DIR_CW) {                 // 16 = CW, 32 = CCW
      fineTuneEncoderMove = 1L;
    } else {
      fineTuneEncoderMove = -1L;
    }
  }
  NCO_FREQ += stepFT * fineTuneEncoderMove;

  if (activeVFO == VFO_A) {
    currentFreqA = centerFreq + NCO_FREQ;
  } else {
     currentFreqB = centerFreq + NCO_FREQ;
  }
  fineTuneEncoderMove = 0L;
}
FASTRUN                   // Causes function to be allocated in RAM1 at startup for fastest performance.
void EncoderFilter()
{
  char result;

  result = filterEncoder.process();   // Read the encoder

  if (result == 0) {
    return;
  }

  switch (result) {
    case DIR_CW:                          // Turned it clockwise, 16
      filterEncoderMove = 1;
      filter_pos = last_filter_pos - 5 * filterEncoderMove;
      break;

    case DIR_CCW:                         // Turned it counter-clockwise
      filterEncoderMove = -1;
      filter_pos = last_filter_pos - 5 * filterEncoderMove;
      break;
  }
}
