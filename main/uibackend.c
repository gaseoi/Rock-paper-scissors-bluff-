#include "uibackend.h"

#include "esp_websocket_client.h"

static const char* TAG = "uibackend";

cJSON* sharedJsonSendBuffer;

esp_websocket_client_handle_t client;

void StartPageCallback(void){
    ESP_LOGI(TAG, "Executing startpage button cb");
    if (cJSON_ReplaceItemInObjectCaseSensitive(sharedJsonSendBuffer, "status", cJSON_CreateString("cardUpload")) == 0){
        ESP_LOGE(TAG, "no status key present to replace with handlecardupload");
    }

    uint8_t rockNum = 0;
    uint8_t paperNum = 0;
    uint8_t scissorNum = 0;

    for (uint8_t i = 0; i<3; i++){
        if (StartPage[i].SpecificData.Symbol.Bitmap == rockMap){
            rockNum++;
        }
        if (StartPage[i].SpecificData.Symbol.Bitmap == paperMap){
            paperNum++;
        }
        if (StartPage[i].SpecificData.Symbol.Bitmap == scissorMap){
            scissorNum++;
        }
    }
    cJSON_AddNumberToObject(sharedJsonSendBuffer,"R", rockNum);
    cJSON_AddNumberToObject(sharedJsonSendBuffer,"P", paperNum);
    cJSON_AddNumberToObject(sharedJsonSendBuffer,"S", scissorNum);
    char* stringJsonSendBuffer = cJSON_Print(sharedJsonSendBuffer);
    if (esp_websocket_client_send_text(client, stringJsonSendBuffer, strlen(stringJsonSendBuffer), portMAX_DELAY) == -1){
        ESP_LOGE(TAG, "ESP client could not send initial choices!");
    }
    else{
        ESP_LOGI(TAG, "ESP client sent inital choices successfully");
    }
    cJSON_DeleteItemFromObjectCaseSensitive(sharedJsonSendBuffer, "R");
    cJSON_DeleteItemFromObjectCaseSensitive(sharedJsonSendBuffer, "P");
    cJSON_DeleteItemFromObjectCaseSensitive(sharedJsonSendBuffer, "S");
    free(stringJsonSendBuffer);
}

RPS_Display_Element_t StartPage[4] = {
    {
        .ElementType = symbol,
        .RPS_Coord.x = 10,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = rockMap,
        .SpecificData.Symbol.TargetPage =  PickPg,
    },
    {
        .ElementType = symbol,
        .RPS_Coord.x = 50,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = rockMap,
        .SpecificData.Symbol.TargetPage =  PickPg,
    },
    {
        .ElementType = symbol,
        .RPS_Coord.x = 90,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = rockMap,
        .SpecificData.Symbol.TargetPage =  PickPg,
    },
    {
        .ElementType = button,
        .RPS_Coord.x = 40,
        .RPS_Coord.y = 60,
        .SpecificData.Button.Font = u8g2_font_6x13_tf,
        .SpecificData.Button.Text = "Send now",
        .SpecificData.Button.TargetPage =  WaitPg,
        .SpecificData.Button.SpecificCb = &StartPageCallback,
    },
};

RPS_Display_Element_t PickPage[4] = {
    {
        .ElementType = symbol,
        .RPS_Coord.x = 10,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = rockMap,
        .SpecificData.Symbol.TargetPage = StartPg,
    },
    {
        .ElementType = symbol,
        .RPS_Coord.x = 50,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = paperMap,
        .SpecificData.Symbol.TargetPage = StartPg,
    },
    {
        .ElementType = symbol,
        .RPS_Coord.x = 90,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = scissorMap,
        .SpecificData.Symbol.TargetPage = StartPg,
    },

    {
        .ElementType = infobox,
        .RPS_Coord.x = 20,
        .RPS_Coord.y = 60,
        .SpecificData.Infobox.Font = u8g2_font_6x13_tf,
        .SpecificData.Infobox.Text = "Press to select"
    },
};

