#ifndef BEENHERE
#include "SDT.h"
#endif

/*****
  Purpose: To process a menu increase button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonMenuIncrease()
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
  Purpose: To process a menu decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonMenuDecrease()
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
  Purpose: Chnage the horizontal scale of the frequency display

  Parameter list:
    void

  Return value:
    int             index of the option selected
*****/
void ButtonZoom()
{
  zoomIndex++;

  if (zoomIndex == MAX_ZOOM_ENTRIES) {
    zoomIndex = 0;
  }
  if (zoomIndex == 0)
    spectrum_zoom = 0;
  else
    spectrum_zoom = zoomIndex;

  ZoomFFTPrep();
  UpdateZoomField();
  DrawFrequencyBarValue();
  ShowBandwidth();
}

/*****
  Purpose: To process a filter button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonFilter()
{
  switchFilterSideband = !switchFilterSideband;
  ControlFilterF();
  FilterBandwidth();
  SetFreq();
  ShowFrequency();
}

/*****
  Purpose: Process demodulation mode

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonDemodMode()
{
  bands[currentBand].mode++;
  if (bands[currentBand].mode > DEMOD_MAX) {
    bands[currentBand].mode = DEMOD_MIN;            // cycle thru demod modes
  }
  AudioNoInterrupts();
  SetupMode(bands[currentBand].mode);
  ShowFrequency();
  ControlFilterF();
  FilterBandwidth();
  DrawSMeterContainer();
  ShowAnalogGain();
  AudioInterrupts();
}


/*****
  Purpose: Set transmission mode for SSB or CW

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonMode()
{
  if (xmtMode == CW_MODE) {
    xmtMode = SSB_MODE;
  } else {
    xmtMode = CW_MODE;
  }
  DrawSpectrumDisplayContainer();
  BandInformation();
  if (xmtMode == CW_MODE) {                             // Doing CW
    decoderFlag = CW_MODE;
    ShowDecoderMessage();
    modeSelectOutL.gain(0, 5);
    modeSelectOutR.gain(0, 5);
    FLoCutOld = bands[currentBand].FLoCut;
    FHiCutOld = bands[currentBand].FHiCut;
    if (currentBand < 2) {
      currentFreqA = centerFreq + NCO_FREQ;
      ShowFrequency();
      bands[currentBand].FLoCut = -3000;
      bands[currentBand].FHiCut = -200;
    } else {
      currentFreqA = centerFreq + NCO_FREQ;
      ShowFrequency();
      bands[currentBand].FLoCut = 200;
      bands[currentBand].FHiCut = 3000;
    }
    TxRxFreq = centerFreq - CWFreqShift;
    SetFreq();
  } else {                                              // Doing SSB
    decoderFlag = SSB_MODE;
    modeSelectOutL.gain(0, 1.0);
    modeSelectOutR.gain(0, 1.0);
    if (currentBand < 2) {
      bands[currentBand].FLoCut = FLoCutOld;
      bands[currentBand].FHiCut = FHiCutOld;
    } else {
      bands[currentBand].FLoCut = FLoCutOld;
      bands[currentBand].FHiCut = FHiCutOld;
    }
    TxRxFreq = centerFreq;
    currentFreqA = centerFreq + NCO_FREQ;
    SetFreq();
    ShowFrequency();
  }
  EraseSpectrumDisplayContainer();
  DrawSpectrumDisplayContainer();
  UpdateDecoderField();
  ControlFilterF();
  FilterBandwidth();
  SetFreq();
  ShowFrequency();


}

/*****
  Purpose: To process select noise reduction

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonNR()  //AFP 2-10-21
{
  NR_Index++;
  if (NR_Index > 3) {
    NR_Index = 0;
  }
  UpdateNoiseField();
}

/*****
  Purpose: To set the notch filter

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonNotchFilter()
{
  ANR_notchOn = !ANR_notchOn;
  MyDelay(100L);
}

/*****
  Purpose: How to present the display

  Parameter list:
    void

  Return value:
    int                       the display option
*****/
int ButtonDisplayOptions()
{
  const char *displayChoices[] = {"Spectrum", "Waterfall", "Both", "None", "Cancel"};
  int currentDisplayMode = displayMode;

  currentDisplayMode = SubmenuSelect(displayChoices, 5, displayMode);
  switch (currentDisplayMode) {
    case SPECTRUM_ONLY:                               // Spectrum only
      displayMode = SPECTRUM_ONLY;
      RefreshMainDisplay(displayMode);
      break;

    case WATERFALL_ONLY:                              // Waterfall only
      displayMode = WATERFALL_ONLY;
      RefreshMainDisplay(displayMode);
      break;

    case BOTH_DISPLAYS:                               // Both displays active
      displayMode = BOTH_DISPLAYS;
      RefreshMainDisplay(displayMode);
      break;

    case NO_DISPLAY:                                  // Neither spectrum or waterfall active
      displayMode = NO_DISPLAY;
      EraseSpectrumDisplayContainer();
      break;

    case 4:                                           // Set noise floor
      ButtonSetNoiseFloor();
      break;

    case 5:                                           // Cancel
    default:
      displayMode = -1;
      break;
  }
  RefreshMainDisplay(displayMode);
  return displayMode;
}

