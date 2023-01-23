#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include "beep.h"

static xQueueHandle beep_queue = NULL;
TaskHandle_t beepTask_Handle = NULL;

#define SERVO_PULSE_GPIO (15) // GPIO connects to the PWM signal line
#define BEEP_TASK_HEAP 2048

const static char *TAG = "BEEP APP";

static void beep_start()
{
    // 设置占空比
    mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_GEN_A, 50);
    // 设置占空比类型，并恢复PWM输出
    mcpwm_set_duty_type(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
}

static void beep_stop()
{
    // 设置MCPWM的某个generator信号为高[或低]
    //  mcpwm_set_signal_high(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_GEN_A);
    mcpwm_set_signal_low(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_GEN_A);
}

static void beep_task(void *arg)
{
    uint32_t evt;
    // int buffer_sz = 0;
    for (;;)
    {
        // buffer_sz = uxTaskGetStackHighWaterMark(beepTask_Handle);
        // ESP_LOGW(TAG,"beep堆栈剩余大小 = %d\n", buffer_sz);
        if (xQueueReceive(beep_queue, &evt, portMAX_DELAY))
        {
            if (evt == BEEP_SHORT_100MS)
            {
                beep_start();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_stop();
            }
            else if (evt == BEEP_SHORT_200MS)
            {
                beep_start();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_stop();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_start();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_stop();
            }
            else if (evt == BEEP_LONG)
            {
                beep_start();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_stop();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_start();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_stop();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_start();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                beep_stop();
            }
            else
            {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }
    }
}

void send_beep_event(BEEP_TYPE_E type)
{
    uint32_t evt = type;
    xQueueSend(beep_queue, &evt, 0);
}

void send_beep_event_from_isr(BEEP_TYPE_E type)
{
    uint32_t evt = type;
    xQueueSendFromISR(beep_queue, &evt, 0);
}

void beep_init(void)
{
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, SERVO_PULSE_GPIO); // To drive a RC servo, one MCPWM generator is enough
    mcpwm_config_t pwm_config = {
        .frequency = 4000, // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,       // duty cycle of PWMxA = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &pwm_config);
    beep_stop();
    // create a queue to handle gpio event from isr
    beep_queue = xQueueCreate(10, sizeof(uint32_t));
    // start gpio task
    xTaskCreatePinnedToCore(beep_task, "beep_task", BEEP_TASK_HEAP, NULL, 10, beepTask_Handle, 1);
}