RPS_Display_Element_t ResultPage[2] = {
    {
        .ElementType = infobox,
        .RPS_Coord.x = 25,
        .RPS_Coord.y = 30,
        .SpecificData.Infobox.Font = u8g2_font_10x20_tf,
        .SpecificData.Infobox.Text = "FOOBAR"
    },

    {
        .ElementType = button,
        .RPS_Coord.x = 45,
        .RPS_Coord.y = 60,
        .SpecificData.Button.Font = u8g2_font_6x13_tf,
        .SpecificData.Button.Text = "Replay",
        .SpecificData.Button.TargetPage = StartPg,
        .SpecificData.Button.SpecificCb = NULL,
    },
};

RPS_Display_Element_t WaitPage[2] = {
    {
        .ElementType = infobox,
        .RPS_Coord.x = 10,
        .RPS_Coord.y = 30,
        .SpecificData.Button.Font = u8g2_font_10x20_tf,
        .SpecificData.Button.Text = "Waiting for"
    },
        {
        .ElementType = infobox,
        .RPS_Coord.x = 10,
        .RPS_Coord.y = 50,
        .SpecificData.Button.Font = u8g2_font_10x20_tf,
        .SpecificData.Button.Text = "server..."
    },
};

RPS_Display_Element_t FinalePage[3] = {
        {
        .ElementType = symbol,
        .RPS_Coord.x = 10,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = rockMap,
        .SpecificData.Symbol.TargetPage = WaitPg,
    },
    {
        .ElementType = symbol,
        .RPS_Coord.x = 50,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = paperMap,
        .SpecificData.Symbol.TargetPage = WaitPg,
    },
    {
        .ElementType = symbol,
        .RPS_Coord.x = 90,
        .RPS_Coord.y = 5,
        .SpecificData.Symbol.Bitmap = scissorMap,
        .SpecificData.Symbol.TargetPage = WaitPg,
    },
};

RPS_Display_Element_t* PageDictionary[] = {
    StartPage, 
    PickPage, 
    WaitPage, 
    ResultPage,
    FinalePage,
};

RPS_Display_Element_t* DefaultHighlightElementDictionary[] = {
    StartPage, 
    PickPage, 
    WaitPage, 
    &ResultPage[1],
    FinalePage,
};

uint8_t LengthDictionary[] = {
    sizeof(StartPage)/sizeof(StartPage[0]), 
    sizeof(PickPage)/sizeof(PickPage[0]), 
    sizeof(WaitPage)/sizeof(WaitPage[0]), 
    sizeof(ResultPage)/sizeof(ResultPage[0]),
    sizeof(FinalePage)/sizeof(FinalePage[0])
};

//probably needs to changed later to initially point to some element
RPS_Display_Element_t* SelectedElement;
RPS_Defined_Pages_t CurrentPage;

//note that u8g2 sendbuffer isnt included in this function. 
//Sendbuffer is only executed inside highlightselecteditem if the selected item actually changes
void RPS_DrawPage(u8g2_t* u8g2, RPS_Defined_Pages_t Page){
    //ESP_LOGI(TAG, "Startin page draw");
    u8g2_ClearBuffer(u8g2);
    RPS_Display_Element_t* TargetPage = PageDictionary[Page];
    uint8_t ElementNum = LengthDictionary[Page];
    for (uint8_t i = 0; i<ElementNum; i++){
        switch (TargetPage[i].ElementType){
            case infobox:
                u8g2_SetFont(u8g2, TargetPage[i].SpecificData.Infobox.Font);
                u8g2_DrawStr(u8g2, TargetPage[i].RPS_Coord.x, 
                    TargetPage[i].RPS_Coord.y, 
                    TargetPage[i].SpecificData.Infobox.Text);
            break;

            case button:
                u8g2_SetFont(u8g2, TargetPage[i].SpecificData.Button.Font);
                u8g2_DrawStr(u8g2, TargetPage[i].RPS_Coord.x, 
                    TargetPage[i].RPS_Coord.y, 
                    TargetPage[i].SpecificData.Button.Text);
            break;

            case symbol:
                u8g2_DrawXBM(u8g2, TargetPage[i].RPS_Coord.x, TargetPage[i].RPS_Coord.y, BITMAPSIZE, BITMAPSIZE, TargetPage[i].SpecificData.Symbol.Bitmap);
            break;
        }
    }
}

