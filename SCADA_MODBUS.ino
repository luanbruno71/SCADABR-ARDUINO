#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Modbus.h>
#include <ModbusSerial.h>
//____________________________________________________
// Posição dos Pinos

#define Potenciometro_Pin A0
#define Temperatura_Pin 2
#define Chave_Pin 4
#define Heat_LED 5 
#define Cool_LED 6 
#define Goal_LED 7
#define Cool_Pin 8 // k1
#define Heat_Pin 9 // k2
#define Air_Pin 10 // k3
#define Pump_Pin 11// k4


//____________________________________________________
// Endereço Modbus

const int Alvo_Hreg = 0;
const int Histerese_Hreg = 1;
const int T1_Ireg = 0;
const int T2_Ireg = 1;
const int T3_Ireg = 2;
const int Chave_Ists = 0;
const int Heat_Coil = 0;
const int Cool_Coil = 1;
const int Pump_Coil = 2;
const int Air_Coil = 3;

//____________________________________________________
// Preparando sensor

OneWire oneWire(Temperatura_Pin);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1, sensor2, sensor3; 
float T1, T2, T3;
int Histerese = 20;

//____________________________________________________
// Preparando Modbus

ModbusSerial mb;
long ts;

//____________________________________________________
// Variavel de controle

int Alvo;

void setup() {
  // Iniciando e identificando sensores
  sensors.begin();
  sensors.getAddress(sensor1, 0);
  sensors.getAddress(sensor2, 1);
  sensors.getAddress(sensor3, 2);
  
  // Iniciando comunicação serial MB:
  mb.config(&Serial, 9600, SERIAL_8N1, 2);

  // Identificador do escravo
  mb.setSlaveId(1);

  // Registrando variáveis MB:
  mb.addHreg(Alvo_Hreg);
  mb.addHreg(Histerese_Hreg);
  mb.addIreg(T1_Ireg);
  mb.addIreg(T2_Ireg);
  mb.addIreg(T3_Ireg);
  mb.addIsts(Chave_Ists);
  mb.addCoil(Heat_Coil);
  mb.addCoil(Cool_Coil);
  mb.addCoil(Pump_Coil);
  mb.addCoil(Air_Coil);

  // Atribuindo função de cada pino
  pinMode(Heat_LED, OUTPUT);
  pinMode(Cool_LED, OUTPUT);
  pinMode(Goal_LED, OUTPUT);  
  
  pinMode(Chave_Pin, INPUT);
  pinMode(Heat_Pin, OUTPUT);
  pinMode(Cool_Pin, OUTPUT);
  pinMode(Pump_Pin, OUTPUT);
  pinMode(Air_Pin, OUTPUT);

  // Sempre inicial com as portas desligadas
  digitalWrite(Heat_Pin, HIGH);
  digitalWrite(Cool_Pin, HIGH);
  digitalWrite(Air_Pin, HIGH);
  digitalWrite(Pump_Pin, HIGH);

  // Preparando Delay
  ts = millis();
}

void loop() {
  //Leitura dos sensores
  sensors.requestTemperatures();

  // Iniciando Modbus
  mb.task();
  if (millis() > ts){
    ts = millis();
    
    if (digitalRead(Chave_Pin)== 1 ){
      Alvo = int(map(analogRead(Potenciometro_Pin), 0, 1023, 15, 80));
    }
    else {
      Alvo = mb.Hreg(Alvo_Hreg);
    }

    Histerese = mb.Hreg(Histerese_Hreg);

    T1 = (sensors.getTempC(sensor1));
    T2 = (sensors.getTempC(sensor2));
    T3 = (sensors.getTempC(sensor3));

    mb.Hreg(Alvo_Hreg, Alvo);
    mb.Ireg(T1_Ireg, 10 * T1);
    mb.Ireg(T2_Ireg, 10 * T2);
    mb.Ireg(T3_Ireg, 10 * T3);
    mb.Ists(Chave_Ists, digitalRead(Chave_Pin));
    digitalWrite(Heat_Pin, !mb.Coil(Heat_Coil));
    digitalWrite(Heat_LED, mb.Coil(Heat_Coil));
    digitalWrite(Cool_Pin, !mb.Coil(Cool_Coil));
    digitalWrite(Cool_LED, mb.Coil(Cool_Coil));
    digitalWrite(Goal_LED, (mb.Coil(Heat_Coil)==LOW)*(mb.Coil(Cool_Coil)==LOW));
    digitalWrite(Pump_Pin, !mb.Coil(Pump_Coil));
    digitalWrite(Air_Pin, !mb.Coil(Air_Coil));
    delay(500);
  }
  
  delay(500);
}

