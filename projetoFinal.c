#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdio.h>

// Definição dos pinos
#define LED_GREEN 11  // LED verde (BitDogLab)
#define LED_RED 13    // LED vermelho (BitDogLab)
#define BTN_A 5       // Botão A (ativa LED vermelho)
#define BTN_B 6       // Botão B (ativa LED verde)

// Variáveis globais
static struct repeating_timer timer;  // Timer
static volatile bool led_verde_ativo = true;  // Controla qual LED está piscando
static volatile bool estado_led = false;  // Estado do LED atual

// Atualiza a mensagem do terminal com base no LED ativo
void atualizar_mensagem_terminal() {
    if (led_verde_ativo) {
        printf("Sistema em Standby\n");
    } else {
        printf("Ajuda a Caminho\n");
    }
}

// Callback do timer - Alterna o estado do LED ativo
bool repeating_timer_callback(struct repeating_timer *t) {   
    estado_led = !estado_led; // Alterna entre ligado e desligado

    if (led_verde_ativo) {
        gpio_put(LED_GREEN, estado_led);  // Pisca o LED verde
        gpio_put(LED_RED, false);         // Garante que o LED vermelho está apagado
    } else {
        gpio_put(LED_RED, estado_led);    // Pisca o LED vermelho
        gpio_put(LED_GREEN, false);       // Garante que o LED verde está apagado
    }

    atualizar_mensagem_terminal();  // Atualiza a mensagem no terminal
    
    return true; // Mantém o timer rodando
}

// Interrupção dos botões - Ativa LED vermelho (A) ou LED verde (B)
void gpio_irq_handler(uint gpio, uint32_t events) {
    //sleep_ms(50); // Pequeno delay para debounce

    if (gpio == BTN_A && led_verde_ativo) {
        // Alterna para o LED vermelho piscando a cada 0,25s
        led_verde_ativo = false;
        cancel_repeating_timer(&timer);
        add_repeating_timer_ms(250, repeating_timer_callback, NULL, &timer);
        gpio_put(LED_GREEN, false);  // Garante que o LED verde desligue
        printf("Botão A pressionado - Ajuda Solicitada\n");
    }
    else if (gpio == BTN_B && !led_verde_ativo) {
        // Alterna para o LED verde piscando a cada 1s
        led_verde_ativo = true;
        cancel_repeating_timer(&timer);
        add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);
        gpio_put(LED_RED, false);  // Garante que o LED vermelho desligue
        printf("Botão B pressionado - Ajuda Cancelada\n");
    }
}

int main() {
    stdio_init_all(); // Inicializa a comunicação serial
    
    // Configuração dos LEDs
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    // Configuração dos botões
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A); // Ativa pull-up interno

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B); // Ativa pull-up interno

    // Configuração da interrupção única para os botões
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);

    // Inicia o timer para piscar o LED verde a cada 1 segundo
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);
    
    // Exibir a mensagem inicial
    atualizar_mensagem_terminal();

    // Loop infinito para manter o programa rodando
    while (true) {
        tight_loop_contents(); // Mantém a CPU ativa sem consumir processamento extra
    }

    return 0;
}