//takes direction and gives you new selected element (if any)
RPS_Display_Element_t* RPS_ProcessDirection(RPS_Direction_t JStickDir){
    RPS_Display_Element_t* ReturnedElement = SelectedElement;
    RPS_Display_Element_t* Candidate = PageDictionary[CurrentPage];
    uint8_t SmallestDistance = MAXDISTANCE;
    //set candidate as first element in currentpage
    switch (JStickDir){
        case none:
        return SelectedElement;
        break;

        case left:
        //iterate over every element within page, then
        //get the index of element closest to the selected element whilst having a smaller x.
        for (uint8_t i = 0; i<LengthDictionary[CurrentPage]; i++){
            if(Candidate->ElementType == infobox){
                continue;
            }
            if (Candidate->RPS_Coord.x < SelectedElement->RPS_Coord.x){
                uint8_t CurrentDistance = SelectedElement->RPS_Coord.x - Candidate->RPS_Coord.x;
                if (CurrentDistance<SmallestDistance){
                    SmallestDistance = CurrentDistance;
                    ReturnedElement = Candidate;
                }
            }
            Candidate++;
        }
        return ReturnedElement;
        break;

        case right:
        for (uint8_t i = 0; i<LengthDictionary[CurrentPage]; i++){
            if(Candidate->ElementType == infobox){
                continue;
            }
            if (Candidate->RPS_Coord.x > SelectedElement->RPS_Coord.x){
                uint8_t CurrentDistance = Candidate->RPS_Coord.x - SelectedElement->RPS_Coord.x ;
                if (CurrentDistance<SmallestDistance){
                    SmallestDistance = CurrentDistance;
                    ReturnedElement = Candidate;
                }
            }
            Candidate++;
        }
        return ReturnedElement;
        break;

        case up:
        for (uint8_t i = 0; i<LengthDictionary[CurrentPage]; i++){
            if(Candidate->ElementType == infobox){
                continue;
            }
            if (Candidate->RPS_Coord.y > SelectedElement->RPS_Coord.y){
                uint8_t CurrentDistance = Candidate->RPS_Coord.y - SelectedElement->RPS_Coord.y ;
                if (CurrentDistance<SmallestDistance){
                    SmallestDistance = CurrentDistance;
                    ReturnedElement = Candidate;
                }
            }
            Candidate++;
        }
        return ReturnedElement;
        break;

        case down:
        for (uint8_t i = 0; i<LengthDictionary[CurrentPage]; i++){
            if(Candidate->ElementType == infobox){
                continue;
            }
            if (Candidate->RPS_Coord.y < SelectedElement->RPS_Coord.y){
                uint8_t CurrentDistance = SelectedElement->RPS_Coord.y - Candidate->RPS_Coord.y;
                if (CurrentDistance<SmallestDistance){
                    SmallestDistance = CurrentDistance;
                    ReturnedElement = Candidate;
                }
            }
            Candidate++;
        }
        return ReturnedElement;
        break;

        default:
        //should never reach here but if i dont have this the compiler complains
        ESP_LOGE(TAG, "Invalid movement direction was passed in");
        return SelectedElement;
    }
}

RPS_Direction_t RPS_ConvertCoordToDirection(int x, int y){
    if (y<1000){
        return left;
    }
    else if(x <1000){
        return up;
    }
    else if (y >3095){
        return right;
    }
    else if (x>3095){
        return down;
    }
    else{
        return none;
    }
}

