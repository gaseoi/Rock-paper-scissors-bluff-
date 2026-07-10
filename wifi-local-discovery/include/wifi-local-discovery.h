#ifndef WIFI_LOCAL_DISCOVERY_H
#define WIFI_LOCAL_DISCOVERY_H

#include "esp_netif.h"

void wifi_init_sta(char* ssid, char* pass);
void esp_mdns_discovery_start(char*hostname, char*instancename);
struct esp_ip4_addr resolve_mdns_host(const char * host_name);

#endif