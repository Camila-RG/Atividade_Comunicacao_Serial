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

#define UART_ID uart0 // Seleciona a UART0
#define BAUD_RATE 115200 // Define a taxa de transmissão
#define UART_TX_PIN 0 // Pino GPIO usado para TX
#define UART_RX_PIN 1 // Pino GPIO usado para RX

#define I2C_PORT i2c1  // Define o barramento I2C a ser usado. Aqui está configurado para i2c1.
#define I2C_SDA 14     // Define o pino de dados I2C (SDA) como o pino 14.
#define I2C_SCL 15     // Define o pino de relógio I2C (SCL) como o pino 15.
#define endereco 0x3C  // Define o endereço I2C do display OLED como 0x3C

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
 
// Função para enviar dados para o LED WS2812
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}
 
 // Função para converter valores RGB em um único valor de 32 bits no formato GRB
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}
 
// Função para acender um número específico na matriz de LEDs
void set_one_led(uint8_t r, uint8_t g, uint8_t b, int numero) {
 
    uint32_t color = urgb_u32(r, g, b); // Define a cor
 
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_buffer[numero][i]) {
            put_pixel(color); // Liga os LEDs
        } else {
            put_pixel(0);  // Desliga os LEDs
        }
    }
}

// Interrupção dos botões com debouncing
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN && current_time - last_interrupt_time_a > 300000){ //Tempo de Debounce: 300 ms
        last_interrupt_time_a = current_time;
        gpio_put(LED_G_PIN, !gpio_get(LED_G_PIN)); // Altera o estado do LED verde entre ligado e desligado
    }
    else if (gpio == BUTTON_B_PIN && current_time - last_interrupt_time_b > 300000) { //Tempo de Debounce: 300 ms
        last_interrupt_time_b = current_time;
        gpio_put(LED_B_PIN, !gpio_get(LED_B_PIN)); // Altera o estado do LED azul entre ligado e desligado
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

    // I2C Inicialização. 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Define a função do pino GPIO como I2C para o pino SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Define a função do pino GPIO como I2C para o pino SCL
    gpio_pull_up(I2C_SDA); // Ativa o resistor de pull-up para a linha de dados (SDA)
    gpio_pull_up(I2C_SCL); // Ativa o resistor de pull-up para a linha de relógio (SCL)


    ssd1306_t ssd; // Inicializa a estrutura que representa o display

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display com as configurações fornecidas
    ssd1306_config(&ssd); // Configura o display com os parâmetros padrão
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display, apagando todos os pixels (o display começa com todos os pixels apagados)
    ssd1306_fill(&ssd, false); 
    ssd1306_send_data(&ssd); // Envia os dados atualizados (todos os pixels apagados) para o display
    
     // Inicializa a UART
     uart_init(UART_ID, BAUD_RATE);

     // Configura os pinos GPIO para a UART
     gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Configura o pino 0 para TX
     gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // Configura o pino 1 para RX
 
     // Mensagem inicial
     const char *init_message = "UART Demo - RP2\r\n";
     uart_puts(UART_ID, init_message);

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    bool cor = true;
    while (true)
    {
        cor = !cor;
        // Atualiza o conteúdo do display com animações
        ssd1306_fill(&ssd, !cor); // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
        ssd1306_draw_string(&ssd, "S2", 8, 10); // Desenha uma string
        ssd1306_draw_string(&ssd, "CAMILA", 20, 30); // Desenha uma string
        ssd1306_draw_string(&ssd, "TESTE", 15, 48); // Desenha uma string      
        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(1000);
    }
    return 0;
}