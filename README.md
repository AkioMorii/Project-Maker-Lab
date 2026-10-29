# Integrantes:
David de Medeiros RM551462;
Kaique Maia, RM552112;
Orlando Akio, RM98067;
Pedro Henrique Alves Guariente, RM550301;

# Carrinho-Robô Bluetooth com Sensor Ultrassônico

Projeto de Maker Lab (FIAP): carrinho-robô controlado pelo celular via Bluetooth, com um sensor ultrassônico que para o carrinho automaticamente quando há um obstáculo à frente.

## Funcionalidades

- Controle pelo celular: frente, ré, giro para a esquerda/direita e diagonais
- 10 níveis de velocidade (0 a 9), além de um comando de velocidade máxima
- Parada automática quando há obstáculo a menos de 20 cm à frente
- Comandos recebidos e distância medida exibidos no Monitor Serial

## Componentes

| Componente | Qtd. | Função |
|---|---|---|
| Arduino Uno (clone com chip CH340) | 1 | Microcontrolador |
| Driver de motor L298N (HW-095) | 1 | Controle dos dois motores |
| Módulo Bluetooth HC-05 | 1 | Comunicação com o celular |
| Sensor ultrassônico HC-SR04 | 1 | Medição de distância |
| Motor DC com roda | 2 | Tração |
| Chassi em acrílico | 1 | Estrutura |
| Mini protoboard | 1 | Ligações |
| Resistores de 1 kΩ e 2 kΩ | 1 de cada | Divisor de tensão do Bluetooth |
| Bateria / suporte de pilhas | 1 | Alimentação dos motores |
| Jumpers | — | Ligações |

## Ligações

### Driver L298N

| L298N | Ligação |
|---|---|
| IN1 | Arduino pino 11 |
| IN2 | Arduino pino 10 |
| IN3 | Arduino pino 9 |
| IN4 | Arduino pino 8 |
| ENA / ENB | Jumpers **colocados** |
| OUT1 / OUT2 | Motor esquerdo |
| OUT3 / OUT4 | Motor direito |
| 12V | Positivo da bateria |
| GND | Negativo da bateria **e** GND do Arduino |

### Módulo Bluetooth HC-05

| HC-05 | Arduino |
|---|---|
| VCC | 5V |
| GND | GND |
| TXD | Pino 7 |
| RXD | Pino 6 (pelo divisor de tensão) |
| STATE / EN | Não conectados |

O RXD do HC-05 trabalha em 3,3 V, por isso o sinal de 5 V do Arduino passa por um divisor de tensão:

```
Arduino pino 6 ──[ 1 kΩ ]──┬── RXD do HC-05
                           │
                       [ 2 kΩ ]
                           │
                          GND
```

### Sensor ultrassônico HC-SR04

| HC-SR04 | Arduino |
|---|---|
| VCC | 5V |
| TRIG | Pino 3 |
| ECHO | Pino 2 |
| GND | GND |

> **Importante:** o GND da bateria, do L298N e do Arduino precisam estar ligados entre si.

## Comandos Bluetooth

O carrinho aceita letras maiúsculas e minúsculas.

| Caractere | Ação |
|---|---|
| `F` | Frente |
| `B` | Ré |
| `L` | Gira para a esquerda |
| `R` | Gira para a direita |
| `G` | Frente-esquerda |
| `I` | Frente-direita |
| `H` | Ré-esquerda |
| `J` | Ré-direita |
| `S` / `D` | Parar |
| `0` a `9` | Velocidade |
| `q` | Velocidade máxima |

Outros caracteres enviados pelo app (luzes, buzina etc.) são ignorados.

| Tecla | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|---|
| PWM | 0 | 80 | 100 | 120 | 140 | 160 | 180 | 200 | 225 | 255 |

Velocidades abaixo de 6 podem não ter força para tirar o carrinho do lugar.

## Como funciona

**Motores.** Como não há pinos livres para ENA/ENB, os jumpers do L298N ficam colocados e a velocidade é controlada por PWM direto nos pinos IN. O pino 8 não tem PWM: na ré do motor direito, ele fica em HIGH e o PWM do pino 9 é invertido (`255 − velocidade`).

**Sensor.** A distância é lida a cada 100 ms. Se o carrinho estiver indo para a frente (`F`, `G` ou `I`) e duas leituras seguidas ficarem abaixo de `DISTANCIA_MINIMA`, ele para e envia `Obstaculo!` pelo Bluetooth. Enquanto o obstáculo estiver lá, os comandos de frente são ignorados; ré e giros continuam liberados.

**Ajustes no código:**

| Variável | Padrão | Descrição |
|---|---|---|
| `velocidade` | 180 | Velocidade inicial (0 a 255) |
| `DISTANCIA_MINIMA` | 20 | Distância de parada em cm (`0` desativa) |
| `TABELA_VELOCIDADE` | — | PWM de cada tecla de 0 a 9 |

## Como gravar o código

1. Instale a [Arduino IDE](https://www.arduino.cc/en/software).
2. Abra `carrinho_bluetooth/carrinho_bluetooth.ino`.
3. Em **Tools → Board**, selecione **Arduino Uno**.
4. Em **Tools → Port**, selecione a porta **USB-SERIAL CH340**. A COM1 é a porta interna do computador, não o Arduino.
5. Clique em **Upload** e aguarde "Done uploading".
6. Abra o **Serial Monitor** em **9600 baud**. Deve aparecer `Robo Bluetooth pronto!`.

> Se nenhuma porta do Arduino aparecer, instale o driver **CH340** (arquivo `CH341SER.EXE`, no site da WCH) ou teste outro cabo USB, já que alguns só carregam e não transmitem dados.

## Como usar

1. Ligue a bateria do carrinho.
2. Em um celular **Android**, pareie com o **HC-05** nas configurações de Bluetooth (senha `1234` ou `0000`). O iPhone não é compatível com o HC-05.
3. Instale o app **Arduino Bluetooth RC Car** e conecte ao HC-05 pelo app.
4. Faça o primeiro teste com as rodas no ar e depois coloque o carrinho no chão.

## Solução de problemas

| Problema | Causa provável |
|---|---|
| Upload trava ou dá "not in sync" | Porta COM errada ou driver CH340 ausente |
| Nada aparece no Monitor Serial ao usar o app | TXD/RXD do HC-05 invertidos ou app não conectado |
| Comandos chegam, mas as rodas não giram | Bateria fraca, jumpers ENA/ENB ausentes ou GND não comum |
| Uma roda gira ao contrário | Inverter os dois fios desse motor no borne do L298N |
| Esquerda e direita trocadas | Trocar os motores de borne (OUT1/OUT2 ↔ OUT3/OUT4) |
| Distância sempre `-1` | Fios de TRIG e ECHO trocados |
| Arduino reinicia quando os motores arrancam | Bateria fraca ou alimentação insuficiente |

## Estrutura do repositório

```
.
├── README.md
└── carrinho_bluetooth/
    └── carrinho_bluetooth.ino
```

## Integrantes

| Nome | RM |
|---|---|
| Nome do integrante | RM00000 |
| Nome do integrante | RM00000 |
| Nome do integrante | RM00000 |
