# Rock-paper-scissors-bluff-
Backup of a personal project using espidf to create a rock paper scissors-esque game over websockets

The game works by 2 players each selecting 3 cards among any combination of rock, paper, or scissors. The cards are sent over websockets to a server, which randomises the cards and returns 3 randomised cards to the players. The players then plays a round of rock, paper, scissors by selecting a card to duel with and sending it to the server. The server then declares who is the winner and who is the loser. 

Im using SSD1306 display driven by u8g2 and u8g2_esp32_hal. The joystick controlled GUI logic is otherwise entirely implemented by me. I used a raspberry pi zero W to host the server using python but any windows/linux based pcs should be fine as long as you have python and include the websockets library via:

```pip install websockets ```

To try for yourself, simply create a new espidf project and import main.c, bitmaps.c/h, and backend.c/h into the project main folder, and include the wifi-local-discovery component which handles the websocket server connection and wifi connection. Be sure to substitute your own network credentials where relevant in the wifi_init_sta(name, pass) function and resolve_mdns_host(hostname) function. You need to place the mdns domain name into the latter function without the .local part. Finally import u8g2 and u8g2 esp32 hal from github as components. 

Modify the existing CMakeLists.txt under main like so:
```
idf_component_register(SRCS "main.c" "bitmaps.c" "uibackend.c"
                    INCLUDE_DIRS "."
                    REQUIRES u8g2 u8g2-hal-esp-idf wifi-local-discovery esp_driver_i2c esp_adc)
```

Then open an espidf terminal and run the following commands so that the compiler can find the necessary dependencies
```
idf.py add-dependency "espressif/mdns"
idf.py add-dependency "espressif/cjson"
idf.py add-dependency "espressif/esp_websocket_client"
 ```
