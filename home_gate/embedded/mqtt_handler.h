// Logic functions

void toggle_gate() {
    // One pulse of radio waves
    gpio_set_level(GPIO_OUTPUT_IO_0, 1);
    gpio_set_level(GPIO_OUTPUT_IO_1, 1);
    vTaskDelay(2500 / portTICK_RATE_MS); // Pulse for 2.5s 
    gpio_set_level(GPIO_OUTPUT_IO_0, 0);
    gpio_set_level(GPIO_OUTPUT_IO_1, 0);
}

void hold_gate(int32_t milisecs) {
    // One pulse of radio waves
    gpio_set_level(GPIO_OUTPUT_IO_0, 1);
    gpio_set_level(GPIO_OUTPUT_IO_1, 1);
    vTaskDelay(500 / portTICK_RATE_MS); // Pulse for 500ms
    milisecs -= 500;
    
    gpio_set_level(GPIO_OUTPUT_IO_0, 0);
    gpio_set_level(GPIO_OUTPUT_IO_1, 0);

    if (milisecs > 0)
    	vTaskDelay(milisecs / portTICK_RATE_MS); // Hold the gate

    // Another pulse of radio waves
    gpio_set_level(GPIO_OUTPUT_IO_0, 1);
    gpio_set_level(GPIO_OUTPUT_IO_1, 1);
    vTaskDelay(1000 / portTICK_RATE_MS); // Pulse for 500ms
    
    gpio_set_level(GPIO_OUTPUT_IO_0, 0);
    gpio_set_level(GPIO_OUTPUT_IO_1, 0);
}

static int process_event_data(esp_mqtt_event_handle_t event) {
    int32_t milisecs;	
	
    //if (strncmp(event->data, "send binary please", event->data_len) == 0) {
    //    ESP_LOGI(TAG, "Sending the binary");
    //    send_binary(client);
    //    return;
    //}

    if (strncmp(event->topic, "/topic/gate", event->topic_len) == 0) {
    	if (strncmp(event->data, "toggle_gate", event->data_len) == 0) {
            ESP_LOGI(TAG, "Received toggle gate!");
	    toggle_gate();
	    return 1;
	}

	sscanf(event->data, "%d", &milisecs);
        ESP_LOGD(TAG, "Toggling gate for %dms", milisecs);
	hold_gate(milisecs);
	return 1;
    }

    return 0;
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);

        // Subscribe to our topic of interest as well
	    msg_id = esp_mqtt_client_subscribe(client, "/topic/gate", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");

        // Reconnect in case of disconnection
        esp_mqtt_client_reconnect(client);

        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
	
	// Actual logic LUL
	if (process_event_data(event)) {
            ESP_LOGI(TAG, "GATE CONFIRM: OK!");
	    esp_mqtt_client_publish(client, "/topic/gate_confirm", "ok", 0, 0, 0);
	} else {
            ESP_LOGI(TAG, "GATE CONFIRM: ERROR!");
	    esp_mqtt_client_publish(client, "/topic/gate_confirm", "error", 0, 0, 0);
	}

	break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}
