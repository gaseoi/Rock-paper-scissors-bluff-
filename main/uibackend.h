#ifndef UIBACKEND_H
#define UIBACKEND_H
#include <stdio.h>
#include "esp_log.h"
#include "u8g2_esp32_hal.h"
#include <cJSON.h>
#include "esp_websocket_client.h"
#include "bitmaps.h"

extern esp_websocket_client_handle_t client;

#define MAXDISTANCE 255

typedef struct{
    uint8_t x;
    uint8_t y;
} RPS_Coordinate_t;

typedef enum{
    infobox, button, symbol
}RPS_ElementType_t;

//making a dictionary to get the pointer to the end of a page
//provided we have the pointer to the start
typedef enum{
    StartPg, PickPg, WaitPg, ResultPg, FinalePg
}RPS_Defined_Pages_t;

typedef struct{
    char Text[30];
    const uint8_t* Font;
}RPS_InfoBox_t;

typedef struct{
    char Text[30];
    const uint8_t* Font;
    RPS_Defined_Pages_t TargetPage;
    void (*SpecificCb) (void);
}RPS_Button_t;

typedef struct{
    const uint8_t *Bitmap;
    RPS_Defined_Pages_t TargetPage;
}RPS_Symbol_t;

typedef struct{
    RPS_Coordinate_t RPS_Coord;
    union{
        RPS_InfoBox_t Infobox;
        RPS_Button_t Button;
        RPS_Symbol_t Symbol;
    } SpecificData;
    RPS_ElementType_t ElementType;
}RPS_Display_Element_t;

extern RPS_Display_Element_t StartPage[4];
extern RPS_Display_Element_t PickPage[4];
extern RPS_Display_Element_t WaitPage[2];
extern RPS_Display_Element_t ResultPage[2];
extern RPS_Display_Element_t FinalePage[3];


typedef enum{
    none, left, right, up, down
}RPS_Direction_t;

extern RPS_Display_Element_t* PageDictionary[];
extern RPS_Display_Element_t* DefaultHighlightElementDictionary[];
extern uint8_t LengthDictionary[];

extern RPS_Display_Element_t* SelectedElement;
extern RPS_Defined_Pages_t CurrentPage;

extern cJSON* sharedJsonSendBuffer;

void RPS_DrawPage(u8g2_t* u8g2, RPS_Defined_Pages_t Page);
RPS_Display_Element_t* RPS_ProcessDirection(RPS_Direction_t JStickDir);
RPS_Direction_t RPS_ConvertCoordToDirection(int x, int y);
void RPS_HighlightSelectedItem(u8g2_t* u8g2);
void RPS_ButtonPressProcessing(u8g2_t* u8g2);

#endif