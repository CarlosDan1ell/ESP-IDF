#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#define LED_1 GPIO_NUM_25 //vermelho
#define LED_2 GPIO_NUM_26 //amarelo
#define LED_3 GPIO_NUM_27 //verde
#define BUTTON_1 GPIO_NUM_16
#define BUTTON_2 GPIO_NUM_22
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<LED_1)|(1ULL<<LED_2)|(1ULL<<LED_3))
#define GPIO_INPUT_PIN_SEL ((1ULL<<BUTTON_1)|(1ULL<<BUTTON_2))
#define ESP_INTR_FLAG_DEFAULT 0
volatile  int cnt = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg){
    //identifica qual botão foi presssionado
    if(BUTTON_1 == (uint32_t) arg){
        if(gpio_get_level((uint32_t)arg ==0)){ //primeiro botao acende o vermelho
            gpio_set_level(LED_1,cnt%2);

        }
    }
    else {
        if(BUTTON_2 == (uint32_t) arg){
            if(gpio_get_level((uint32_t)arg ==0)){  //segundo botao acende o amarelo
                gpio_set_level(LED_2,cnt%2);

            }
        }

        cnt++;

    }
}






void Task_LED(void *pvParameter){
    
    //Configura o descritor de Outputs(LED)
    gpio_config_t io_conf; //Declara a variável descritora do drive de GPIO.

    io_conf.intr_type =  GPIO_INTR_DISABLE; //Desabilita o recurso de interrupção neste descritor
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    //Configura o descritor de Inputs(botões)
    io_conf.intr_type =  GPIO_INTR_NEGEDGE; //Habilita interrupção na borda de descida 
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    //Ao utilizar gpio_install_isr_service(); todas as interrupções de GPIO do descritor
    //vão chamar a mesma interrupção. A função de callback que será chamada para cada interrupção
    //é definida em gpio_isr_handler_add();
    //o flag ESP_INTR_FLAG_DEFAULT tem a ver com a alocação do vetor de interrupção, que neste caso
    //o valor zero informa para alocar no setor de interrupção não compartilhado de nível 1, 2 ou 3

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(BUTTON_2, gpio_isr_handler, (void*) BUTTON_2); //Registra a interrupção externa do BOTTON_1
    gpio_isr_handler_add(BUTTON_1, gpio_isr_handler, (void*) BUTTON_1); //Registra a interrupção externa do BOTTON_2
    
    //função de interrupção 
    
    while(1){
    //Aguardando a interrupção das GPIOs serem geradas...
    vTaskDelay(300/portTICK_PERIOD_MS);   
         

    }
}

void app_main(void){
    xTaskCreate(Task_LED,"Task_LED",2048,NULL,1,NULL);
}