//--------------------------------------------------------------------------------------

#include "power_and_battery.h"
#include "fpga_control.h"        //Port E functionality is defined here
#include "interrupt.h"
#include "variables.h"
#include "scope_functions.h"

//--------------------------------------------------------------------------------------

void battery_check_init(void)
{
  //Set KEY ADC to this fixed value for now. Raises level B to 1.8V and enables the conversions
  *KEY_ADC_CFG_REG = 0x010000159;
  
  //Set port pin E12 as input
  *FPGA_CTRL_CFG_REG &= 0xFFF0FFFF;
}

//--------------------------------------------------------------------------------------

void battery_check_status(void)
{
  int32 chargelevel;
  
  //Get the status of the battery charging indicator pin
  //A low level on the pin means charging
  scopesettings.batterycharging = !(*FPGA_DATA_REG & 0x00001000);
  
  //Get data from the key adc, which is current battery level
  chargelevel = *KEY_ADC_DATA_REG & 0x3F;

  //Compensate for higher brightness settings. When brighter the current is higher and the internal resistance of the battery lowers the voltage while the charge is still there
  chargelevel = chargelevel + ((scopesettings.screenbrightness * chargelevel) / 1000);

  //Compensate for when charging
  if(scopesettings.batterycharging)
  {
    //When charging the measured voltage is ~0,21875V higher
    chargelevel -= 7;
  }

  //The original code has some lowest over time detection based on time base setting which is not done here
  //Need to see how it behaves and if not some filtering is needed
  
  //It is more or less done this way in the original code, so also here
  //Take of the minimum value needed?? Below this value the scope most likely won't work any more
  chargelevel -= 25;
  
  //Make sure it does not drop below zero
  if(chargelevel <= 0)
  {
    chargelevel = 0;
  }
  else
  {
    //Tweak it down just a little
    chargelevel = (chargelevel * 20) / 21;
  }
  
  //Copy it to the global variable
  scopesettings.batterychargelevel = chargelevel;
  
  //Display the level on the screen
  scope_battery_status();
}

//--------------------------------------------------------------------------------------

void power_interrupt_init(void)
{
  //The original code uses functions for these settings
  //Set port pin E11 as interrupt input
  *FPGA_CTRL_CFG_REG = (*FPGA_CTRL_CFG_REG & 0xFFFF0FFF) | 0x00006000;
  
  //Set the external interrupt configuration for it. Probably some trigger edge setting
  *EINT_PE_CFG_REG1 = (*EINT_PE_CFG_REG1 & 0xFFFF0FFF) | 0x00001000;
  
  //Enable the external interrupt???
  *EINT_PE_ENB_REG |= 0x00000800;
  
  //Link the handler to the interrupt handling
  setup_interrupt(PORTE_EINT_IRQ, power_interrupt_handler, 3);
}

//--------------------------------------------------------------------------------------

void power_interrupt_handler(void)
{
  //Need to check if it is ok to save settings.
  //Only when in normal view mode, so need a flag to signal picture or waveform viewing
  //And a flag to signal actual setting has been changed???
  
  //Also need some scheme for wear leveling of the FLASH. It has enough room to do so
  
  //Check if not in view mode
  if(viewactive == VIEW_NOT_ACTIVE)
  {
    //Get the settings in the working buffer and write them to the flash
    scope_save_configuration_data();
  }
  
  //Power has been turned off and settings are saved when needed so hang here until complete shutdown
  while(1);
}

//--------------------------------------------------------------------------------------
