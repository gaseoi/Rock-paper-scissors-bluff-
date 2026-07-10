#include <stdio.h>
#include "esp_log.h"
#include "uibackend.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/semphr.h"

#include "wifi-local-discovery.h"

#define PIN_SDA 8
#define PIN_SCL 9

static const char* TAG = "mainfile.c";

static void IRAM_ATTR joystickpress_isr(void* arg){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static TickType_t lastPressTime = 0;
    TickType_t currentPressTime = xTaskGetTickCountFromISR();
    if ((currentPressTime - lastPressTime) > pdMS_TO_TICKS(100) ){
        lastPressTime = currentPressTime;
        xEventGroupSetBitsFromISR(clientEventGroup, PRESS_BIT, &xHigherPriorityTaskWoken);
    }
}

void handleEventGroup(u8g2_t* u8g2){
    EventBits_t setBits;
    const EventBits_t bitsToCheck = ( PRESS_BIT | SERVER_READY_BIT | RANDOMISATION_DONE_BIT | FINAL_RESULT_BIT);
    setBits = xEventGroupWaitBits(clientEventGroup, bitsToCheck, pdTRUE, pdFALSE, 0);
    if( ( setBits & PRESS_BIT ) != 0 )
    {
        RPS_ButtonPressProcessing(u8g2);
    }
    if( ( setBits & SERVER_READY_BIT ) != 0 )
    {
        //trigger a page change 
        CurrentPage = StartPg;
        SelectedElement = DefaultHighlightElementDictionary[StartPg];
        RPS_DrawPage(u8g2, CurrentPage);
        RPS_HighlightSelectedItem(u8g2);
        ESP_LOGI(TAG, "page change to startpg");
    }
    if( ( setBits & RANDOMISATION_DONE_BIT ) != 0 )
    {
        CurrentPage = FinalePg;
        SelectedElement = DefaultHighlightElementDictionary[FinalePg];
        RPS_DrawPage(u8g2, CurrentPage);
        RPS_HighlightSelectedItem(u8g2);
        ESP_LOGI(TAG, "page change to finalepg");
    }
    if( ( setBits & FINAL_RESULT_BIT ) != 0 )
    {
        CurrentPage = ResultPg;
        SelectedElement = DefaultHighlightElementDictionary[ResultPg];
        RPS_DrawPage(u8g2, CurrentPage);
        RPS_HighlightSelectedItem(u8g2);
        ESP_LOGI(TAG, "page change to resultpg");
    }
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

    switch (event_id) {

    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        sharedJsonSendBuffer = cJSON_CreateObject();
        cJSON_AddStringToObject(sharedJsonSendBuffer, "status", "init");
        char* stringJsonSendBuffer = cJSON_Print(sharedJsonSendBuffer);
        if (esp_websocket_client_send_text(client, stringJsonSendBuffer, strlen(stringJsonSendBuffer), portMAX_DELAY) == -1){
            ESP_LOGE(TAG, "ESP client could not send init!");
        }
        else{
            ESP_LOGI(TAG, "ESP client sent init successfully");
        }
        free(stringJsonSendBuffer);
        break;

    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        
        ESP_LOGI(TAG, "Received=%.*s\n\n", data->data_len, (char *)data->data_ptr);
        if (data->op_code == 0xA) {
            ESP_LOGI(TAG, "Received a PONG from the server.");
            break;
            }
        if (data->op_code == 0x9) {
            ESP_LOGI(TAG, "Received a PING frame from the server.");
            break;
        }

        cJSON *messageJSON = cJSON_Parse(data->data_ptr);

        cJSON *status = cJSON_GetObjectItem(messageJSON, "status");
        ESP_LOGI(TAG, "value of status key: %s", status->valuestring);
        if (strcmp(status->valuestring, "ready") == 0){
            xEventGroupSetBits(clientEventGroup, SERVER_READY_BIT);
            ESP_LOGI(TAG, "server ready status bit set");
        }

        if (strcmp(status->valuestring, "randomised")== 0){
            //iterate over R, P, and S keys, and add the pointers to each card's bitmap
            //to an array with 3 elements before event group gets set
            ESP_LOGI(TAG, "websocket event handler saw status randomised.");
            cJSON *rock = cJSON_GetObjectItem(messageJSON, "R");
            cJSON *paper = cJSON_GetObjectItem(messageJSON, "P");
            cJSON *scissor = cJSON_GetObjectItem(messageJSON, "S");
            int recvCards[3] = {
                rock->valueint, paper->valueint, scissor->valueint,
            };
            int* recvCardsPtr = recvCards;
            uint8_t recvCardsIndex = 0;
            for (uint8_t i = 0; i<3; i++){
                while(*recvCardsPtr == 0){
                    recvCardsPtr++;
                    recvCardsIndex++;
                }
                switch (recvCardsIndex){
                    case 0:
                    FinalePage[i].SpecificData.Symbol.Bitmap = rockMap;
                    break;
                    case 1:
                    FinalePage[i].SpecificData.Symbol.Bitmap = paperMap;
                    break;
                    case 2:
                    FinalePage[i].SpecificData.Symbol.Bitmap = scissorMap;
                    break;
                    default:
                    //should never reach here
                    ESP_LOGE(TAG, "recvCardsIndex went beyond 2 (bad)");
                }
                (*recvCardsPtr)--;
            }
            xEventGroupSetBits(clientEventGroup, RANDOMISATION_DONE_BIT);
            ESP_LOGI(TAG, "randomisation event bit set");
        }

        if (strcmp(status->valuestring, "final")== 0){
            cJSON *outcome = cJSON_GetObjectItem(messageJSON, "outcome");
            if (strcmp(outcome->valuestring, "tie")== 0){
                strncpy(ResultPage[0].SpecificData.Infobox.Text, "TIE",4);
            }
            else if (strcmp(outcome->valuestring, "lose")== 0){
                strncpy(ResultPage[0].SpecificData.Infobox.Text, "LOSE",5);
            }
            else{
                strncpy(ResultPage[0].SpecificData.Infobox.Text, "WIN",4);

            }
            xEventGroupSetBits(clientEventGroup, FINAL_RESULT_BIT);
        }

        cJSON_Delete(messageJSON);
        break;
    }
}

