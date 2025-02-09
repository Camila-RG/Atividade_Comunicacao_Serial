// Bibliotecas necessárias para funcionamento
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definição de constantes da matriz de leds
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

// Variáveis globais para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 255;
uint8_t led_g = 0;
uint8_t led_b = 127;

// Variáveis de controle de estado
static volatile int numero_atual = 0; // Número exibido na matriz de LEDs
static uint32_t last_interrupt_time_a = 0; // Armazena o último tempo de interrupção do botão A
static uint32_t last_interrupt_time_b = 0; // Armazena o último tempo de interrupção do botão B

// Definição dos pinos dos leds e botões
#define LED_G_PIN 11
#define LED_B_PIN 12
#define LED_R_PIN 13

#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Função para inicializar os LEDs e botões(Componentes)
void inicializacaocomponentes() {
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_put(LED_G_PIN, 0);

    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
    gpio_put(LED_B_PIN, 0);

    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_put(LED_R_PIN, 0);

    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
}

// Interrupção dos botões com debouncing
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN && current_time - last_interrupt_time_a > 300000){ //Tempo de Debounce: 300 ms
        last_interrupt_time_a = current_time;
        gpio_get(!LED_G_PIN); // Altera o estado do LED verde entre ligado e desligado
    }
    else if (gpio == BUTTON_B_PIN && current_time - last_interrupt_time_b > 300000) { //Tempo de Debounce: 300 ms
        last_interrupt_time_b = current_time;
        gpio_get(!LED_B_PIN); // Altera o estado do LED azul entre ligado e desligado
    }
}

int main()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    
    stdio_init_all();
    inicializacaocomponentes(); //Inicialização dos pinos dos LEDs e botões
        
    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true)
    {
    }
    return 0;
}