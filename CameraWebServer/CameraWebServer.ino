/*
 * From https://randomnerdtutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/
 * 
 * Not using OTA ==> use with board AI Thinker ESP32-CAM
 * 
 * Using OTA:
 * (From https://www.esp32.com/viewtopic.php?t=14700#p65758)
 * - Use ESP32 dev module instead of AI Thinker ESP32-Cam
 * - Configure Partition Scheme to default 4MB with spiffs
 * - Enable PSRAM
 * and it works using this (not sure if it is necessary):
 * - Upload Speed: 921600
 * - CPU Frequency: 240MHz (WiFi/BT)
 * - Flash Frequency: 80MHz
 * - Flash Mode: QIO
 * - Flash Size: 4MB (32Mb)
 * - Core Debug Level: None
 * 
 * OTA only works during boot (40 seconds) to not increase the delay in the stream
 * 
 * URL: http://<ip address>:80
 * URL: http://<ip address>:81/stream
 */
 
#define DEBUG_____

/*
 * GPIO assignment
 * 0  - XCLK
 * 1
 * 2
 * 3
 * 4  - LED_FLASH
 * 5  - Y2
 * 6
 * 7
 * 8
 * 9
 * 10
 * 11
 * 12
 * 13
 * 14
 * 15
 * 16
 * 17
 * 18 - Y3
 * 19 - Y4
 * 20
 * 21 - Y5
 * 22 - PCKL
 * 23 - HREF
 * 24
 * 25 - VSYNC
 * 26 - SIOD
 * 27 - SIOC
 * 28
 * 29
 * 30
 * 31
 * 32 - PWDN
 * 33 - LED
 * 34 - Y8
 * 35 - Y9
 * 36 - Y6
 * 37
 * 38
 * 39 - Y7
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
// 1 #include <BluetoothSerial.h>
#include <esp_camera.h>
#include <esp_http_server.h>
#include <esp_timer.h>
#include <ESPmDNS.h>
#include <fb_gfx.h>
#include <img_converters.h>
#include <soc/soc.h>            // disable brownout problems
#include <soc/rtc_cntl_reg.h>   // disable brownout problems
#include <WiFi.h>
#include <WiFiUdp.h>
#include <credentials.h>        // My WiFi credentials

const char *ssid = MY_SSID;
const char *password = MY_PASSWORD;
#define OTA_HOSTNAME  "CameraWideOTA"

/*
 * Camera
 */
#define PART_BOUNDARY "123456789000000000000987654321"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
  
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

/*
 * LED
 */
#define LED_PIN 33
/*
 * Macros
 */

#ifdef DEBUG_____
#define myprint(x)    {Serial.print(x);   client.print(x);}
#define myprintln(x)  {Serial.println(x); client.println(x);}
#else
#define myprint(x)    {client.print(x);}
#define myprintln(x)  {client.println(x);}
#endif

#ifdef NCE
#define HOST "10.10.15.222"
#define PORT (8752)
#else
#define HOST "10.0.0.101"
#define PORT (8752)
#endif

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>ESP32-CAM Robot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
  <body>
    <center>
      <img src="" id="photo" style="width: auto; max-width: 100%; height: auto;">
    </center>
    <script>
      document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
    </script>
  </body>
</html>
)rawliteral";

/*
 * New types
 */
typedef unsigned long MY_TIME_T;

/*
 * Prototypes
 */
void myDelay(MY_TIME_T waitTime);

/*
 * To enable Bluetooth serial monitor (from Bluetooth example)
 */
// 1 BluetoothSerial SerialBT;


// WiFi console
WiFiClient client;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);

// 1  SerialBT.begin("RobotCar"); //Bluetooth device name
  
  // Wi-Fi connection
/* 1
  WiFi.mode(WIFI_STA);
  Serial.printf("Usando SSDI %s and password %s\n", ssid, password);
  WiFi.begin(ssid, password);
  while(WiFi.waitForConnectResult() != WL_CONNECTED) {
    myprintln("Connection Failed! Try a new SSID/Password via Bluetooth.");
    String ssid = readSSID();
    String password = readPassword();
    myprint("Trying with "); myprint(ssid.c_str()); myprint(" and "); myprintln(password.c_str()); 
    SerialBT.print("Trying with "); SerialBT.print(ssid.c_str()); SerialBT.print(" and "); SerialBT.println(password.c_str()); 
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    delay(5000);
//    myprintln("Connection Failed! Rebooting...");
//    ESP.restart();
  }
  myprintln("\nWiFi connected");
*/

  // Connect to Wi-Fi network
  WiFi.mode(WIFI_STA);
#ifdef NCE
  WiFi.begin(ssid);
#else
  WiFi.begin(ssid, password);
#endif
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("Connection Failed! Rebooting...");
    delay(10000);
    ESP.restart();
  }
  myprintln("Connected to WiFi");

  // Arduino OTA
  ArduinoOTA.setHostname(OTA_HOSTNAME);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      myprintln("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
    printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      myprintln("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      myprintln("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      myprintln("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      myprintln("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      myprintln("End Failed");
    }
  });
  ArduinoOTA.begin();
  myprintln("Ready!");

  // WiFi console
  if(!client.connect(HOST, PORT)) {
    myprint("Could not connect to WiFi console at ");
  } else {
    myprint("Connected to WiFi console at ");  
  }
  myprint(HOST);
  myprint(":");
  myprintln(PORT);
  delay(1000);

  myprintln("Waiting for program upload command.");
  int ledState = LOW;             // ledState used to set the LED
  pinMode(LED_PIN, OUTPUT);
  for(int i=0; i<40; i++) {
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(LED_PIN, ledState);
    //ArduinoOTA.handle();  // remember!!! myDelay ***MUST*** call ArduinoOta.handle()
    myDelay(1000L);
  }
  myprintln("No program upload command received.");  

  myprintln("Booting");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  myprint("Camera Stream Ready! Go to: http://");
  myprintln(WiFi.localIP());
  
  // Start streaming web server
  startCameraServer();
}

void loop() {
  //ArduinoOTA.handle();
  //myDelay(1000);
}

static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    //ArduinoOTA.handle();

    fb = esp_camera_fb_get();
    if (!fb) {
      myprintln("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            myprintln("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  sensor_t * s = esp_camera_sensor_get();
  int res = 0;
  
  if(res){
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };
  
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

/**
 * Reads a SSID via Bluetooth
 * Parameters:
 *  None
 * Returns:
 *  On success, the SSID as String
 *  On error, (does not return)
 */
/* 1
String readSSID(void) {
  String ssid;

  SerialBT.print("Entre a new SSID: ");
  do {
    delay(200);
    ssid = SerialBT.readString();
  } while(ssid.length()==0);
  ssid.trim();
  return ssid;
}
*/

/**
 * Reads a SSID password via Bluetooth.
 * Parameters:
 *  None
 * Returns:
 *  On success, the SSID password as String
 *  On error, (does not return)
 */
/* 1
String readPassword(void) {
  String password;
  
  SerialBT.print("Entre a new password: ");
  do {
    delay(200);
    password = SerialBT.readString();
  } while(password.length()==0);
  password.trim();
  return password;
}
*/

/**
 * myDelay: busy-wait delay compatible with OTA
 * Parameters:
 *  waitTime: unsigned long delay time
 * Returns:
 *  void
*/
void myDelay(MY_TIME_T waitTime) {
  MY_TIME_T previousMillis;
  previousMillis = (MY_TIME_T)millis();
  while((MY_TIME_T)millis() - previousMillis < waitTime) {
    ArduinoOTA.handle();
    delay(10);
  }
}
