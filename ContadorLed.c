#include <stdio.h>           // Biblioteca padrão de entrada e saída
#include "pico/stdlib.h"     // Biblioteca padrão para o Raspberry Pi Pico
#include <hardware/pio.h>    // Biblioteca para manipulação de periféricos PIO
#include "hardware/clocks.h" // Biblioteca para controle de relógios do hardware
#include "ws2818b.pio.h"     // Biblioteca PIO para controle de LEDs WS2818B
#include "hardware/adc.h"    // Biblioteca para controle do ADC (Conversor Analógico-Digital)
#include "hardware/irq.h"    // Biblioteca para controle de interrupções
#include "hardware/pwm.h"    // Biblioteca para controle de PWM

// Definições de constantes
#define LED_COUNT 25 // Número de LEDs na matriz
#define LED_PIN 7    // Pino GPIO conectado aos LEDs

#define LED_RED 13 // Pino GPIO para o LED vermelho

#define BUTTON_A 5 // Pino GPIO para o botão A
#define BUTTON_B 6 // Pino GPIO para o botão B

#define LUMINOSIDADE 50 // Defina a luminosidade constante (0-255)

// Estrutura para representar um pixel com componentes de cor: Verde, Vermelho e Azul
struct pixel_t
{
    uint32_t G, R, B; // Componentes de cor: Verde, Vermelho e Azul
};

typedef struct pixel_t pixel_t; // Alias para a estrutura pixel_t
typedef pixel_t npLED_t;        // Alias para facilitar o uso no contexto de LEDs

npLED_t leds[LED_COUNT]; // Array para armazenar o estado de cada LED
PIO np_pio;              // Variável para referenciar a instância PIO usada
uint sm;                 // Variável para armazenar a state machine usada

volatile int currentNumber = 0;           // Número atual exibido
volatile absolute_time_t lastButtonTimeA; // Tempo da última ativação do botão A
volatile absolute_time_t lastButtonTimeB; // Tempo da última ativação do botão B

// Função para inicializar os LEDs
void npInit(uint pin)
{
    uint offset = pio_add_program(pio0, &ws2818b_program); // Carregar o programa PIO
    np_pio = pio0;                                         // Usar o primeiro bloco PIO

    sm = pio_claim_unused_sm(np_pio, false); // Tentar usar uma state machine do pio0
    if (sm < 0)                              // Se não houver disponível no pio0
    {
        np_pio = pio1;                          // Mudar para o pio1
        sm = pio_claim_unused_sm(np_pio, true); // Usar uma state machine do pio1
    }

    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); // Inicializar state machine para LEDs

    for (uint i = 0; i < LED_COUNT; ++i) // Inicializar todos os LEDs como apagados
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Função para definir a cor de um LED específico
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
    leds[index].R = r; // Definir componente vermelho
    leds[index].G = g; // Definir componente verde
    leds[index].B = b; // Definir componente azul
}

// Função para limpar (apagar) todos os LEDs
void npClear()
{
    for (uint i = 0; i < LED_COUNT; ++i) // Iterar sobre todos os LEDs
        npSetLED(i, 0, 0, 0);            // Definir cor como preta (apagado)
}

// Função para enviar os dados dos LEDs para o hardware
void npWrite()
{
    for (uint i = 0; i < LED_COUNT; ++i) // Iterar sobre todos os LEDs
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G << 24); // Enviar componente verde
        pio_sm_put_blocking(np_pio, sm, leds[i].R << 24); // Enviar componente vermelho
        pio_sm_put_blocking(np_pio, sm, leds[i].B << 24); // Enviar componente azul
    }
}

// Função para inverter a matriz de números
void invertMatrix(const uint8_t original[10][25], uint8_t inverted[10][25])
{
    for (int digit = 0; digit < 10; digit++)
    {
        for (int row = 0; row < 5; row++)
        {
            for (int col = 0; col < 5; col++)
            {
                inverted[digit][row * 5 + col] = original[digit][(4 - row) * 5 + col];
            }
        }
    }
}