void app_main(void)
{
    u8g2_t u8g2;
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = PIN_SDA;
    u8g2_esp32_hal.bus.i2c.scl = PIN_SCL;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,
		  U8G2_R0,
		  u8g2_esp32_i2c_byte_cb,
		  u8g2_esp32_gpio_and_delay_cb);
    u8g2_SetI2CAddress(&u8g2.u8x8,0x78);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    int joystickX;
    int joystickY;

    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config);

    SelectedElement = &WaitPage[0];
    CurrentPage = WaitPg;

    clientEventGroup = xEventGroupCreate();
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1<< GPIO_NUM_5);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, joystickpress_isr, NULL);

    wifi_init_sta("---", "---");
    esp_mdns_discovery_start("esp32c3", "client"); //name doesnt matter

    char serveraddr_str[INET_ADDRSTRLEN];
    struct esp_ip4_addr serveraddr_espip4 = resolve_mdns_host("---");
    esp_ip4addr_ntoa(&serveraddr_espip4, serveraddr_str, INET_ADDRSTRLEN);

    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.host = serveraddr_str;
    websocket_cfg.port = 8001;
    websocket_cfg.pingpong_timeout_sec = 90;
    websocket_cfg.disable_auto_reconnect = true;

    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void*)client);
    esp_websocket_client_start(client);

    while(1){
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &joystickX));
        //ESP_LOGI(TAG, "joystick x value: %d", joystickX);

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_1, &joystickY));
        //ESP_LOGI(TAG, "joystick Y value: %d", joystickY);

        RPS_Direction_t Dir = RPS_ConvertCoordToDirection(joystickX,joystickY);

        /*
        switch (Dir){
            case none:
            ESP_LOGI(TAG, "No joystick movement");
            break;
            case left:
            ESP_LOGI(TAG, "pointing left");
            break;
            case up:
            ESP_LOGI(TAG, "pointing up");
            break;
            case right:
            ESP_LOGI(TAG, "pointing right");
            break;
            case down:
            ESP_LOGI(TAG, "pointing down");
            break;
            default:
            ESP_LOGE(TAG, "Invalid direction");
            break;
        }
        */

        SelectedElement = RPS_ProcessDirection(Dir);
        RPS_DrawPage(&u8g2, CurrentPage);
        RPS_HighlightSelectedItem(&u8g2);
        handleEventGroup(&u8g2);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
