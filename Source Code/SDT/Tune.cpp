#ifndef BEENHERE
#include "SDT.h"
#endif


/*****
  Purpose: SetFrequency

  Parameter list:
    void

  Return value;
    void

  CAUTION: SI5351_FREQ_MULT is set in the si5253.h header file and is 100UL
*****/
void SetFreq() {

  // Changes for Bobs Octave Filters:  18 March 2018  W7PUA <<<<<<
  // http://www.janbob.com/electron/FilterBP1/FiltBP1.html
  // NEVER USE AUDIONOINTERRUPTS HERE: that introduces annoying clicking noise with every frequency change
  // SI5351_FREQ_MULT is 100ULL, MASTER_CLK_MULT is 4;

  hilfsf = ((centerFreq * SI5351_FREQ_MULT) + IFFreq * SI5351_FREQ_MULT) * MASTER_CLK_MULT * 1000000000;        // AFP 12-30-21

  if (IQ_RecCalFlag == 0) {                                                                                     //IQ receive Test flag
    hilfsfEx = ((TxRxFreq + CWFreqShift) * SI5351_FREQ_MULT) * MASTER_CLK_MULT * 1000000000;                    //Xmit Clock CLK1
  } else {
    if (IQ_RecCalFlag == 1) {
      hilfsfEx = ((7180000 * SI5351_FREQ_MULT)) * 1 * 1000000000;                                               //create test tone at 7.180MHZ for IQ calibration
    }
  }

  hilfsf = hilfsf / calibration_factor;
  hilfsfEx = hilfsfEx / calibration_factor;

  si5351.set_freq(hilfsf, SI5351_CLK2);                                                                         // this is actually SI5351_CLK2
  si5351.set_freq(hilfsfEx, SI5351_CLK1);                                                                       // this is actually SI5351_CLK1

//  DrawBandWidthIndicatorBar();
  DrawFrequencyBarValue();
//  CenterFilterOverlay();
}

/*****
  Purpose: Set Frequency Display Cursor for Fast Tune

  Parameter list:

  Return value;
    void
    Extensively modified 12-23-21 AFP
    Note:  Fast tune frequency offset is determined in FreqShift2()
*****/
void SetFineTuneFrequency()
{
  if (newCursorPosition != oldCursorPosition) {
    //DrawBandWidthIndicatorBar();                                      // Did fast tune cursor change? // AFP 03-27-22 Layers
//    CenterFilterOverlay();                // JJP 7/24/22    
    oldCursorPosition = newCursorPosition;
    TxRxFreq = centerFreq + NCO_FREQ;
    currentFreqA = TxRxFreq;
    ShowSpectrum();
  }
  if (xmtMode == CW_MODE && decoderFlag == true) { // No reason to reset if we're not doing decoded CW
    ResetHistograms();
  }
  if (displayMode == NO_DISPLAY || displayMode == WATERFALL_ONLY) {   // Don't show a Spectrum
    return;
  }
  tft.writeTo(L1);                                                    //AFP 03-27-22 Layers

}

/*****
  Purpose: Places the Fast Tune cursor in the center of the spectrum display

  Parameter list:

  Return value;
    void
*****/
void CenterFastTune()
{
  //oldCursorPosition = newCursorPosition = 256;    // AFP 12-10-21
  //  fineTuneEncoder.write(newCursorPosition);
  tft.drawFastVLine(oldCursorPosition, SPECTRUM_TOP_Y + 20, SPECTRUM_HEIGHT - 27, RA8875_BLACK);
  tft.drawFastVLine(newCursorPosition , SPECTRUM_TOP_Y + 20, SPECTRUM_HEIGHT - 27, RA8875_RED);
}

/*****
  Purpose: Purpose is to sety VFOa to receive frequency and VFOb to the transmit frequency

  Parameter list:
    void

  Return value;
    int           the offset as an int

  CAUTION: SI5351_FREQ_MULT is set in the si5253.h header file and is 100UL
*****/
int DoSplitVFO()
{
  char freqBuffer[15];
  int val;
  long chunk = SPLIT_INCREMENT;
  long splitOffset;
  
  //  TxRxFreq = currentFreqB;
  //  activeVFO = VFO_B;

                // 537 - 10 = 527             292 - 2 = 290
  tft.drawRect(INFORMATION_WINDOW_X - 10, INFORMATION_WINDOW_Y - 2, 260, 200, RA8875_MAGENTA);
  tft.fillRect(INFORMATION_WINDOW_X - 8, INFORMATION_WINDOW_Y, 250, 170, RA8875_BLACK);  // Clear volume field
  tft.setFontScale( (enum RA8875tsize) 1);
  tft.setCursor(INFORMATION_WINDOW_X + 10, INFORMATION_WINDOW_Y + 5);
  tft.print("xmit offset: ");

  splitOffset = chunk;                                                    // Set starting offset to 500Hz
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(INFORMATION_WINDOW_X + 60, INFORMATION_WINDOW_Y + 90);
  tft.print(splitOffset);
  tft.print("Hz  ");

  while (true) {
    if (filterEncoderMove != 0) {                     // Changed encoder?
      splitOffset += filterEncoderMove * chunk;
      tft.fillRect(INFORMATION_WINDOW_X + 60, INFORMATION_WINDOW_Y + 90, 150, 50, RA8875_BLACK);
      tft.setCursor(INFORMATION_WINDOW_X + 60, INFORMATION_WINDOW_Y + 90);
      tft.print(splitOffset);
      tft.print("Hz  ");
    }
    filterEncoderMove = 0L;

    val = ReadSelectedPushButton();                                  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    MyDelay(150L);
    if (val == MENU_OPTION_SELECT) {                              // Make a choice??
      hilfsfEx += splitOffset;                                    // New transmit frequency
      ShowDefaultSettings();
      filterEncoderMove = 0L;
      break;
    }
  }
  currentFreqB = currentFreqA + splitOffset;
  FormatFrequency(currentFreqB, freqBuffer);
  tft.fillRect(FREQUENCY_X_SPLIT, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);
  tft.setCursor(FREQUENCY_X_SPLIT, FREQUENCY_Y);
  tft.setFont(&FreeMonoBold24pt7b);
  tft.setTextColor(RA8875_GREEN);
  tft.print(freqBuffer);                                          // Show VFO_A

  tft.setFont(&FreeMonoBold18pt7b);
  FormatFrequency(currentFreqA, freqBuffer);
  tft.setTextColor(RA8875_LIGHT_GREY);
  tft.setCursor(FREQUENCY_X, FREQUENCY_Y + 6);
  tft.print(freqBuffer);                                          // Show VFO_A

  tft.useLayers(1);                 //mainly used to turn on layers! //AFP 03-27-22 Layers
  tft.layerEffect(OR);
  tft.writeTo(L2);
  tft.clearMemory();
  tft.writeTo(L1);

//  tft.setFontScale( (enum RA8875tsize) 0);
  tft.setFont(&FreeMono9pt7b);
  tft.setTextColor(RA8875_RED);
  tft.setCursor(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y + 6);
  tft.print("Split Active");

  tft.setFontDefault();
  return (int) splitOffset;                                       // Can be +/-
}