// Função para exibir um número na matriz de LEDs
void displayNumber(int number, uint8_t r, uint8_t g, uint8_t b)
{
    npClear(); // Limpar todos os LEDs

    // Defina os padrões para os números de 0 a 9
    const uint8_t numbers[10][25] = {
        {1, 1, 1, 1, 1,
         1, 1, 0, 1, 1,
         1, 1, 0, 1, 1,
         1, 1, 0, 1, 1,
         1, 1, 1, 1, 1}, // 0

        {0, 0, 1, 0, 0,
         0, 1, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         1, 1, 1, 1, 1}, // 1

        {1, 1, 1, 1, 1,
         0, 0, 0, 0, 1,
         1, 1, 1, 1, 1,
         1, 0, 0, 0, 0,
         1, 1, 1, 1, 1}, // 2

        {1, 1, 1, 1, 1,
         0, 0, 0, 0, 1,
         1, 1, 1, 1, 1,
         0, 0, 0, 0, 1,
         1, 1, 1, 1, 1}, // 3

        {1, 0, 0, 0, 1,
         1, 0, 0, 0, 1,
         1, 1, 1, 1, 1,
         0, 0, 0, 0, 1,
         1, 0, 0, 0, 0}, // 4

        {1, 1, 1, 1, 1,
         1, 0, 0, 0, 0,
         1, 1, 1, 1, 1,
         0, 0, 0, 0, 1,
         1, 1, 1, 1, 1}, // 5

        {1, 1, 1, 1, 1,
         1, 0, 0, 0, 0,
         1, 1, 1, 1, 1,
         1, 0, 0, 0, 1,
         1, 1, 1, 1, 1}, // 6

        {1, 1, 1, 1, 1,
         0, 0, 0, 0, 1,
         0, 1, 0, 0, 0,
         0, 0, 0, 1, 0,
         0, 1, 0, 0, 0}, // 7

        {1, 1, 1, 1, 1,
         1, 0, 0, 0, 1,
         1, 1, 1, 1, 1,
         1, 0, 0, 0, 1,
         1, 1, 1, 1, 1}, // 8

        {1, 1, 1, 1, 1,
         1, 0, 0, 0, 1,
         1, 1, 1, 1, 1,
         0, 0, 0, 0, 1,
         1, 1, 1, 1, 1} // 9
    };

    uint8_t invertedNumbers[10][25];        // Matriz para armazenar os números invertidos
    invertMatrix(numbers, invertedNumbers); // Inverter a matriz de números

    for (uint i = 0; i < LED_COUNT; ++i) // Iterar sobre todos os LEDs
    {
        if (invertedNumbers[number][i]) // Se o LED deve estar aceso
        {
            npSetLED(i, r, g, b); // Definir cor do LED
        }
    }

    npWrite(); // Enviar os dados dos LEDs para o hardware
}

// Função principal
int main()
{
    stdio_init_all(); // Inicializar a comunicação serial
    npInit(LED_PIN);  // Inicializar os LEDs
    npClear();        // Apagar todos os LEDs
    npWrite();        // Atualizar o estado inicial dos LEDs

    // Inicializar os botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Inicializar PWM para LED_RED
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_RED);
    pwm_set_wrap(slice_num, 255);     // Configurar o contador PWM para 8 bits (0-255)
    pwm_set_gpio_level(LED_RED, 0);   // Inicialmente desligar o LED
    pwm_set_enabled(slice_num, true); // Habilitar PWM

    displayNumber(currentNumber, 2, 2, 2); // Inicializar com cor verde

    bool ledState = false;                                 // Estado do LED vermelho
    absolute_time_t lastBlinkTime = get_absolute_time();   // Tempo da última mudança de estado do LED vermelho
    absolute_time_t lastButtonTimeA = get_absolute_time(); // Tempo da última ativação do botão A
    absolute_time_t lastButtonTimeB = get_absolute_time(); // Tempo da última ativação do botão B

    while (true) // Loop principal
    {
        absolute_time_t currentTime = get_absolute_time(); // Obter o tempo atual

        // Verificar botão A com debounce
        if (!gpio_get(BUTTON_A) && absolute_time_diff_us(lastButtonTimeA, currentTime) >= 200000) // 200ms debounce
        {
            currentNumber = (currentNumber + 1) % 10; // Incrementar o número atual
            displayNumber(currentNumber, 2, 2, 2);    // Alterar cor conforme necessário
            lastButtonTimeA = currentTime;            // Atualizar o tempo da última ativação do botão A
        }

        // Verificar botão B com debounce
        if (!gpio_get(BUTTON_B) && absolute_time_diff_us(lastButtonTimeB, currentTime) >= 200000) // 200ms debounce
        {
            currentNumber = (currentNumber - 1 + 10) % 10; // Decrementar o número atual
            displayNumber(currentNumber, 2, 2, 2);         // Alterar cor conforme necessário
            lastButtonTimeB = currentTime;                 // Atualizar o tempo da última ativação do botão B
        }

        // Ajustar brilho do LED_RED com PWM para piscar 5 vezes por segundo
        if (absolute_time_diff_us(lastBlinkTime, currentTime) >= 100000) // 100ms = 0.1s
        {
            ledState = !ledState;                                     // Alternar estado do LED
            pwm_set_gpio_level(LED_RED, ledState ? LUMINOSIDADE : 0); // Definir luminosidade constante
            lastBlinkTime = currentTime;                              // Atualizar o tempo da última mudança de estado do LED vermelho
        }

        sleep_ms(10); // Aguardar 10ms antes de continuar o loop
    }
}