/*****
  Purpose: Allows quick setting of noise floor in spectrum display

  Parameter list:
    void

  Return value;
    int           the current noise floor value
*****/
int ButtonSetNoiseFloor()
{
  int currentNoiseFloor = spectrumNoiseFloor;
  int val;

  currentNoiseFloor = (SPECTRUM_TOP_Y + SPECTRUM_HEIGHT - 3) - spectrumNoiseFloor;      // Adjust to display
  tft.setFontScale( (enum RA8875tsize) 1);

  MyDelay(150L);
  tft.fillRect(SECONDARY_MENU_X, 0, MENU_WIDTHS, CHAR_HEIGHT, RA8875_MAGENTA);   // Large second prompt moved left 30 pixels
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X, 1);
  tft.print("dB above X:");
  tft.setCursor(SECONDARY_MENU_X + 200, 1);
  tft.print(currentNoiseFloor);
  while (true) {
    if (filterEncoderMove != 0) {
      currentNoiseFloor += filterEncoderMove;                 // It moves the display
      if (currentNoiseFloor < 0) {               // Don't exceed limits
        currentNoiseFloor = 0;
      } else {
        if (currentNoiseFloor > 50) {
          currentNoiseFloor = 50;
        }
      }
      tft.fillRect(SECONDARY_MENU_X + 190, 0, 80, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 200, 1);
      tft.print(currentNoiseFloor);
      filterEncoderMove = 0;
    }
    val = ReadSelectedPushButton();              // Get ADC value
    MyDelay(100L);
    val = ProcessButtonPress(val);
    if (val == MENU_OPTION_SELECT)               // If they made a choice...
    {
      spectrumNoiseFloor = (SPECTRUM_TOP_Y + SPECTRUM_HEIGHT - 2) - currentNoiseFloor;    // -2 to force above X axis
      if (spectrumNoiseFloor > SPECTRUM_BOTTOM) {
        spectrumNoiseFloor = SPECTRUM_BOTTOM;
      } else {
        if (spectrumNoiseFloor < SPECTRUM_BOTTOM - 50)
          spectrumNoiseFloor = SPECTRUM_BOTTOM - 50;
      }
      EEPROMData.spectrumNoiseFloor = spectrumNoiseFloor;
      EEPROMWrite();
      break;
    }
  }
  EraseMenus();
  EraseSpectrumDisplayContainer();
  tft.setTextColor(RA8875_WHITE);
  DrawSpectrumDisplayContainer();
  ShowSpectrumdBScale();
  ShowSpectrum();
  CenterFastTune();
  return spectrumNoiseFloor;
}
