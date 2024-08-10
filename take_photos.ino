#include "esp_camera.h"
// #include "FS.h"
// #include "SD.h"
// #include "SPI.h"

#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM

#include "camera_pins.h"

void photo_save() {
  /*
  typedef struct {
    uint8_t * buf;              // Pointer to the pixel data
    size_t len;                 // Length of the buffer in bytes
    size_t width;               // Width of the buffer in pixels
    size_t height;              // Height of the buffer in pixels
    pixformat_t format;         // Format of the pixel data
    struct timeval timestamp;   // Timestamp since boot of the first DMA buffer of the frame
  } camera_fb_t;
  */

  /*
  // esp_camera.h -> sensor.h  
  typedef enum {
      PIXFORMAT_RGB565,    // 2BPP/RGB565, 0
      PIXFORMAT_YUV422,    // 2BPP/YUV422, 1
      PIXFORMAT_YUV420,    // 1.5BPP/YUV420, 2
      PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE, 3
      PIXFORMAT_JPEG,      // JPEG/COMPRESSED, 4
      PIXFORMAT_RGB888,    // 3BPP/RGB888, 5
      PIXFORMAT_RAW,       // RAW, 6
      PIXFORMAT_RGB444,    // 3BP2P/RGB444, 7
      PIXFORMAT_RGB555,    // 3BP2P/RGB555, 8
  } pixformat_t;
  */
  
  camera_fb_t * fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.println("Failed to get camera frame buffer");
    return;
  }

  Serial.println(fb->height);
  Serial.println(fb->width);
  Serial.println(fb->len);
  Serial.println(fb->format);

  // Release image buffer
  esp_camera_fb_return(fb);
  Serial.println("Photo saved to file");
}


void setup() {
  Serial.begin(115200);
  while(!Serial); // When the serial monitor is turned on, the program starts to execute
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
  config.pixel_format = PIXFORMAT_RGB565; // RGB format

  // Frame parameters
  config.frame_size = FRAMESIZE_VGA; // Start with VGA resolution
  config.jpeg_quality = 10; // Low quality for faster transfer
  config.fb_count = 1; // Number of frame buffers
  config.fb_location = CAMERA_FB_IN_PSRAM; // Use PSRAM for frame buffers
  
  // Initialize the camera
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }
  
  Serial.println("Camera init success");
}

const char *ASCII_CHARS = " .:-=+*#%@";


void loop() {
  // Capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Process the frame
  Serial.printf("Captured frame %dx%d, size: %d bytes\n", fb->width, fb->height, fb->len);

  // Access the image data
  uint8_t* image_data = fb->buf;

  // Iterate over all pixels to convert to grayscale and print as ASCII
  for (int y = 0; y < fb->height; y += 8) { // Adjust step to reduce output size
    for (int x = 0; x < fb->width; x += 4) { // Adjust step to reduce output size
      uint16_t pixel = image_data[(y * fb->width + x) * 2] | (image_data[(y * fb->width + x) * 2 + 1] << 8); // Combine two bytes
      uint8_t r = (pixel >> 11) & 0x1F;
      uint8_t g = (pixel >> 5) & 0x3F;
      uint8_t b = pixel & 0x1F;

      // Convert RGB565 values to 8-bit values
      r = (r << 3) | (r >> 2);
      g = (g << 2) | (g >> 4);
      b = (b << 3) | (b >> 2);

      // Calculate grayscale value
      uint8_t gray = (r * 30 + g * 59 + b * 11) / 100;

      // Map grayscale value to ASCII character
      char ascii_char = ASCII_CHARS[gray / 25]; // 256/10 ~ 25

      // Print ASCII character
      Serial.print(ascii_char);
    }
    Serial.println(); // Newline at the end of each row
  }

  // Return the frame buffer back to the driver
  esp_camera_fb_return(fb);

  // Delay before capturing next frame
  delay(1000); // 1 second delay

}