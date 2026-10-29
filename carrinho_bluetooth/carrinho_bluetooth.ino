#include <SoftwareSerial.h>

// ================== PINOS ==================

// Bluetooth HC-05: SoftwareSerial(RX do Arduino, TX do Arduino)
// Pino 7 <- TXD do modulo | Pino 6 -> RXD do modulo (com divisor de tensao)
SoftwareSerial bluetooth(7, 6);

// Driver L298N (jumpers de ENA e ENB COLOCADOS)
const int IN1 = 11; // Motor A (esquerdo) - PWM
const int IN2 = 10; // Motor A (esquerdo) - PWM
const int IN3 = 9;  // Motor B (direito)  - PWM
const int IN4 = 8;  // Motor B (direito)  - sem PWM

// Sensor ultrassonico HC-SR04
const int TRIG = 3;
const int ECHO = 2;

// ================== AJUSTES ==================

int velocidade = 180;             // 0 a 255
const int DISTANCIA_MINIMA = 20;  // cm. Coloque 0 para desativar a parada automatica

// Velocidades enviadas pelo app (teclas 0 a 9)
const int TABELA_VELOCIDADE[10] = {0, 80, 100, 120, 140, 160, 180, 200, 225, 255};

// ================== ESTADO ==================

char estado = 'S';               // movimento atual
long distancia = -1;             // ultima leitura em cm (-1 = sem eco)
int leiturasPerto = 0;           // leituras seguidas abaixo do limite
unsigned long ultimaLeitura = 0;

// ================== MOTORES ==================

// v de -255 (re) a 255 (frente)
void motorA(int v) {
  if (v > 0) {
    analogWrite(IN1, v);
    digitalWrite(IN2, LOW);
  } else if (v < 0) {
    digitalWrite(IN1, LOW);
    analogWrite(IN2, -v);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}

void motorB(int v) {
  if (v > 0) {
    analogWrite(IN3, v);
    digitalWrite(IN4, LOW);
  } else if (v < 0) {
    // O pino 8 nao tem PWM: deixa ele em HIGH e inverte o PWM do pino 9
    digitalWrite(IN4, HIGH);
    analogWrite(IN3, 255 + v);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
}

void mover(char direcao) {
  int v = velocidade;
  int meia = velocidade / 2;

  switch (direcao) {
    case 'F': motorA(v);     motorB(v);     break; // frente
    case 'B': motorA(-v);    motorB(-v);    break; // re
    case 'L': motorA(-v);    motorB(v);     break; // gira para a esquerda
    case 'R': motorA(v);     motorB(-v);    break; // gira para a direita
    case 'G': motorA(meia);  motorB(v);     break; // frente-esquerda
    case 'I': motorA(v);     motorB(meia);  break; // frente-direita
    case 'H': motorA(-meia); motorB(-v);    break; // re-esquerda
    case 'J': motorA(-v);    motorB(-meia); break; // re-direita
    default:
      direcao = 'S';
      motorA(0);
      motorB(0);
      break;
  }
  estado = direcao;
}

bool indoParaFrente() {
  return estado == 'F' || estado == 'G' || estado == 'I';
}

bool obstaculoNaFrente() {
  // Exige 2 leituras seguidas para evitar paradas falsas
  return DISTANCIA_MINIMA > 0 && leiturasPerto >= 2;
}

// ================== SENSOR ==================

long lerDistanciaCm() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH, 25000); // espera no maximo ~4 m
  if (duracao == 0) return -1;               // sem eco: nada a frente
  return duracao / 58;                       // microssegundos -> cm
}

// ================== SETUP ==================

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  Serial.begin(9600);
  bluetooth.begin(9600);

  mover('S');

  Serial.println("Robo Bluetooth pronto!");
  bluetooth.println("Robo pronto!");
}

// ================== LOOP ==================

void loop() {
  // 1) Comandos recebidos pelo Bluetooth
  if (bluetooth.available()) {
    char comando = toupper(bluetooth.read());

    if (comando != '\n' && comando != '\r') {
      Serial.print("Comando: ");
      Serial.println(comando);

      if (comando >= '0' && comando <= '9') {
        velocidade = TABELA_VELOCIDADE[comando - '0'];
        mover(estado); // mantem a direcao com a nova velocidade
      } else {
        switch (comando) {
          case 'F':
          case 'G':
          case 'I':
            // So avanca se nao houver obstaculo perto
            if (obstaculoNaFrente()) {
              mover('S');
            } else {
              mover(comando);
            }
            break;

          case 'B':
          case 'L':
          case 'R':
          case 'H':
          case 'J':
            mover(comando);
            break;

          case 'S':
          case 'D':
            mover('S');
            break;

          case 'Q': // velocidade maxima no app
            velocidade = 255;
            mover(estado);
            break;

          default:
            // Luzes, buzina etc.: ignora sem responder
            break;
        }
      }
    }
  }

  // 2) Leitura do sensor a cada 100 ms
  if (millis() - ultimaLeitura >= 100) {
    ultimaLeitura = millis();
    distancia = lerDistanciaCm();

    if (distancia > 0 && distancia < DISTANCIA_MINIMA) {
      if (leiturasPerto < 10) leiturasPerto++;
    } else {
      leiturasPerto = 0;
    }

    // Para sozinho se estiver indo para a frente e houver obstaculo
    if (indoParaFrente() && obstaculoNaFrente()) {
      mover('S');
      Serial.println("Obstaculo! Parado.");
      bluetooth.println("Obstaculo!");
    }

    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }
}
