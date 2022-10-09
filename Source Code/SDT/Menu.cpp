#ifndef BEENHERE
#include "SDT.h"
#endif
  // Useful in understanding menues:
  //
  //  const char *topMenus[]    = { "CW", "Display Choices", "Spectrum Set", "AGC", "NR Set",
  //                                "IQ Manual", "EQ Rec Set", "EQ Xmt Set", "Mic Comp", "Freq Cal",
  //                                "NB Set", "RF Set", "Audio Post Proc", "VFO Select", "EEPROM"      
  //                              };
  /* int (*functionPtr[])()     = {&CWOptions,  &SpectrumOptions, &AGCOptions, &NROptions, 
                                   &IQOptions, &EqualizerRecOptions, &EqualizerXmtOptions, &MicOptions, &FrequencyOptions, 
                                   &NBOptions, &RFOptions, &PostProcessorAudio, &VFOSelect, &EEPROMOptions
                                   }; 
*/
/*****
  Purpose: void ShowMenu()

  Parameter list:
    char *menuItem          pointers to the menu
    int where               0 is a primary menu, 1 is a secondary menu

  Return value;
    void
*****/
void ShowMenu(const char *menu[], int where)
{
  tft.setFontScale( (enum RA8875tsize) 1);  

  if (where == PRIMARY_MENU) {                          // Should print on left edge of top line
    tft.fillRect(0, 0, 300, CHAR_HEIGHT, RA8875_BLUE);        // Top-left of display
    tft.setCursor(5, 0);
    tft.setTextColor(RA8875_WHITE);
    tft.print(*menu);                                      // Primary Menu
  } else {
    tft.fillRect(250, 0, 300, CHAR_HEIGHT, RA8875_GREEN);        // Right of primary display
    tft.setCursor(35, 0);
    tft.setTextColor(RA8875_WHITE);
    tft.print(*menu);                                      // Primary Menu
  }
  return;
}
