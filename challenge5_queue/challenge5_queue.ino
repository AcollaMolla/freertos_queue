#include <LiquidCrystal_I2C.h>
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
static int blink_rate = 1000;

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
  int numbers[5] = {-1, -1, -1, -1, -1};

  for(int i=5;i<=10;i++){
    if((int)buf[i] < 58 && (int)buf[i] > 47){
      numbers[factor] = ((int)buf[i]) - 48;
      factor++;
    }
  }
  delay = numbers[0];
  if(delay <= 0){
    Serial.println("[ERROR]Value must be > 0!");
    return blink_rate; 
  }
  else{
    for(int i=1;i<5;i++){
      if(numbers[i] >= 0){
        delay *= 10;
        delay += numbers[i];
      }
    }
  }
  return delay;
}

void BlinkLED(void *parameter){
  int count = 0, total_count = 0;
  char msg[9] = "Blinked ";
  char msg2[8] = " times!";
  char dest[31];
  char countstr[16];

  while(1){
    if(count == 100){
      total_count += count;
      itoa(total_count, countstr, 10);
      strcpy(dest, msg);
      strcat(dest, countstr);
      strcat(dest, msg2);
      xQueueSend(queue_2, (void *)&(dest), 0);
      count = 0;
    }
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, HIGH);
    vTaskDelay(blink_rate/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(blink_rate/portTICK_PERIOD_MS);
    count++;
  }
}

void ReadFromQueue1(void *parameter){
  int buf;
  
  
  while(1){
    if(xQueueReceive(queue_1, (void *)&buf, 0)==pdTRUE){
      Serial.print("[INFO]Blink rate: ");
      Serial.println(buf);
      blink_rate = buf;
    }
  }
}

void PrintFromQueue2(void *parameter){
  char buf[buf_len];
  int delay = 1000;
  int lcdColumns = 16;
  int lcdRows = 2;
  LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  
  while(1){
    if(xQueueReceive(queue_2, (void *)&buf, 0)==pdTRUE){
      if(checkStringForDelayCommand(buf)){
        delay = parseDelayCommand(buf);
        xQueueSend(queue_1, (void *)&delay, 0);
       }
      else{
        Serial.print("Echo: ");
        Serial.println(buf);
        lcd.print(buf);
        lcd.setCursor(9,0);
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

  queue_1 = xQueueCreate(queue_len, sizeof(int));
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
   1500,
   NULL,
   1,
   NULL,
   app_cpu);

 xTaskCreatePinnedToCore(ReadFromQueue1,
  "Adjust LED blink rate according to value in Queue 1",
  1024,
  NULL,
  1,
  NULL,
  app_cpu);

xTaskCreatePinnedToCore(BlinkLED,
 "Blink LED",
 1224,
 NULL,
 1,
 NULL,
 app_cpu);
}

void loop() {

}
