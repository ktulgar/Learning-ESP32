#include <stdio.h> 
#include <string.h> 
#include "esp_wifi.h" 
#include "esp_event.h" 
#include "nvs_flash.h" 
#include "lwip/sockets.h"
#include "driver/gpio.h"
#include "soc/gpio_num.h"


uint8_t incoming_message[25];
uint8_t led_status = 0; 
uint8_t status = 0;
uint8_t trial = 0;

const char *wifi_name = "Name Of WIFI That You want to connect";
const char *password  = "WIFI's password";

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data)
{
	
    if(event_id == WIFI_EVENT_STA_START)
    {
      printf("... WIFI is connecting ... \n");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
     printf("... WIFI Connected ... \n");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
     printf("... WIFI lost the connection ... \n");
     if(trial<5)
       {esp_wifi_connect();
        trial++;
        printf("... Retrying to Connect... \n");
       }
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
     printf("Wifi got IP...\n\n");
     status = 1;
    }
    
}

void wifi_connection()
{

    esp_netif_init();
    esp_event_loop_create_default();    
    esp_netif_create_default_wifi_sta(); 
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //     
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    wifi_config_t wifi_configuration = {0};
    strcpy((char*)wifi_configuration.sta.ssid, wifi_name);
    strcpy((char*)wifi_configuration.sta.password, password);    

    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);

    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_connect();
 
}

void app_main(void)
{
   nvs_flash_init();
   wifi_connection();
   
   gpio_reset_pin(GPIO_NUM_2);
   gpio_set_direction(GPIO_NUM_2,GPIO_MODE_OUTPUT);
   gpio_set_level(GPIO_NUM_2, led_status);
 
   while(status != 1);
   
   struct sockaddr_in myaddr;
   myaddr.sin_addr.s_addr = inet_addr("192.168.1.44");
   myaddr.sin_family = AF_INET; //  IPV4 
   myaddr.sin_port = htons(1881);
   
   // AF_INET     => IPV4 
   // SOCK_STREAM => TCP
   int socket_fd =  socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
   
   if(socket_fd < 0) 
   {
	   printf("... Socket could not be created ... \n");
   }
   
   int ret_value = connect(socket_fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
   
   if(ret_value < 0) 
   {
	   printf("... Socket could not connect into the server  ... \n");
   }
   
   while(1) 
   {
	   uint8_t len = 0;
	   len = recv(socket_fd,  incoming_message ,sizeof(incoming_message) - 1,0);
	   
	   if(len > 0 && len < 255)
	   {
		  printf("%s\n",incoming_message);
		  if(strcmp("Toggle", (const char *) incoming_message) == 0 ) 
		  {
			  led_status = !led_status;
			  gpio_set_level(GPIO_NUM_2, led_status);
		  }
		  
		 memset(incoming_message,0,strlen((const char *) incoming_message));
		 
	   }

	   vTaskDelay(pdMS_TO_TICKS(250));
	   
   }
    
    
}