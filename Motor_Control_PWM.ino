/*
 * Controle de Motor DC com Encoder + TB6612FNG + LCD I2C + Controle PID
 * Hardware: Arduino Mega UNO
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pinos do Motor ---
const int PWMA = 9;
const int AIN1 = 7;
const int AIN2 = 8;

// --- Pinos do Encoder ---
const int ENC_C1 = 2;   
const int ENC_C2 = 3;   

const int POT_PIN = A0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Variáveis do Encoder ---
volatile long pulseCount = 0;   
unsigned long lastTime = 0;
float rpm = 0.0;
const float PULSES_PER_REVOLUTION = 1496.0;

unsigned long lastLCDTime = 0;

// --- Variáveis do PID ---
float setpointRPM = 0.0;  // RPM desejado (definido pelo potenciômetro)
float inputRPM = 0.0;     // RPM medido atual
float outputPWM = 0.0;    // Sinal de comando para o motor (0 a 255)

// Ganhos do PID
float Kp = 1.1;           // Proporcional: Corrige o erro presente imediato
float Ki = 1.3;           // Integral: Corrige erros acumulados no tempo (força estática)
float Kd = 0.01;           // Derivativo: Amortece a resposta para não passar do ponto

float error = 0.0;
float lastError = 0.0;
float integral = 0.0;
float derivative = 0.0;

const unsigned long PID_INTERVAL = 50; // Roda o cálculo a cada 50ms (20 vezes por segundo)

void setup() {
  Serial.begin(9600);
  
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  
  pinMode(ENC_C1, INPUT_PULLUP);
  pinMode(ENC_C2, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(ENC_C1), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_C2), encoderISR, CHANGE);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   PID Ativo...");
  delay(1000);
  
  lastTime = millis();
}

void encoderISR() {
  pulseCount++;
}

// Executa o cálculo de velocidade e o controle PID no mesmo intervalo fixo
void controladorPID() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastTime;
  
  if (elapsedTime >= PID_INTERVAL) {
    // Calcula o RPM atual baseado na janela de 50ms
    float frequency = (float)pulseCount / (elapsedTime / 1000.0);
    rpm = (frequency * 60.0) / PULSES_PER_REVOLUTION;
    inputRPM = rpm;
    
    // Reseta contagem do encoder imediatamente para a próxima janela
    pulseCount = 0;
    lastTime = currentTime;
    
    // Algoritmo PID
    error = setpointRPM - inputRPM;
    
    // Termo Integral com Anti-Windup (evita que o Ki acumule infinitamente se o motor travar)
    integral += error * (PID_INTERVAL / 1000.0);
    if (integral > 100) integral = 100;
    else if (integral < -100) integral = -100;
    
    // Termo Derivativo
    derivative = (error - lastError) / (PID_INTERVAL / 1000.0);
    
    // Soma tudo para gerar a correção
    outputPWM = (Kp * error) + (Ki * integral) + (Kd * derivative);
    
    // Limita a saída para os níveis físicos permitidos pelo sinal PWM do Arduino (0-255)
    if (outputPWM > 255) outputPWM = 255;
    if (outputPWM < 0) outputPWM = 0;
    
    // Salva o erro para o próximo ciclo derivativo
    lastError = error;

    // Aplica a velocidade corrigida no motor (Sentido horário fixo)
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, (int)outputPWM);
  }
}

void loop() {
  // Lê o potenciômetro
  int potValue = analogRead(POT_PIN);
  
  // Se o potenciômetro estiver quase no zero (ruído menor que 10), força o alvo para 0
  if (potValue < 10) {
    setpointRPM = 0;
  } else {
    // Começa em 10 RPM para evitar que o motor fique tentando girar sem força
    setpointRPM = map(potValue, 10, 1023, 10, 300);
  }
  
  // Executa o cálculo e correção do PID (controlado internamente a cada 50ms)
  controladorPID();
  
  // Janela de atualização do LCD
  unsigned long currentTime = millis();
  if (currentTime - lastLCDTime >= 350) { // 350ms é para o LCD não borrar
    
    lcd.setCursor(0, 0);
    lcd.print("Alvo: ");
    lcd.print((int)setpointRPM);
    lcd.print(" RPM   "); // Espaços limpam sobras de dígitos antigos
    
    lcd.setCursor(0, 1);
    lcd.print("Real: ");
    lcd.print((int)rpm);
    lcd.print(" RPM   ");
    
    lastLCDTime = currentTime; // Reseta o cronômetro do LCD
  }
  
  // Log Serial formatado para o Plotter (O Trace)
  Serial.print("Alvo_RPM:");
  Serial.print(setpointRPM);
  Serial.print(" "); // espaço separa as variáveis no Plotter
  
  Serial.print("Real_RPM:");
  Serial.print(rpm);
  Serial.print(" ");
  
  Serial.print("Erro:");
  Serial.print(setpointRPM - rpm);
  Serial.println(); // O println final indica o fim da linha de dados atual
}