//highlights selected item and sends buffer
void RPS_HighlightSelectedItem(u8g2_t* u8g2){
    static uint8_t firstRun = 1;
    static RPS_Display_Element_t* lastSelectedElement;

    if (firstRun){
        lastSelectedElement = SelectedElement;
        firstRun = 0;
        goto HighlightingBusiness;
    }

    if (lastSelectedElement == SelectedElement){
        return;
    }
    else{
        lastSelectedElement = SelectedElement;
    }

    if (SelectedElement->ElementType == infobox){
        //return and do nothing
        ESP_LOGI(TAG, "Selected infobox, no highlight.");
        u8g2_SendBuffer(u8g2);
        return;
    }

    HighlightingBusiness:
    switch (SelectedElement->ElementType){
        case button:
        u8g2_SetFont(u8g2, SelectedElement->SpecificData.Button.Font);
        u8g2_DrawButtonUTF8(u8g2, 
            SelectedElement->RPS_Coord.x,
            SelectedElement->RPS_Coord.y,
            U8G2_BTN_BW1,
            0,
            2,
            2,
            SelectedElement->SpecificData.Button.Text);
        break;
        case symbol:
        u8g2_DrawFrame(u8g2, SelectedElement->RPS_Coord.x, SelectedElement->RPS_Coord.y, BITMAPSIZE, BITMAPSIZE);
        break;
        default:
        ESP_LOGE(TAG, "invalid elementtype when highlighting");
        break;
    }
    u8g2_SendBuffer(u8g2);
}

//Called after every button press to executes any special callbacks. 
//Then draws their target page and places the selected element in the right place
void RPS_ButtonPressProcessing(u8g2_t* u8g2){
    //run specific callback if any, then general page logic

    static RPS_Display_Element_t* lastSymbolSelected;

    if (SelectedElement->ElementType == button){
        if (SelectedElement->SpecificData.Button.SpecificCb != NULL){
            SelectedElement->SpecificData.Button.SpecificCb();
            ESP_LOGI(TAG, "Button callback executed");
        } 
        CurrentPage = SelectedElement->SpecificData.Button.TargetPage;
    }
    else if (SelectedElement->ElementType == symbol){
        if (CurrentPage == StartPg){
            lastSymbolSelected = SelectedElement;
        }

        else if (CurrentPage == PickPg){
            lastSymbolSelected->SpecificData.Symbol.Bitmap = SelectedElement->SpecificData.Symbol.Bitmap;
        }

        else if (CurrentPage == FinalePg){
            //depending on the bitmap contained within symbol, send different websocket commands
            ESP_LOGI(TAG, "starting showdown send");
            if (cJSON_ReplaceItemInObjectCaseSensitive(sharedJsonSendBuffer, "status", cJSON_CreateString("showdown")) == 0){
                ESP_LOGE(TAG, "No status key in sharedjsonsendbuffer");
            }
            if (SelectedElement->SpecificData.Symbol.Bitmap == rockMap){
                cJSON_AddStringToObject(sharedJsonSendBuffer, "final", "R");

            }
            else if (SelectedElement->SpecificData.Symbol.Bitmap == paperMap){
                cJSON_AddStringToObject(sharedJsonSendBuffer, "final", "P");
            }
            else{
                cJSON_AddStringToObject(sharedJsonSendBuffer, "final", "S");
            }

            char* stringJsonSendBuffer = cJSON_Print(sharedJsonSendBuffer);
            if (esp_websocket_client_send_text(client, stringJsonSendBuffer, strlen(stringJsonSendBuffer), portMAX_DELAY) == -1){
                ESP_LOGE(TAG, "showdown send fail!");
            }
            else{
                ESP_LOGI(TAG, "showdown send success");
            }
            cJSON_DeleteItemFromObjectCaseSensitive(sharedJsonSendBuffer, "final");
            free(stringJsonSendBuffer);
        }

        else{
            //shouldnae reach here
            ESP_LOGI(TAG, "Selected element was unknown symbol");
        }
        CurrentPage = SelectedElement->SpecificData.Symbol.TargetPage;
    }
    else{
        //origin mustve been an infobox.
        ESP_LOGI(TAG, "no callbacks for infobox");
    }
    SelectedElement = DefaultHighlightElementDictionary[CurrentPage];
    RPS_DrawPage(u8g2, CurrentPage);
    RPS_HighlightSelectedItem(u8g2);
}