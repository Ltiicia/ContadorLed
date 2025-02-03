/* 
*************************************************
Letícia Gonçalves Souza 
EmbarcaTech - Tarefa 1, unidade 4 (Interrupções)
*************************************************
*/

#include <stdio.h>          
#include "pico/stdlib.h"     
#include <hardware/pio.h>    
#include "hardware/clocks.h"
#include "ws2818b.pio.h"    
#include "hardware/adc.h"    
#include "hardware/irq.h"   
#include "hardware/pwm.h"    

// Definições de constantes
#define LED_COUNT 25 
#define LED_PIN 7    

#define LED_RED 13 

#define BUTTON_A 5 
#define BUTTON_B 6

#define LUMINOSIDADE 50 

// Estrutura para representar um pixel com componentes de cor: Verde, Vermelho e Azul
struct pixel_t
{
    uint32_t G, R, B; 
};

typedef struct pixel_t pixel_t; 
typedef pixel_t npLED_t;        

npLED_t leds[LED_COUNT];
PIO np_pio;              
uint sm;               

volatile int currentNumber = 0;           
volatile absolute_time_t lastButtonTimeA; 
volatile absolute_time_t lastButtonTimeB; 

/**
 * @brief Função para inicializar os LEDs
 * 
 * @param pin Pino GPIO conectado aos LEDs
 */
void npInit(uint pin)
{
    uint offset = pio_add_program(pio0, &ws2818b_program); 
    np_pio = pio0;                                       

    sm = pio_claim_unused_sm(np_pio, false); 
    if (sm < 0)                              
    {
        np_pio = pio1;                          
        sm = pio_claim_unused_sm(np_pio, true);
    }

    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); 

    for (uint i = 0; i < LED_COUNT; ++i) 
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

/**
 * @brief Função para definir a cor de um LED específico
 * 
 * @param index Índice do LED na matriz
 * @param r Componente vermelho da cor
 * @param g Componente verde da cor
 * @param b Componente azul da cor
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
    leds[index].R = r; 
    leds[index].G = g; 
    leds[index].B = b; 
}

/**
 * @brief Função para limpar (apagar) todos os LEDs
 */
void npClear()
{
    for (uint i = 0; i < LED_COUNT; ++i) 
        npSetLED(i, 0, 0, 0);            
}

/**
 * @brief Função para enviar os dados dos LEDs para o hardware
 */
void npWrite()
{
    for (uint i = 0; i < LED_COUNT; ++i) 
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G << 24); 
        pio_sm_put_blocking(np_pio, sm, leds[i].R << 24); 
        pio_sm_put_blocking(np_pio, sm, leds[i].B << 24); 
    }
}

/**
 * @brief Função para inverter a matriz de números
 * 
 * @param original Matriz original de números
 * @param inverted Matriz invertida de números
 */
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

/**
 * @brief Função para exibir um número na matriz de LEDs
 * 
 * @param number Número a ser exibido
 * @param r Componente vermelho da cor
 * @param g Componente verde da cor
 * @param b Componente azul da cor
 */
void displayNumber(int number, uint8_t r, uint8_t g, uint8_t b)
{
    npClear();

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

    uint8_t invertedNumbers[10][25];      
    invertMatrix(numbers, invertedNumbers); 

    for (uint i = 0; i < LED_COUNT; ++i) 
    {
        if (invertedNumbers[number][i])
        {
            npSetLED(i, r, g, b); 
        }
    }

    npWrite(); 
}

/**
 * @brief Função principal
 */
int main()
{
    stdio_init_all(); 
    npInit(LED_PIN);  
    npClear();        
    npWrite();        

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
    pwm_set_wrap(slice_num, 255);    
    pwm_set_gpio_level(LED_RED, 0);   
    pwm_set_enabled(slice_num, true); 

    displayNumber(currentNumber, 2, 2, 2);

    bool ledState = false;                                
    absolute_time_t lastBlinkTime = get_absolute_time();   
    absolute_time_t lastButtonTimeA = get_absolute_time(); 
    absolute_time_t lastButtonTimeB = get_absolute_time(); 

    while (true) 
    {
        absolute_time_t currentTime = get_absolute_time();

        // Verificar botão A com debounce
        if (!gpio_get(BUTTON_A) && absolute_time_diff_us(lastButtonTimeA, currentTime) >= 200000) // 200ms debounce
        {
            currentNumber = (currentNumber + 1) % 10; 
            displayNumber(currentNumber, 2, 2, 2);    
            lastButtonTimeA = currentTime;            
        }

        // Verificar botão B com debounce
        if (!gpio_get(BUTTON_B) && absolute_time_diff_us(lastButtonTimeB, currentTime) >= 200000) // 200ms debounce
        {
            currentNumber = (currentNumber - 1 + 10) % 10; 
            displayNumber(currentNumber, 2, 2, 2);         
            lastButtonTimeB = currentTime;                 
        }

        // Ajustar brilho do LED_RED com PWM para piscar 5 vezes por segundo
        if (absolute_time_diff_us(lastBlinkTime, currentTime) >= 100000) 
        {
            ledState = !ledState;                                     
            pwm_set_gpio_level(LED_RED, ledState ? LUMINOSIDADE : 0); 
            lastBlinkTime = currentTime;                             
        }

        sleep_ms(10); 
    }
}