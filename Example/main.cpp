/**
 * @file main.cpp
 * @author Evandro Teixeira
 * @brief 
 * @version 0.1
 * @date 06-03-2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include "MatrixKeyboard.hpp"
#include "Buzzer/buzzer.hpp"

#define COLOR_BLACK         "\e[0;30m"
#define COLOR_RED           "\e[0;31m"
#define COLOR_GREEN         "\e[0;32m"
#define COLOR_YELLOW        "\e[0;33m"
#define COLOR_BLUE          "\e[0;34m"
#define COLOR_PURPLE        "\e[0;35m"
#define COLOR_CYAN          "\e[0;36m"
#define COLOR_WRITE         "\e[0;37m"
#define COLOR_RESET         "\e[0m"
#define BUZZER_FREQUENCY        30                  // Frequency 2000 Hz
#define BUZZER_DUTYCYCLE        75                  // Init DutyCycle 50%
#define BUZZER_CONFIG           (BuzzerPin_t){15,4} // Pin, Channel PWM
#define SW_TIMER_PERIOD_BUZZER  500                 // 500 ms
#define KEY_ALL_BITS            0x0000FFFF
#define BAUDE_RATE              115200              

typedef struct 
{
  BuzzerNote_t Note;
  uint8_t Octave;
  char StringNote[8];
}BuzzerApp_t; 

const BuzzerApp_t NoteBuzzer[16] = {
  {Buzzer_Note_A,0,"Nota A"},
  {Buzzer_Note_A,4,"Nota A"},
  {Buzzer_Note_B,0,"Nota B"},
  {Buzzer_Note_B,4,"Nota B"},
  {Buzzer_Note_C,0,"Nota C"},
  {Buzzer_Note_C,4,"Nota C"},
  {Buzzer_Note_D,0,"Nota D"},
  {Buzzer_Note_D,4,"Nota D"},
  {Buzzer_Note_E,0,"Nota E"},
  {Buzzer_Note_E,4,"Nota E"},
  {Buzzer_Note_F,0,"Nota F"},
  {Buzzer_Note_F,4,"Nota F"},
  {Buzzer_Note_G,0,"Nota G"},
  {Buzzer_Note_G,4,"Nota G"},
  {Buzzer_Note_Gs,0,"Nota Gs"},
  {Buzzer_Note_Gs,4,"Nota Gs"},
};

/**
 * @brief Cria objeto Buzzer
 * @return buzzer 
 */
buzzer Buzzer(BUZZER_CONFIG);
/**
 * @brief Cria objeto Teclado
 * @return MatrixKeyboard 
 */
MatrixKeyboard Teclado(Key4x4);
EventGroupHandle_t xEventGroupKey;
TimerHandle_t TimerBuzzer;

/**
 * @brief 
 * @param parameters 
 */
void Tarefa_Teclado(void *parameters);
void Tarefa_Buzzer(void *parameters);
void Callback_TimerBuzzer(TimerHandle_t timer);

void setup() 
{
  KeyboardPin_t ConfigPin;
  ConfigPin.line1 = 16;
  ConfigPin.line2 = 17;
  ConfigPin.line3 = 18;
  ConfigPin.line4 = 32;
  ConfigPin.column1 = 33;
  ConfigPin.column2 = 13;
  ConfigPin.column3 = 2;
  ConfigPin.column4 = 4;

  // Inicializa Serial
  Serial.begin(BAUDE_RATE);
  Serial.printf("\n\rFreeRTOS - Event Groups\n\r");

  // Inicializa o Buzzer 
  Buzzer.begin(BUZZER_FREQUENCY,BUZZER_DUTYCYCLE);
  Buzzer.stop();

  // Inicializa Teclado
  Teclado.Init(ConfigPin);

  // Cria Event Group
  xEventGroupKey = xEventGroupCreate();
  if(xEventGroupKey == NULL)
  {
    Serial.printf("\n\rFalha em criar a Event Group xEventGroupKey");
  }

  // Cria SoftwareTimer  TimerBuzzer
  TimerBuzzer = xTimerCreate("TIMER_PULSO",pdMS_TO_TICKS(SW_TIMER_PERIOD_BUZZER),pdFALSE,NULL,Callback_TimerBuzzer);
  if(TimerBuzzer == NULL)
  {
    Serial.printf("\n\rFalha em criar SW_Timer TimerBuzzer");
  }

  // Cria tarefas da aplicação
  xTaskCreate(Tarefa_Teclado, "TECLADO", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, NULL);
  xTaskCreate(Tarefa_Buzzer, "BUZZER", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void loop() 
{
  Serial.printf("\n\rSupende tarefa LOOP");
  vTaskSuspend(NULL);
}

/**
 * @brief 
 * @param parameters 
 */
void Tarefa_Teclado(void *parameters)
{
  uint16_t keyOld = 0xFFFF;
  static Key_t Key;
  char ch;

  while(1)
  {
    // Executa rotina do teclado
    Teclado.Run();
    // Le as informações do teclado
    Key = Teclado.Read();
    ch = Teclado.GetCharKey();

    if(keyOld != Key.data)
    {
      if(Key.data != 0)
      {
        Serial.print(COLOR_RED);
        Serial.printf("\n\rInformações do teclado: %d | Tecla: %c ",Key.data, ch);
        Serial.printf("\n\rDefine Bit em xEventGroupKey %04X",Key.data);
        Serial.print(COLOR_RESET);

        xEventGroupSetBits(xEventGroupKey,(EventBits_t)(Key.data));
      }
    }
    // update
    keyOld = Key.data;

    vTaskDelay(50/portTICK_PERIOD_MS);
  }
}

/**
 * @brief 
 * @param parameters 
 */
void Tarefa_Buzzer(void *parameters)
{
  EventBits_t keyBits;
  uint16_t value = 0x0001;
  uint8_t i = 0;

  while(1)
  { 
    /* Le os bits xEventGroupKey */
    keyBits = xEventGroupWaitBits(xEventGroupKey, (EventBits_t)(KEY_ALL_BITS), true, true, pdMS_TO_TICKS(1000) );

    /* Busca qual bit esta definido */
    for(i=0;i<16;i++)
    {
      value = 1 << i;
      if(keyBits & value)
      {
        Serial.print(COLOR_YELLOW);
        Serial.printf("\n\rkeyBits:%5d - %s Octave: %d",keyBits,NoteBuzzer[i].StringNote,NoteBuzzer[i].Octave);
        Serial.printf("\n\rLiga Buzzer | Inicia TimerBuzzer | Limpa bit xEventGroupKey "); 
        Serial.print(COLOR_RESET);

        // Liga Buzzer por 500 millessegundos
        Buzzer.start();
        Buzzer.note(NoteBuzzer[i].Note, NoteBuzzer[i].Octave);
        
        // Inicia TimerBuzzer com 500 millessegundos
        xTimerStart(TimerBuzzer, 0 );

        // Limpa bit xEventGroupKey
        xEventGroupClearBits(xEventGroupKey, value);
        break;
      }
    }
  }
}

/**
 * @brief 
 * @param timer 
 */
void Callback_TimerBuzzer(TimerHandle_t timer)
{
  if(timer == TimerBuzzer)
  {
    // Encera o TimerBuzzer
    xTimerStopFromISR(TimerBuzzer, pdFALSE);
    // Desliga Buzzer
    Buzzer.stop();

    Serial.print(COLOR_BLUE);
    Serial.printf("\n\rDesliga Buzzer | Encera o TimerBuzzer "); 
    Serial.print(COLOR_RESET);
  }
}