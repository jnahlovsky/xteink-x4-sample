// DebugHelper.cpp
#include "DebugHelper.h"

DebugHelper::DebugHelper(BatteryMonitor &battery)
    : _battery(battery)
{
}

void DebugHelper::logDebugInfo()
{
  // Log raw analog levels
  int rawBat = analogRead(BAT_GPIO0);
  int rawBtn1 = analogRead(BTN_GPIO1);
  int rawBtn2 = analogRead(BTN_GPIO2);
  int rawBtn3 = digitalRead(BTN_GPIO3);

  Serial.print("ADC BTN1=");
  Serial.print(rawBtn1);
  Serial.print("    BTN2=");
  Serial.print(rawBtn2);
  Serial.print("    BTN3=");
  Serial.print(rawBtn3);
  Serial.println("");

  // Log battery info
  bool charging = digitalRead(UART0_RXD) == HIGH;
  Serial.printf("== Battery (charging: %s) ==\n", charging ? "yes" : "no");
  Serial.print("Value from pin (raw/calibrated): ");
  Serial.print(rawBat);
  Serial.print(" / ");
  Serial.println(BatteryMonitor::millivoltsFromRawAdc(rawBat));
  Serial.print("Volts: ");
  Serial.println(_battery.readVolts());
  Serial.print("Charge level: ");
  Serial.println(_battery.readPercentage());
  Serial.println("");
}
