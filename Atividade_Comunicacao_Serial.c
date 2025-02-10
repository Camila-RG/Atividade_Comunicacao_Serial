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

// Definição para UART
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// Definição para I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Definição de constantes da matriz de leds
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

// Variáveis globais para armazenar a cor
uint8_t led_r = 255;
uint8_t led_g = 0;
uint8_t led_b = 127;

// Variáveis de controle de estado
static volatile int numero_atual = 0;
static uint32_t last_interrupt_time_a = 0;
static uint32_t last_interrupt_time_b = 0;

// Definição dos pinos dos leds e botões
#define LED_G_PIN 11
#define LED_B_PIN 12
#define LED_R_PIN 13

#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

void inicializacaocomponentes(); // Função para inicializar os LEDs e botões(Componentes)

// Buffer para armazenar quais LEDs estão ligados matriz 5x5 e formar os números de 0 a 9
bool led_buffer[10][NUM_PIXELS] = {
    //1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25}
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // Número 0
    {0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0}, // Número 1
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // Número 2
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // Número 3
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1}, // Número 4
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1}, // Número 5
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1}, // Número 6
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1}, // Número 7
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // Número 8
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // Número 9
};
 
//Funções para enviar dados, converter valores e acender os LEDs

static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void set_one_led(uint8_t r, uint8_t g, uint8_t b, int numero);

// Inicialização do display fora da interrupção
static ssd1306_t ssd;

 // Inicializa o display
void init_display();

// Rotina de interrupção com debouncing dos botões
static void gpio_irq_handler(uint gpio, uint32_t events);

int main()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    
    stdio_init_all();
    inicializacaocomponentes();
    init_display();

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    bool cor = true;
    while (true)
    {
    if (uart_is_readable(UART_ID)) {

        char c = uart_getc(UART_ID);
        // Se um número de 0 a 9 for digitado ele é exibido no display 
        if (c >= '0' && c <= '9') {
            int numero = c - '0';
            set_one_led(led_r, led_g, led_b, numero);
            
            ssd1306_fill(&ssd, false);
            
            int x_numero = 64 - (6 / 2);
            int x_texto = 64 - (8 * 4) / 2;

            ssd1306_draw_string(&ssd, "Numero:", x_texto, 20);
            
            char str[2] = {c, '\0'};
            ssd1306_draw_string(&ssd, str, x_numero, 40);
            
            ssd1306_send_data(&ssd);
        }

        // Se uma letra maiúscula ou minúscula for digitada, será exibida no display
        else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ') {
            char texto[2] = {c, '\0'};

            ssd1306_fill(&ssd, false);

            int x_letra = 64 - (6 / 2);
            int x_texto = 64 - (6 * 4) / 2;
            
            ssd1306_draw_string(&ssd, "Letra:", x_texto, 20);
            ssd1306_draw_string(&ssd, texto, x_letra, 40);
            
            ssd1306_send_data(&ssd);
        }


        // Envia de volta o caractere lido (eco)
        uart_putc(UART_ID, c);

        uart_puts(UART_ID, " <- Eco do RP2\r\n");
        }
    }
    sleep_ms(1000);

    return 0;
}

// Funções abaixo são para a matriz de LED RGB e botões

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

     // I2C Inicialização
     i2c_init(I2C_PORT, 400 * 1000);

     gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
     gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
     gpio_pull_up(I2C_SDA);
     gpio_pull_up(I2C_SCL);
  
    // Inicializa a UART
      uart_init(UART_ID, BAUD_RATE);
 
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  
    const char *init_message = "Digite para exibir um caractere\r\n";
    uart_puts(UART_ID, init_message);
}


// Funções abaixo são para a matriz de LEDs ws2812

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}
 
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}
 
void set_one_led(uint8_t r, uint8_t g, uint8_t b, int numero) {
 
    uint32_t color = urgb_u32(r, g, b);
 
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_buffer[numero][i]) {
            put_pixel(color);
        } else {
            put_pixel(0);
        }
    }
}

// Funções do display ssd1306

void init_display() {
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Funções de interrupções dos botões com debounce

static void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN && current_time - last_interrupt_time_a > 300000) { // Deboucing
        last_interrupt_time_a = current_time;

    // Alterna o estado do LED Verde
        bool estado = !gpio_get(LED_G_PIN);
        gpio_put(LED_G_PIN, estado);

        uart_puts(UART_ID, estado ? "LED Verde ligado\r\n" : "LED Verde desligado\r\n");

        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, estado ? "LED Verde ON" : "LED Verde OFF", 10, 30);
        ssd1306_send_data(&ssd);
    }
    else if (gpio == BUTTON_B_PIN && current_time - last_interrupt_time_b > 300000) { // Deboucing
        last_interrupt_time_b = current_time;

        // Alterna o estado do LED Azul
        bool estado = !gpio_get(LED_B_PIN);
        gpio_put(LED_B_PIN, estado);

        uart_puts(UART_ID, estado ? "LED Azul ligado\r\n" : "LED Azul desligado\r\n");

        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, estado ? "LED Azul ligado" : "LED Azul desligado", 10, 30);
        ssd1306_send_data(&ssd);
    }
}