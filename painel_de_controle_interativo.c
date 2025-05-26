#include "painel_de_controle_interativo.h"

/*******************************************************
                      VARIÁVEIS GLOBAIS
*******************************************************/

/*******************************************************
                      TAREFAS
*******************************************************/

void vTaskEntrada(void *pvParameters)
{
    while (true)
    {
        if (!gpio_get(BOTAO_A)) // verifica se o botão foi pressionado
        {
            vTaskDelay(pdMS_TO_TICKS(DEBOUCE)); // Debounce de 20ms

            // Tenta consuir uma vaga (entrada de usuário)
            if (xSemaphoreTake(xSemaphoreUsuarios, 0) == pdTRUE)
            {

                // Atualiza display caso não tenha alguém usando
                if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    UBaseType_t usuarios = MAX_USUARIOS - uxSemaphoreGetCount(xSemaphoreUsuarios);
                    atualizar_display(usuarios);
                    xSemaphoreGive(xMutexDisplay);
                }
            }
            else // caso não tenha mais vagas para consumir, informa que o limete foi atingido
            {

                if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    ssd1306_fill(&ssd, false);
                    ssd1306_rect(&ssd, 0, 0, 127, 63, true, false);
                    ssd1306_draw_string(&ssd, "LIMITE", 40, 20);
                    ssd1306_draw_string(&ssd, "MAXIMO!", 40, 35);
                    ssd1306_send_data(&ssd);
                    pwm_set_enabled(slice_buzzer, true);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    pwm_set_enabled(slice_buzzer, false);
                    vTaskDelay(pdMS_TO_TICKS(300));
                    UBaseType_t usuarios = MAX_USUARIOS - uxSemaphoreGetCount(xSemaphoreUsuarios);
                    atualizar_display(usuarios);
                    xSemaphoreGive(xMutexDisplay);
                }
            }
            UBaseType_t usuarios = MAX_USUARIOS - uxSemaphoreGetCount(xSemaphoreUsuarios);
            alertas_leds(usuarios);

            while (!gpio_get(BOTAO_A))
                ; // Espera soltar para não acontecer varias leituras se eu segurar o botão
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vTaskSaida(void *pvParameters)
{
    while (true)
    {
        if (!gpio_get(BOTAO_B))
        {
            vTaskDelay(pdMS_TO_TICKS(DEBOUCE)); // Debounce

            // LIBERA uma vaga (saída de usuário)
            if (xSemaphoreGive(xSemaphoreUsuarios) == pdTRUE)
            {
                UBaseType_t usuarios = MAX_USUARIOS - uxSemaphoreGetCount(xSemaphoreUsuarios);
                // Atualiza display
                if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100)) == pdTRUE)
                {

                    atualizar_display(usuarios);
                    xSemaphoreGive(xMutexDisplay);
                }

                alertas_leds(usuarios);
            }

            while (!gpio_get(BOTAO_B))
                ; // Espera soltar
        }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
}

void vTaskReset(void *pvParameters)
{
    while (true)
    {
        // Espera pelo semáforo de reset (liberado pela interrupção do botão C)
        if (xSemaphoreTake(xSemaphoreReset, portMAX_DELAY) == pdTRUE)
        {
            // Reseta o contador de usuários
            while (xSemaphoreGive(xSemaphoreUsuarios) == pdTRUE)
            {
                // Continua dando até esvaziar todas as vagas
                if (uxSemaphoreGetCount(xSemaphoreUsuarios) == MAX_USUARIOS)
                    break;
            }

            // Atualiza display
            if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                atualizar_display(0); // Mostra 0 usuários
                xSemaphoreGive(xMutexDisplay);
            }
            alertas_leds(0);
            pwm_set_enabled(slice_buzzer, true);
            vTaskDelay(pdMS_TO_TICKS(150));
            pwm_set_enabled(slice_buzzer, false);
            vTaskDelay(pdMS_TO_TICKS(50));
            pwm_set_enabled(slice_buzzer, true);
            vTaskDelay(pdMS_TO_TICKS(150));
            pwm_set_enabled(slice_buzzer, false);
        }
    }
}

/*******************************************************
                  INTERRUPÇÃO DO BOTÃO C
*******************************************************/
// ISR do botão
void gpio_callback(uint gpio, uint32_t events)
{
    uint32_t tempo = time_us_32() / 1000; // Converte para milissegundos

    // Verifica se o tempo desde a última interrupção é maior que o debounce
    if ((tempo - ultimo_tempo) >= Debouce_botao_C)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xSemaphoreReset, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    ultimo_tempo = tempo; // Atualiza o tempo da última interrupção
}
/*******************************************************
                      FUNÇÕES AUXILIARES
*******************************************************/

