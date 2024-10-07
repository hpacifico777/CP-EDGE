#include <LiquidCrystal.h>
// Setando os pinos do LCD:

LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);

// Fazendo a configuração do logo da empresa:
 byte SIMB1[8] = {
B00000,
B01111,
B01000,
B01011,
B01010,
B01010,
B01010,
B01010
};
 byte SIMB2[8] = {
B00000,
B11111,
B00000,
B10101,
B00101,
B00101,
B00101,
B00111
};

 byte SIMB3[8] = {
B00000,
B11111,
B00000,
B10101,
B11101,
B11101,
B11101,
B10101
};

 byte SIMB4[8] = {
B00000,
B11110,
B00010,
B01010,
B01010,
B01010,
B01010,
B01010
};

 byte SIMB5[8] = {
B01010,
B01010,
B01011,
B01000,
B01111,
B00000,
B00000,
B00000
};

 byte SIMB6[8] = {
B00101,
B00101,
B10101,
B00000,
B11111,
B00001,
B00111,
B00111
};

 byte SIMB7[8] = {
B10101,
B10101,
B10100,
B00000,
B11111,
B10000,
B11100,
B11100
};

 byte SIMB8[8] = {
B01010,
B01010,
B10010,
B00010,
B11110,
B00000,
B00000,
B00000
};

// Setando pino analógico 0 para ler o sensor LDR:

const int pinoLDR = A0;

// Setando os pinos e as váriaveis:
int valorLDR; // Valor recebido do sensor LDR.
int intensidadeLuz; // Pega o valor do sensor LDR e transforma numa escala de 0 a 100.
int mediaMovel[10]; // Array para média móvel.
int contagemLeituras = 0; // Contador de leituras.
int ledAma = 8; // Pino led amarelo.
int ledVerm = 9; // Pino led vermelho.
int ledVer = 7; // Pino led verde.
int buzzer = 6; // Pino buzzer.

void setup() {
  Serial.begin(9600); // Inicia a comunicação serial.
  pinMode(pinoLDR, INPUT); // Define o pino do LDR como entrada.
  lcd.begin(16, 2);

  // Criar os caracteres personalizados para o logo:
  
  lcd.createChar(1, SIMB1);
  lcd.createChar(2, SIMB2);
  lcd.createChar(3, SIMB3);
  lcd.createChar(4, SIMB4);
  lcd.createChar(5, SIMB5);
  lcd.createChar(6, SIMB6);
  lcd.createChar(7, SIMB7);
  lcd.createChar(8, SIMB8);
  
  // Limpa o display e começa a exibir o logo:
  
  lcd.clear();
  
  // Exibir o logo utilizando os caracteres criados:
  
  lcd.setCursor(0, 0);
  lcd.write(byte(1)); // SIMB1
  lcd.write(byte(2)); // SIMB2
  lcd.write(byte(3)); // SIMB3
  lcd.write(byte(4)); // SIMB4

  lcd.setCursor(0, 1);
  lcd.write(byte(5)); // SIMB5
  lcd.write(byte(6)); // SIMB6
  lcd.write(byte(7)); // SIMB7
  lcd.write(byte(8)); // SIMB8
  
  // Adicionar texto logo após a imagem do logo na segunda linha: 
  lcd.setCursor(4, 0);
  lcd.print("    CHMV");
  lcd.setCursor(4, 1);
  lcd.print("    TECH");

  // Setando os leds e buzzer como saída:
  pinMode(ledAma, OUTPUT);
  pinMode(ledVerm, OUTPUT);
  pinMode(ledVer, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  // Tempo para exibir o logo antes de começar o loop:
  
  delay(4000);
}
void loop() {
  valorLDR = analogRead(pinoLDR);
  
  // Atualiza a média móvel:
  
  mediaMovel[contagemLeituras % 10] = valorLDR; // Substitui a leitura mais antiga
  contagemLeituras++;

  // Calcula a média a cada 10 leituras do sensor:
  
  if (contagemLeituras >= 10) {
    int soma = 0;
    for (int i = 0; i < 10; i++) {
      soma += mediaMovel[i];
    }
    valorLDR = soma / 10;
    intensidadeLuz = map(valorLDR, 49, 969, 100, 0); // Função map pega o valor bruto do sensor LDR e coloca em uma escala de 0 a 100.
    
    Serial.print("Intensidade de Luz: "); // Printa no monitor serial a intensidade da luz em uma escala de 0 a 100, e o valor bruto do sensor LDR
    Serial.println(intensidadeLuz);
    Serial.println(valorLDR);
  }

  // Atualiza o LCD:
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Intensidade:"); // O que aparecerá no LCD
  lcd.print(intensidadeLuz);
  delay(500); // Atualiza o LCD a cada meio segundo

  // Controle dos LEDs e buzzer:
  
  if (intensidadeLuz <= 20) { // Se a intensidade da luz for menor ou igual a 20, o led verde ficará ligado, sem som no buzzer.
    digitalWrite(ledVer, HIGH);
    digitalWrite(ledAma, LOW);
    digitalWrite(ledVerm, LOW);
  } else if (intensidadeLuz > 20 && intensidadeLuz <= 50) { // Se a intensidade da luz for maior do que 20 e menor ou igual a 50, o led amarelo ficara ligado e o buzzer fará som por 3 segundos.
    digitalWrite(ledVer, LOW);
    digitalWrite(ledAma, HIGH);
    digitalWrite(ledVerm, LOW);
    tone(buzzer, 1500);
    delay(3000);
    noTone(buzzer);
  } else { // Se a intensidade da luz for maior do que 50, o led vermelho ficará ligado e o buzzer fará som continuamente, até a luz ser ajeitada.
    digitalWrite(ledVer, LOW);
    digitalWrite(ledAma, LOW);
    digitalWrite(ledVerm, HIGH);
    tone(buzzer, 3000);
  }
}
