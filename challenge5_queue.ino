#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t queue_len = 5;
static QueueHandle_t queue_1;

void controlBlinkRate(void *parameter){
  
}

void printMessages(void *parameter){
  int item;

  while(1){
    if(xQueueReceive(queue_1, (void *)&item, 0)==pdTRUE){
      //Serial.println(item);
    }
    Serial.println(item);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);

  queue_1 = xQueueCreate(msg_queue_len, sizeof(int));
}

void loop() {
  static int num = 100;
  if(xQueueSend(msg_queue, (void *)&num, 10) != pdTRUE){
    Serial.println("Queue full!");
  }
  num *= 2;
  vTaskDelay(10000/portTICK_PERIOD_MS);
}