void atualizar_display(int valor_contagem)
{
    char buffer[20];
    ssd1306_fill(&ssd, false);

    // Desenha borda de 1 pixel em toda a tela
    ssd1306_rect(&ssd, 0, 0, 127, 63, true, false);
    snprintf(buffer, sizeof(buffer), "CONTROLE DA");
    ssd1306_draw_string(&ssd, buffer, 20, 8);
    snprintf(buffer, sizeof(buffer), "BIBLIOTECA");
    ssd1306_draw_string(&ssd, buffer, 25, 20);
    ssd1306_rect(&ssd, 0, 0, 127, 30, true, false);

    // Mostra contagem
    snprintf(buffer, sizeof(buffer), "ATIVOS: %d/%d", valor_contagem, MAX_USUARIOS);
    ssd1306_draw_string(&ssd, buffer, 10, 45);

    ssd1306_send_data(&ssd);
}

void inicializar_botoes()
{
    // Configuração do Botão A (Entrada)
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A); // Habilita resistor pull-up interno

    // Configuração do Botão B (Saída)
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    // Configuração do Botão C (Joystick)
    gpio_init(BOTAO_C);
    gpio_set_dir(BOTAO_C, GPIO_IN);
    gpio_pull_up(BOTAO_C);
    gpio_set_irq_enabled_with_callback(BOTAO_C, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

void inicializar_display_i2c()
{ // FUNÇÃO PARA INICIALIZAR O I2C DO DISPLAY
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    char buffer[20];
    ssd1306_fill(&ssd, false);

    // Desenha borda de 1 pixel em toda a tela
    ssd1306_rect(&ssd, 0, 0, 127, 63, true, false);
    snprintf(buffer, sizeof(buffer), "CONTROLE DA");
    ssd1306_draw_string(&ssd, buffer, 20, 8);
    snprintf(buffer, sizeof(buffer), "BIBLIOTECA");
    ssd1306_draw_string(&ssd, buffer, 25, 20);
    ssd1306_rect(&ssd, 0, 0, 127, 30, true, false);

    // Mostra contagem
    snprintf(buffer, sizeof(buffer), "ATIVOS: 0/%d", MAX_USUARIOS);
    ssd1306_draw_string(&ssd, buffer, 10, 45);

    ssd1306_send_data(&ssd);
}

void inicializar_pwms_buzzer()
{
    // Buzzer A - grave audível (~2 kHz)
    gpio_set_function(BUZZER_A, GPIO_FUNC_PWM);
    slice_buzzer = pwm_gpio_to_slice_num(BUZZER_A);
    pwm_set_wrap(slice_buzzer, 62499);
    pwm_set_chan_level(slice_buzzer, pwm_gpio_to_channel(BUZZER_A), 31250);
    pwm_set_enabled(slice_buzzer, false);
}


void inicializar_leds()
{
    // Inicializa o pino do LED vermelho
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, 0); // Desliga o LED inicialmente

    // Inicializa o pino do LED azul
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_put(LED_AZUL, 1); // Desliga o LED inicialmente

    // Inicializa o pino do LED azul
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_VERDE, 0); // LIGA o LED inicialmente
}

void alertas_leds(int num_usuarios)
{
    if (num_usuarios == 0)
    {
        gpio_put(LED_VERMELHO, 0);
        gpio_put(LED_VERDE, 0);
        gpio_put(LED_AZUL, 1);
    }
    else if ((num_usuarios > 0) && (num_usuarios <= MAX_USUARIOS - 2))
    {
        gpio_put(LED_AZUL, 0);
        gpio_put(LED_VERMELHO, 0);
        gpio_put(LED_VERDE, 1);
    }
    else if (num_usuarios == MAX_USUARIOS - 1)
    {
        gpio_put(LED_AZUL, 0);
        gpio_put(LED_VERMELHO, 1);
        gpio_put(LED_VERDE, 1);
    }
    else if (num_usuarios == MAX_USUARIOS)
    {
        gpio_put(LED_VERDE, 0);
        gpio_put(LED_AZUL, 0);
        gpio_put(LED_VERMELHO, 1);
    }
}

int main()
{
    // Inicializa hardware
    inicializar_display_i2c();
    inicializar_botoes();
    inicializar_pwms_buzzer();
    inicializar_leds();
    // Cria semáforo com 10 vagas TOTALMENTE DISPONÍVEIS
    xSemaphoreUsuarios = xSemaphoreCreateCounting(MAX_USUARIOS, MAX_USUARIOS);
    xMutexDisplay = xSemaphoreCreateMutex();
    xSemaphoreReset = xSemaphoreCreateBinary(); // Semáforo binário para reset
    // Cria tarefas
    xTaskCreate(vTaskEntrada, "Entrada", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Saida", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "Reset", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1)
        ;
}