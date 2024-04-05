#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

//GPIOs para os segmentos dos displays
const int segmentPins[] = {2, 3, 8, 5, 4, 7, 10, 6};
const int mosfetPins[] = {1, 0}; // GPIOs para os MOSFETs que controlam os displays

const int displayCodes[][8] = {
    {1, 1, 1, 1, 1, 1, 0, 0},  // 0
    {0, 1, 1, 0, 0, 0, 0, 0},  // 1
    {1, 1, 0, 1, 1, 0, 1, 0},  // 2
    {1, 1, 1, 1, 0, 0, 1, 0},  // 3
    {0, 1, 1, 0, 0, 1, 1, 0},  // 4
    {1, 0, 1, 1, 0, 1, 1, 0},  // 5
    {1, 0, 1, 1, 1, 1, 1, 0},  // 6
    {1, 1, 1, 0, 0, 0, 0, 0},  // 7
    {1, 1, 1, 1, 1, 1, 1, 0},  // 8
    {1, 1, 1, 1, 0, 1, 1, 0},  // 9
    {1, 1, 1, 0, 1, 1, 1, 0},  // A
    {0, 0, 1, 1, 1, 1, 1, 0},  // B
    {1, 0, 0, 1, 1, 1, 0, 0},  // C
    {0, 1, 1, 1, 1, 0, 1, 0},  // D
    {1, 0, 0, 1, 1, 1, 1, 0},  // E
    {1, 0, 0, 0, 1, 1, 1, 0}   // F
};
static const char* TAG = "CounterDisplay";

// Callbacks para os temporizadores
static void update_counter_callback(void* arg);
static void refresh_display_callback(void* arg);

// Funções de configuração e envio para os displays
void setup_segments(void);
void setup_mosfets(void);
void send2Displays(int value, int base);

void app_main(void) {
    setup_segments();
    setup_mosfets();

    static int counter = 0; // Contador global

    //  temporizador periódico para atualizar o contador
    //frequencia 1 hz
    const esp_timer_create_args_t counter_timer_args = {
        .callback = update_counter_callback,
        .arg = &counter,
        .name = "counter_update"
    };

    esp_timer_handle_t counter_timer;
    ESP_ERROR_CHECK(esp_timer_create(&counter_timer_args, &counter_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(counter_timer, 1000000)); // 1Hz

    // Criação e configuração do temporizador para refresh dos displays
    ////frequencia 100hz 
    const esp_timer_create_args_t display_timer_args = {
        .callback = refresh_display_callback,
        .arg = &counter,
        .name = "display_refresh"
    }; 

    esp_timer_handle_t display_timer;
    ESP_ERROR_CHECK(esp_timer_create(&display_timer_args, &display_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(display_timer, 10000)); // 100Hz
    
    while (1) {
        vTaskDelay(portMAX_DELAY); // Bloqueia a tarefa
    }
}

static void update_counter_callback(void* arg) {
    int* counter = (int*)arg;
    *counter = (*counter + 1) % 256; // Incrementa o contador e limita a 0-255
    ESP_LOGI(TAG, "Counter updated: 0x%X", *counter); // Imprime o valor em hexadecimal
}


static void refresh_display_callback(void* arg) {
    int counter = *((int*)arg);
    send2Displays(counter, 16); // Update the display with the current counter value
}

void setup_segments(void) {
    for (int i = 0; i < 8; i++) {
        gpio_reset_pin(segmentPins[i]);
        gpio_set_direction(segmentPins[i], GPIO_MODE_OUTPUT);
    }
}

void setup_mosfets(void) {
    gpio_reset_pin(mosfetPins[0]);
    gpio_set_direction(mosfetPins[0], GPIO_MODE_OUTPUT);
    gpio_reset_pin(mosfetPins[1]);
    gpio_set_direction(mosfetPins[1], GPIO_MODE_OUTPUT);
}

void send2Displays(int value, int base) {
    static char displayFlag = 0;
    int digit1 = value / base; // dígito mais significativo
    int digit2 = value % base; // dígito menos significativo

    if (displayFlag == 0) { // Alterna entre o display da direita e da esquerda
        gpio_set_level(mosfetPins[0], 0);
        gpio_set_level(mosfetPins[1], 1);
        for (int i = 0; i < 8; i++) {
            gpio_set_level(segmentPins[i], displayCodes[digit2][i]);
        }
    } else {
        gpio_set_level(mosfetPins[0], 1);
        gpio_set_level(mosfetPins[1], 0);
        for (int i = 0; i < 8; i++) {
            gpio_set_level(segmentPins[i], displayCodes[digit1][i]);
        }
    }
    displayFlag = !displayFlag;
}