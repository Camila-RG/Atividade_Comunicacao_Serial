# Atividade EmbarcaTech - Comunicação Serial

## Descrição
Este repositório apresenta a solução da atividade prática proposta no EmbarcaTech(Aula 03/02/2025), sobre comunicação serial(UART e I2C), manipulação de LEDs e botões com interrupções e debouncing no microcontrolador RP2040.

O projeto consiste na interação do usuário com um display SSD1306, uma matriz de LEDs WS2812 e um LED RGB, utilizando botões físicos e comunicação serial via UART.

## Funcionalidades
### **1. Entrada de caracteres via UART**
- O usuário digita um caractere no Serial Monitor.
- O caractere é exibido no display SSD1306.
- Caso um número de 0 a 9 seja digitado, o símbolo correspondente é exibido na matriz de LEDs WS2812.

### **2. Interação com os botões**
#### **Botão A**
- Alterna o estado do LED RGB Verde (ligado/desligado).
- Exibe uma mensagem no display SSD1306 informando o estado do LED.
- Envia um texto descritivo para o Serial Monitor via UART.

#### **Botão B**
- Alterna o estado do LED RGB Azul (ligado/desligado).
- Exibe uma mensagem no display SSD1306 informando o estado do LED.
- Envia um texto descritivo para o Serial Monitor via UART.

## **Componentes e Ferramentas Utilizados**
- **Microcontrolador**: RP2040 (BitDogLab)
- **Matriz 5x5 de LEDs WS2812**
- **LED RGB** 
- **2 Push Buttons**
- **Display SSD1306**
- **Simulador Wokwi**
- **Visual Studio Code**
- **Linguagem de Programação**: C


## **Instruções de Uso**

### **1. Preparando o Ambiente de Desenvolvimento**
- **Instalar o SDK da Raspberry Pi Pico W**: Antes de começar, é necessário ter o SDK da Raspberry Pi Pico W configurado no VS Code para que o código seja compilado corretamente.
  
- **Importar o projeto pela extensão da Raspberry Pi Pico no VSCode**

- **Configuração do Simulador Wokwi**: Para simular o projeto, use o simulador Wokwi.

- **Monitor Serial**: Após carregar o código, abra o Serial Monitor para interagir com o programa com os caracteres enviados como mensagens de controle dos LEDs.

### **2. Usando a Interface**

- **Entrada via UART**: No Serial Monitor, digite caracteres (números de 0 a 9, letras de A-Z e letras de a-z). Os caracteres serão exibidos no display SSD1306, e os números acenderão a matriz de LEDs WS2812.

- **Interação com os botões**: Pressione o **Botão A** para alternar o estado do LED RGB Verde e o **Botão B** para alternar o LED RGB Azul. O estado de cada LED será exibido no display SSD1306 e enviado para o Serial Monitor.

## **Passo a Passo da Implementação**
### **1. Configuração do Ambiente**
- Configuração do VS Code para desenvolvimento.
- Configuração do simulador Wokwi para testar os componentes.
- Organização dos arquivos no repositório.

### **2. Configuração dos Pinos**
- Configuração das GPIOs para os botões como entradas com resistores pull-up internos.
- Configuração das GPIOs para os LEDs como saídas.

### **3. Implementação das Interrupções**
- Configuração de interrupções para os botões.
- Alterar estado dos LEDs com base nos eventos capturados.

### **4. Tratamento de Bouncing (Debouncing)**
- Os botões mecânicos podem gerar vários pulsos elétricos ao serem pressionados, causando leituras erradas. Para evitar isso, foi adicionado um pequeno delay após a detecção do evento.

### **5. Modificação da Biblioteca font.h**
- Adição de caracteres minúsculos para exibição no display SSD1306.

### **6. Exibição dos Números na Matriz WS2812**
- Cada número de 0 a 9 é representado por um padrão fixo de LEDs acesos.
- O valor do contador é traduzido para um padrão correspondente na matriz.

### **7. Comunicação via UART e I2C**
- UART: A comunicação com o Serial Monitor foi configurada via UART. Quando o usuário digita um caractere, ele é exibido no Serial Monitor e também refletido no display SSD1306 e na matriz WS2812.
- I2C: A exibição no display SSD1306 foi implementada utilizando a comunicação I2C, permitindo que as mensagens sobre o estado dos LEDs e os números digitados via UART sejam mostradas no display em tempo real.

### **8. Testes e Validação**
- Teste do comportamento dos LEDs e botões foram realizados no Wokwi e kit BitDogLab.

## **Desenvolvedor**
Camila Ramos Gomes

# **Vídeo Explicativo**
[vídeo explicativo do projeto]
