#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "esp_timer.h"

#define BTN_UP GPIO_NUM_14
#define BTN_DOWN GPIO_NUM_17
#define BTN_PWR GPIO_NUM_15
#define BTN_OK GPIO_NUM_16
#define VBUS_INTR GPIO_NUM_36

xQueueHandle BTN_UPQueue;
xQueueHandle BTN_DOWNQueue;
xQueueHandle BTN_PWRQueue;
xQueueHandle BTN_OKQueue;
xQueueHandle VBUS_INTRQueue;


uint32_t pre_time_up = 0;
uint32_t pre_time_down = 0;
uint32_t pre_time_pwr = 0;
uint32_t pre_time_ok = 0;
uint8_t pre_time_vbus_intr = 0;
uint64_t pre_time= 0;
SSD1306_t dev;


#define TAG "SSD1306"
#include "ssd1306_i2c_new.c"
float soc =0;
int who_am_i_reg;
int count=0;
bool up_flag =true;
bool down_flag =true;
bool pwr_flag =true;
bool ok_flag =true;
bool vbus_flag =true;


uint8_t all_done [1024] = {
		// 'logoBN128x64, 128x64px
// 'final', 128x64px
// 'all_done', 128x64px
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x07, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x7f, 0xfc, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x83, 0xff, 0xff, 0x83, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xff, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x60, 0x00, 0x03, 0x80, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x60, 0x00, 0x07, 0xc0, 0x00, 0x0c, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x70, 0x00, 0x07, 0xc0, 0x00, 0x1c, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x78, 0x00, 0x1f, 0xe0, 0x00, 0x3c, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x7c, 0x00, 0x3f, 0xf8, 0x00, 0x7c, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0x00, 0xff, 0xfe, 0x01, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xf8, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xf8, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xf1, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xc1, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xc0, 0x03, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xff, 0x80, 0x0f, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xff, 0xfb, 0xff, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void BTN_UPTask(void *param)
{
    gpio_set_direction(BTN_UP, GPIO_MODE_INPUT);
    gpio_set_intr_type(BTN_UP, GPIO_INTR_NEGEDGE);
    int BTN_NUMBER = 0;
    while (1)
    {
        if (xQueueReceive(BTN_UPQueue, &BTN_NUMBER, portMAX_DELAY))
        {
            printf("Up Button pressed!\n");
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text_x3(&dev, 0, " Up", 3, false);
            
            if(up_flag)
            {
                count++;
                up_flag=false;
            }
            xQueueReset(BTN_UPQueue);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void BTN_DOWNTask(void *params)
{
    gpio_set_direction(BTN_DOWN, GPIO_MODE_INPUT);
    gpio_set_intr_type(BTN_DOWN, GPIO_INTR_NEGEDGE);
    int BTN_NUMBER = 0;
    while (1)
    {

        if (xQueueReceive(BTN_DOWNQueue, &BTN_NUMBER, portMAX_DELAY))
        {
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text_x3(&dev, 0, "Down", 4, false);
            printf("Down Button pressed!\n");
            if(down_flag)
            {
                count++;
                down_flag=false;
            }
            xQueueReset(BTN_DOWNQueue);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void BTN_PWRTask(void *params)
{
    gpio_set_direction(BTN_PWR, GPIO_MODE_INPUT);
    gpio_set_intr_type(BTN_PWR, GPIO_INTR_NEGEDGE);
    int BTN_NUMBER = 0;
    while (1)
    {
        if (xQueueReceive(BTN_PWRQueue, &BTN_NUMBER, portMAX_DELAY))
        {
            printf("Power Button pressed!\n");
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text_x3(&dev, 0, "Power", 5, false);
            if(pwr_flag)
            {
                count++;
                pwr_flag=false;
            }            
            xQueueReset(BTN_PWRQueue);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
            

}

void BTN_OKTask(void *params)
{
    gpio_set_direction(BTN_OK, GPIO_MODE_INPUT);
    gpio_set_intr_type(BTN_OK, GPIO_INTR_NEGEDGE);
    int BTN_NUMBER = 0;
    while (1)
    {
        if (xQueueReceive(BTN_OKQueue, &BTN_NUMBER, portMAX_DELAY))
        {
            printf("OK Button pressed!\n");
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text_x3(&dev, 0, " OK", 3, false);
            if(ok_flag)
            {
                count++;
                ok_flag=false;
            }
            xQueueReset(BTN_OKQueue);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void VBUS_INTRTask(void *params)
{
    gpio_set_direction(VBUS_INTR, GPIO_MODE_INPUT);
    gpio_set_intr_type(VBUS_INTR, GPIO_INTR_POSEDGE);
    int BTN_NUMBER = 0;
    while (1)
    {
        if (xQueueReceive(VBUS_INTRQueue, &BTN_NUMBER, portMAX_DELAY))
        {
            
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text(&dev, 0, " Charging..", 12, false);
            ssd1306_display_text(&dev, 3, " Check lED..", 13, false);
            if(vbus_flag)
            {
                printf("Charging....!\n");
                vbus_flag=false;
            }
            xQueueReset(VBUS_INTRQueue);
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}


static void IRAM_ATTR BTN_UP_interrupt_handler(void *args)
{
    
    int pinNumber = (int)args;
    if(esp_timer_get_time() - pre_time_up > 400000)
    {
        xQueueSendFromISR(BTN_UPQueue, &pinNumber, NULL);

    }
    pre_time_up= esp_timer_get_time();
}

static void IRAM_ATTR BTN_DOWN_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    if(esp_timer_get_time() - pre_time_down > 400000)
    {
        xQueueSendFromISR(BTN_DOWNQueue, &pinNumber, NULL);
    }
    pre_time_down = esp_timer_get_time();
}

static void IRAM_ATTR BTN_PWR_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    if(esp_timer_get_time() - pre_time_pwr > 400000){
    xQueueSendFromISR(BTN_PWRQueue, &pinNumber, NULL);
    }
    pre_time_pwr = esp_timer_get_time();
}

static void IRAM_ATTR BTN_OK_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    if(esp_timer_get_time() - pre_time_ok > 400000){
    xQueueSendFromISR(BTN_OKQueue, &pinNumber, NULL);
    }
    pre_time_ok = esp_timer_get_time();
}

static void IRAM_ATTR VBUS_INTR_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    if(esp_timer_get_time() - pre_time_vbus_intr > 400000){
    xQueueSendFromISR(VBUS_INTRQueue, &pinNumber, NULL);
    }
    pre_time_vbus_intr = esp_timer_get_time();
}

void app_main(void)
{
	
    float soc =0;
    int who_am_i_reg;

    BTN_UPQueue = xQueueCreate(10, sizeof(int));
    BTN_DOWNQueue = xQueueCreate(10, sizeof(int));
    BTN_PWRQueue = xQueueCreate(10, sizeof(int));
    BTN_OKQueue = xQueueCreate(10, sizeof(int));
    VBUS_INTRQueue = xQueueCreate(10, sizeof(int));

#if CONFIG_I2C_INTERFACE
	ESP_LOGI(TAG, "INTERFACE is i2c");
	ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE



#if CONFIG_FLIP
	dev._flip = true;
	ESP_LOGW(TAG, "Flip upside down");
#endif
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN_UP, BTN_UP_interrupt_handler, (void *)BTN_UP);
    gpio_isr_handler_add(BTN_DOWN, BTN_DOWN_interrupt_handler, (void *)BTN_DOWN);
    gpio_isr_handler_add(BTN_PWR, BTN_PWR_interrupt_handler, (void *)BTN_PWR);
    gpio_isr_handler_add(BTN_OK, BTN_OK_interrupt_handler, (void *)BTN_OK);
    gpio_isr_handler_add(VBUS_INTR, VBUS_INTR_interrupt_handler, (void *)BTN_OK);
#if CONFIG_SSD1306_128x64
	ESP_LOGI(TAG, "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64
#if CONFIG_SSD1306_128x32
	ESP_LOGI(TAG, "Panel is 128x32");
	ssd1306_init(&dev, 128, 32);
#endif // CONFIG_SSD1306_128x32



	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, 1, "Display", 8, false);
    ssd1306_display_text(&dev, 3, "  test", 8, false);
    ssd1306_display_text(&dev, 5, "    Done", 10, false);
    printf("1.Display test Done! ");
    printf("\xE2\x9C\x93\n");
	vTaskDelay(2000 / portTICK_PERIOD_MS);
    soc = max17260_read_soc();
    ssd1306_clear_screen(&dev, false);
    if(soc>0.0 && soc<100.0)
    {
	    ssd1306_display_text(&dev, 1, "Battery", 8, false);
        ssd1306_display_text(&dev, 3, "  test", 8, false);
        ssd1306_display_text(&dev, 5, "    Done", 10, false);
        printf("2.Battery test Done!(Batter status: %.2f%%) ",soc);  
        printf("\xE2\x9C\x93\n");      
    }
	vTaskDelay(2000 / portTICK_PERIOD_MS);
    who_am_i_reg = mpu6050_who_am_i();
    if(who_am_i_reg==104)
    {
        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text(&dev, 1, "MPU6050", 7, false);
        ssd1306_display_text(&dev, 3, "  test", 8, false);
        ssd1306_display_text(&dev, 5, "    Done", 10, false);
        printf("3.Mpu6050 test Done!(Address: %d ) ",who_am_i_reg); 
        printf("\xE2\x9C\x93\n"); 
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("--------------------------------------------------------------------------\n");
    printf("Press Each Buttons!\n");
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 1, "Press", 5, false);
    ssd1306_display_text(&dev, 3, "  Each", 7, false);
    ssd1306_display_text(&dev, 5, "    Buttons", 12, false);

    xTaskCreate(BTN_UPTask, "BTN_UPTask", 2048, NULL, 1, NULL);
    xTaskCreate(BTN_DOWNTask, "BTN_DOWNTask", 2048, NULL, 1, NULL);
    xTaskCreate(BTN_PWRTask, "BTN_PWRTask", 2048, NULL, 1, NULL);
    xTaskCreate(BTN_OKTask, "BTN_OKTask", 8000, NULL, 1, NULL);

    while(count!=4)
    {
      vTaskDelay(100/portTICK_PERIOD_MS);
    }
    printf("--------------------------------------------------------------------------\n");
    printf("4.All Buttons works Fine! ");
    printf("\xE2\x9C\x93\n");
    printf("Connect Charger!");
    xTaskCreate(VBUS_INTRTask, "VBUS_INTRTask", 8000, NULL, 1, NULL);

    ssd1306_clear_screen(&dev, false);
    while(vbus_flag)
    {
        ssd1306_display_text(&dev, 1, "Connect", 8, false);
        ssd1306_display_text(&dev, 3, "  Charger", 10, false);
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 1, "Charging", 9, false);
    ssd1306_display_text(&dev, 3, "  check LED", 12, false);
    vTaskDelay(3000/portTICK_PERIOD_MS);
    ssd1306_bitmaps(&dev, 0, 0, all_done, 128, 64, false);
    ssd1306_display_text(&dev, 7, "   Good to go!", 15, true);
    printf("All test done :) ");
    printf("\xE2\x9C\x93\n");

}


