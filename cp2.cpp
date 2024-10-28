#include <LiquidCrystal.h>
#include <RTClib.h> // Biblioteca para Relógio em Tempo Real
#include <Wire.h>   // Biblioteca para comunicação I2C
#include <EEPROM.h>
#include "DHT.h"
#define LOG_OPTION 1     // Opção para ativar a leitura do log
#define SERIAL_OPTION 0  // Opção de comunicação serial: 0 para desligado, 1 para ligado
#define UTC_OFFSET -3    // Ajuste de fuso horário para UTC-3

// Configurações do DHT22 e RTC
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 RTC;

// Configurações da EEPROM
const int maxRecords = 100;
const int recordSize = 8; // Tamanho de cada registro em bytes
int startAddress = 0;
int endAddress = maxRecords * recordSize;
int currentAddress = 0;

int lastLoggedMinute = -1;

// Triggers de temperatura e umidade
float trigger_t_min = 20.0; // Exemplo: valor mínimo de temperatura
float trigger_t_max = 30.0; // Exemplo: valor máximo de temperatura
float trigger_u_min = 30.0; // Exemplo: valor mínimo de umidade
float trigger_u_max = 60.0; // Exemplo: valor máximo de umidade

// Setando os pinos do LCD:

LiquidCrystal lcd(13, 12, 11, 10, 9, 8, 7);

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
int ledVerm = 6; // Pino led vermelho.
int ledAma = 5; // Pino led amarelo.
int ledVer = 4; // Pino led verde.
int buzzer = 3; // Pino buzzer.

void setup() {
  dht.begin();
  Serial.begin(9600); // Inicia a comunicação serial.
  pinMode(pinoLDR, INPUT); // Define o pino do LDR como entrada.
  lcd.begin(16, 2);
  RTC.begin();    // Inicialização do Relógio em Tempo Real
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // RTC.adjust(DateTime(2024, 5, 6, 08, 15, 00));  // Ajustar a data e hora apropriadas uma vez inicialmente, depois comentar
  EEPROM.begin();

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
      DateTime now = RTC.now();

    // Calculando o deslocamento do fuso horário
    int offsetSeconds = UTC_OFFSET * 3600; // Convertendo horas para segundos
    now = now.unixtime() + offsetSeconds; // Adicionando o deslocamento ao tempo atual

    // Convertendo o novo tempo para DateTime
    DateTime adjustedTime = DateTime(now);

    if (LOG_OPTION) get_log();

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
    intensidadeLuz = map(valorLDR, 8, 1015, 100, 0); // Função map pega o valor bruto do sensor LDR e coloca em uma escala de 0 a 100.
    
    Serial.print("Intensidade de Luz: "); // Printa no monitor serial a intensidade da luz em uma escala de 0 a 100, e o valor bruto do sensor LDR
    Serial.println(intensidadeLuz);
    Serial.println(valorLDR);
  }
    // Verifica se o minuto atual é diferente do minuto do último registro
    if (adjustedTime.minute() != lastLoggedMinute) {
        lastLoggedMinute = adjustedTime.minute();

        // Ler os valores de temperatura e umidade
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();
        float luz = intensidadeLuz;

        // Verificar se os valores estão fora dos triggers
        if (temperature < trigger_t_min || temperature > trigger_t_max || humidity < trigger_u_min || humidity > trigger_u_max) {
            // Converter valores para int para armazenamento
            int tempInt = (int)(temperature * 100);
            int humiInt = (int)(humidity * 100);

            // Escrever dados na EEPROM
            EEPROM.put(currentAddress, now.unixtime());
            EEPROM.put(currentAddress + 4, tempInt);
            EEPROM.put(currentAddress + 6, humiInt);

            // Atualiza o endereço para o próximo registro
            getNextAddress();
        }
    }

    lcd.setCursor(0, 0);
    lcd.print("DATA: ");
    lcd.print(adjustedTime.day() < 10 ? "0" : ""); // Adiciona zero à esquerda se dia for menor que 10
    lcd.print(adjustedTime.day());
    lcd.print("/");
    lcd.print(adjustedTime.month() < 10 ? "0" : ""); // Adiciona zero à esquerda se mês for menor que 10
    lcd.print(adjustedTime.month());
    lcd.print("/");
    lcd.print(adjustedTime.year());
    lcd.setCursor(0, 1);
    lcd.print("HORA: ");
    lcd.print(adjustedTime.hour() < 10 ? "0" : ""); // Adiciona zero à esquerda se hora for menor que 10
    lcd.print(adjustedTime.hour());
    lcd.print(":");
    lcd.print(adjustedTime.minute() < 10 ? "0" : ""); // Adiciona zero à esquerda se minuto for menor que 10
    lcd.print(adjustedTime.minute());
    lcd.print(":");
    lcd.print(adjustedTime.second() < 10 ? "0" : ""); // Adiciona zero à esquerda se segundo for menor que 10
    lcd.print(adjustedTime.second());

    delay(1000);


  
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

void getNextAddress() {
    currentAddress += recordSize;
    if (currentAddress >= endAddress) {
        currentAddress = 0; // Volta para o começo se atingir o limite
    }
}

void get_log() {
    Serial.println("Data armazenado no EEPROM:");
    Serial.println("Horário\t\t\tTemperatura\tUmidade");

    for (int address = startAddress; address < endAddress; address += recordSize) {
        long timeStamp;
        int tempInt, humiInt, luzInt;

        // Ler dados da EEPROM
        EEPROM.get(address, timeStamp);
        EEPROM.get(address + 4, tempInt);
        EEPROM.get(address + 6, humiInt);
        EEPROM.get(address + 8, luzInt);

        // Converter valores
        float temperature = tempInt / 100.0;
        float humidity = humiInt / 100.0;

        // Verificar se os dados são válidos antes de imprimir
        if (timeStamp != 0xFFFFFFFF) { // 0xFFFFFFFF é o valor padrão de uma EEPROM não inicializada
            //Serial.print(timeStamp);
            DateTime dt = DateTime(timeStamp);
            Serial.print(dt.timestamp(DateTime::TIMESTAMP_FULL));
            Serial.print("\t");
            Serial.print(temperature);
            Serial.print(" C\t\t");
            Serial.print(humidity);
            Serial.println(" %");
        }
    }
}
