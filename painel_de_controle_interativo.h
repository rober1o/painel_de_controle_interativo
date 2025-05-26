#ifndef PAINEL_DE_CONTROLE_INTERATIVO // Previne múltiplas inclusões do cabeçalho
#define PAINEL_DE_CONTROLE_INTERATIVO

#include <stdio.h>
#include <stdint.h> // Para tipos uint32_t e uint8_t

// Bibliotecas do Raspberry Pi Pico
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"

// Bibliotecas para displays e fontes
#include "lib/ssd1306.h"
#include "lib/font.h"


// Bibliotecas para FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Definições de pinos e parâmetros
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define BUZZER_A 10 // Pino conectado ao buzzer
#define DEBOUCE 200


// Definições dos LEDs
#define LED_VERDE 11
#define LED_VERMELHO 13
#define LED_AZUL 12

// definições de botão

#define BOTAO_A 5
#define BOTAO_B 6
#define BOTAO_C 22

ssd1306_t ssd; // display ssd
uint slice_buzzer;
int MAX_USUARIOS = 6;

static volatile uint32_t ultimo_tempo = 0;
const uint32_t Debouce_botao_C = 200;  // Tempo de debounce em milissegundos

// Semáforos e mutex
SemaphoreHandle_t xSemaphoreUsuarios;
SemaphoreHandle_t xSemaphoreReset;
SemaphoreHandle_t xMutexDisplay;

// DECLARAÇÃO FUNÇÕES DE INICALIZAÇÃO E CALLBACK
void atualizar_display(int valor_contagem);
void inicializar_botoes();
void gpio_callback(uint gpio, uint32_t events);
void inicializar_pwms_buzzer();
void inicializar_leds();
void alertas_leds(int num_usuarios);
// DECLARAÇÃO DAS TAREFAS
void vEntrada();
void vSaida();
void vReset();



#endif // PAINEL_DE_CONTROLE_INTERATIVO