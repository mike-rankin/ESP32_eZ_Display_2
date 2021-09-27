/*
  Digital and analogRead
  Battery analog pin 32
  Battery charging pin 25
  
*/
int Charging = 25;


void setup() 
{
 Serial.begin(115200);
 pinMode(Charging, INPUT);
}


void loop() 
{
  int ChargeState = digitalRead(Charging);   // read the input pin:

  Serial.println(readBattery());
  Serial.println(ChargeState);
  delay(1000);       
}

String readBattery()
{
  uint8_t percentage = 100;
  float voltage = analogRead(32) / 4096.0 * 7.23;    
  Serial.println("Voltage = " + String(voltage));
  percentage = 2808.3808 * pow(voltage, 4) - 43560.9157 * pow(voltage, 3) + 252848.5888 * pow(voltage, 2) - 650767.4615 * voltage + 626532.5703;
  if (voltage > 4.19) percentage = 100;
  else if (voltage <= 3.50) percentage = 0;
  return String(percentage)+"%";
}
