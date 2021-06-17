#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int led_pin = LED_BUILTIN;
static const uint8_t queue_len = 5;
static const uint8_t buf_len = 255;
static QueueHandle_t queue_1;
static QueueHandle_t queue_2;

bool checkStringForDelayCommand(char buf[buf_len]){
  int sum = 0;
  for(int i=0;i<5;i++){
    sum += (int)buf[i];
  }

  if(sum == 527)return true;
  return false;
}

int parseDelayCommand(char buf[buf_len]){
  int delay = 0;
  int factor = 0;
  int numbers[4] = {-1, -1, -1, -1};

  for(int i=5;i<=9;i++){
    if((int)buf[i] < 58 && (int)buf[i] > 47){
      //Serial.println((int)buf[i] - 48);
      numbers[factor] = ((int)buf[i]) - 48;
      factor++;
    }
  }

  Serial.println(numbers[0]);
  Serial.println(numbers[1]);
  Serial.println(numbers[2]);
  Serial.println(numbers[3]);

  delay = numbers[0];
  if(delay==0)
    return 1000;
  else{
    for(int i=1;i<4;i++){
      if(numbers[i] >= 0){
        delay *= 10;
        delay += numbers[i];
      }
    }
  }
  return delay;
}

void controlBlinkRate(void *parameter){
  pinMode(led_pin, OUTPUT);
  int rate = 1000;
  
  while(1){
    if(xQueueReceive(queue_1, (void *)&rate, 0)==pdTRUE){}
    Serial.print("Blinking for ");
    Serial.print(rate);
    Serial.println(" ms");
    digitalWrite(led_pin, HIGH);
    vTaskDelay(rate/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
  }
}

void PrintFromQueue2(void *parameter){
  char buf[buf_len];
  int delay = 1000;
  
  while(1){
    if(xQueueReceive(queue_2, (void *)&buf, 0)==pdTRUE){
      if(checkStringForDelayCommand(buf)){
        Serial.println("Delay command!");
        delay = parseDelayCommand(buf);
        Serial.print("Parsed delay: ");
        Serial.println(delay);
       }
      else{
        Serial.print("Echo: ");
        Serial.println(buf);
      }
    }
  }
}

void readUserInput(void *parameter){
  char c;
  char buf[buf_len];
  uint8_t idx = 0;
  int delay = 1000;

  while(1){
    if(Serial.available() > 0){
      c = Serial.read();
      if(idx < buf_len-1){
        buf[idx] = c;
        idx++;
      }
      if(c == '\n'){
        buf[idx-1] = '\0';
        xQueueSend(queue_2, (void *)&buf, 0);
        memset(buf, 0, buf_len);
        idx = 0;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);

  queue_1 = xQueueCreate(queue_len, sizeof(char[255]));
  queue_2 = xQueueCreate(queue_len, sizeof(char[255]));

  xTaskCreatePinnedToCore(readUserInput,
    "Read user input",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(PrintFromQueue2,
   "Print user input stored in Queue 2",
   1024,
   NULL,
   1,
   NULL,
   app_cpu);
}

void loop() {
  /*static int num = 100;
  if(xQueueSend(queue_1, (void *)&num, 10) != pdTRUE){
    Serial.println("Queue full!");
  }
  num *= 2;
  vTaskDelay(10000/portTICK_PERIOD_MS);*/
}
