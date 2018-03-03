#ifndef DISPLAY_SSD1306_H
#define DISPLAY_SSD1306_H

struct Glyph {
  int8_t skip;
  int8_t xoffset;
  int8_t yoffset;
  uint8_t columns;
  const uint32_t* data;
};

const uint32_t BatteryBar16_data[] = {
   0b00000000000000000000000000000000UL,
   0b00000111111111111111111111100000UL,
   0b00011111111111111111111111111000UL,
   0b00111111111111111111111111111100UL,
   0b01111111111111111111111111111110UL,
   0b11111111111111111111111111111111UL,
   0b11111111111111111111111111111111UL,
   0b11111111111111111111111111111111UL,
   0b11111111111111111111111111111111UL,
   0b11111111111111111111111111111111UL,
   0b11111111111111111111111111111111UL,
   0b01111111111111111111111111111110UL,
   0b00111111111111111111111111111100UL,
   0b00011111111111111111111111111000UL,
   0b00000111111111111111111111100000UL,
};

#define GLYPHDATA(X) NELEM(X), X

const Glyph BatteryBar16 = { 16, 0, 0, GLYPHDATA(BatteryBar16_data) };

#include "StarJedi10Font.h"

#define FONT_NAME Starjedi10pt7bGlyphs
#include "saber_logo.h"
const Glyph SaberLogo = { 0, 0, 0, GLYPHDATA(saberLogoLS6) };
class SSD1306 : public I2CDevice, Looper, StateMachine, SaberBase, CommandParser {
public:
  static const int WIDTH = 128;
  static const int HEIGHT = 32;
  const char* name() override { return "SSD1306"; }

  enum Commands {
    SETCONTRAST = 0x81,
    DISPLAYALLON_RESUME = 0xA4,
    DISPLAYALLON = 0xA5,
    NORMALDISPLAY = 0xA6,
    INVERTDISPLAY = 0xA7,
    DISPLAYOFF = 0xAE,
    DISPLAYON = 0xAF,

    SETDISPLAYOFFSET = 0xD3,
    SETCOMPINS = 0xDA,

    SETVCOMDETECT = 0xDB,

    SETDISPLAYCLOCKDIV = 0xD5,
    SETPRECHARGE = 0xD9,

    SETMULTIPLEX = 0xA8,

    SETLOWCOLUMN = 0x00,
    SETHIGHCOLUMN = 0x10,

    SETSTARTLINE = 0x40,

    MEMORYMODE = 0x20,
    COLUMNADDR = 0x21,
    PAGEADDR   = 0x22,

    COMSCANINC = 0xC0,
    COMSCANDEC = 0xC8,

    SEGREMAP = 0xA0,

    CHARGEPUMP = 0x8D,

    EXTERNALVCC = 0x1,
    SWITCHCAPVCC = 0x2,

    // Scrolling commands
    ACTIVATE_SCROLL = 0x2F,
    DEACTIVATE_SCROLL = 0x2E,
    SET_VERTICAL_SCROLL_AREA = 0xA3,
    RIGHT_HORIZONTAL_SCROLL = 0x26,
    LEFT_HORIZONTAL_SCROLL = 0x27,
    VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL = 0x29,
    VERTICAL_AND_LEFT_HORIZONTAL_SCROLL = 0x2A,
  };

  enum Screen {
    SCREEN_STARTUP,
    SCREEN_SAVER,
    SCREEN_MESSAGE,
    SCREEN_PLI,
  };

  SSD1306() : I2CDevice(0x3C), CommandParser() {}
  void Send(int c) { writeByte(0, c); }

 void DrawPixel(int x, int y, uint8_t color){

    if (y<0 || x< 0) return;
    if (y>HEIGHT || x> WIDTH) return;
 
  switch (color)
    {
      case WHITE:   frame_buffer_[x] |= 0x1<<y; break;
      case BLACK:    frame_buffer_[x] &= ~(0x1 << y); break;
      case INVERSE:  frame_buffer_[x] ^= (0x1 << y); break;
    }
 }
  void Draw(const Glyph& glyph, int x, int y) {
    x += glyph.xoffset;
    y += glyph.yoffset;
    int begin = max(0, -x);
    int end = min(glyph.columns, WIDTH - x);
    uint32_t *pos = frame_buffer_ + x;
    if (y > 0) {
      for (int i = begin; i < end; i++) pos[i] |= glyph.data[i] << y;
    } else if (y < 0) {
      for (int i = begin; i < end; i++) pos[i] |= glyph.data[i] >> -y;
    } else {
      for (int i = begin; i < end; i++) pos[i] |= glyph.data[i];
    }
  }

#ifndef _swap_uint8_t
#define _swap_uint8_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

void DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
    uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_uint8_t(x0, y0);
        _swap_uint8_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_uint8_t(x0, x1);
        _swap_uint8_t(y0, y1);
    }

    uint8_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    uint8_t err = dx / 2;
    uint8_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            DrawPixel(y0, x0, color);
        } else {
            DrawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

  void DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {
    uint8_t f = 1 - r;
    uint8_t ddF_x = 1;
    uint8_t ddF_y = -2 * r;
    uint8_t x = 0;
    uint8_t y = r;

    DrawPixel(x0  , y0+r, color);
    DrawPixel(x0  , y0-r, color);
    DrawPixel(x0+r, y0  , color);
    DrawPixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        DrawPixel(x0 + x, y0 + y, color);
        DrawPixel(x0 - x, y0 + y, color);
        DrawPixel(x0 + x, y0 - y, color);
        DrawPixel(x0 - x, y0 - y, color);
        DrawPixel(x0 + y, y0 + x, color);
        DrawPixel(x0 - y, y0 + x, color);
        DrawPixel(x0 + y, y0 - x, color);
        DrawPixel(x0 - y, y0 - x, color);
    }
}

void DrawFullRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    for (uint8_t i=x; i<x+w; i++) {
        DrawLine(i, y, i, h, color);
    }
}

   bool Parse(const char* cmd, const char* arg) override {
    if (!strcmp(cmd, "ssd")) {     
          SB_Message(arg);     
         return true;        
    }
    return false;
  }

  void Help() override {
    STDOUT.print("ssd \"text\"");
    STDOUT.println(" - Prints on LCD ");
 }

  void DrawBatteryBar(const Glyph& bar) {
    int start, end;
    if (bar.skip < bar.columns) {
      start = -bar.skip;
      end = WIDTH + bar.columns - 1;
    } else {
      start = 0;
      end = WIDTH;
    }
    int max_bars = (end - start) / bar.skip;
    int pos = start;
    int bars = floor(
        battery_monitor.battery_percent() * (0.5 + max_bars) / 100);
    for (int i = 0; i < bars; i++) {
      Draw(bar, pos, 0);
      pos += bar.skip;
    }
  }
  void DrawText(const char* str,
                int x, int y,
                const Glyph* font) {
    while (*str) {
      if (*str == '\n') {
        x = 0;
        y += 16;
      } else if (*str >= 0x20 && *str <= 0x7f) {
        Draw(font[*str - 0x20], x, y);
        x += font[*str - 0x20].skip;
      }
      str++;
    }
  }

void DrawScreenSaver( const char* text){
   static int yPosition = 0;
   static int xPosition = 0;
   static int xDirection = 1;
   static int yDirection = 1;
    if (xPosition == 0){
      xDirection = 1;
    }
    else 
    if (xPosition == WIDTH){
      xDirection = -1;
    }

    if (yPosition == 0){
      yDirection = 1;
    }
    else 
    if (yPosition == HEIGHT){
      yDirection = -1;
    }
 
    DrawText(text, xPosition, yPosition, FONT_NAME); 
    xPosition += xDirection;
    yPosition += yDirection;  
}

char* GfxBatteryPercentage(void){
  static char bat_info[6];
  static uint32_t ref_time=0;

  if (uint32_t(millis()- ref_time) > 2000){
    ref_time=millis();
    int perc = floor(battery_monitor.battery_percent() );
    sprintf(bat_info, "%d%%", perc);   
  }
  return bat_info;
}

  void FillFrameBuffer() {
    memset(frame_buffer_, 0, sizeof(frame_buffer_));

    if (millis() - displayed_when_ > 5000)
      screen_ = SCREEN_SAVER;

    switch (screen_) {
      case SCREEN_STARTUP:
        DrawText("==SabeR===", 0,15, FONT_NAME);
        DrawText("++Teensy++",-4,31, FONT_NAME);
        break;

      case SCREEN_SAVER:        
        DrawScreenSaver(GfxBatteryPercentage());       
        break;

      case SCREEN_PLI:
        DrawBatteryBar(BatteryBar16);
        break;

      case SCREEN_MESSAGE:
        if (strchr(message_, '\n')) {
          DrawText(message_, 0, 15, FONT_NAME);
        } else {
          DrawText(message_, 0, 23, FONT_NAME);
        }
    }
  }

  void SB_Message(const char* text) override {
    strncpy(message_, text, sizeof(message_));
    message_[sizeof(message_)-1] = 0;
    displayed_when_ = millis();
    screen_ = SCREEN_MESSAGE;
  }

  void SB_Top() override {
    STDOUT.print("display fps: ");
    loop_counter_.Print();
    STDOUT.println("");
  }

  void Loop() override {
    STATE_MACHINE_BEGIN();
    while (!i2cbus.inited()) YIELD();

    // Init sequence
    Send(DISPLAYOFF);                    // 0xAE
    Send(SETDISPLAYCLOCKDIV);            // 0xD5
    Send(0x80);                                  // the suggested ratio 0x80
    
    Send(SETMULTIPLEX);                  // 0xA8
    Send(HEIGHT - 1);
    
    Send(SETDISPLAYOFFSET);              // 0xD3
    Send(0x0);                                   // no offset
    Send(SETSTARTLINE | 0x0);            // line #0
    Send(CHARGEPUMP);                    // 0x8D
    Send(0x14);
    Send(MEMORYMODE);                    // 0x20
    Send(0x01);                          // vertical address mode
    Send(SEGREMAP | 0x1);
    Send(COMSCANDEC);

    Send(SETCOMPINS);                    // 0xDA
    Send(0x02);  // may need to be 0x12 for some displays
    Send(SETCONTRAST);                   // 0x81
    Send(0x8F);

    Send(SETPRECHARGE);                  // 0xd9
    Send(0xF1);
    Send(SETVCOMDETECT);                 // 0xDB
    Send(0x40);
    Send(DISPLAYALLON_RESUME);           // 0xA4
    Send(NORMALDISPLAY);                 // 0xA6
    
    Send(DEACTIVATE_SCROLL);

    Send(DISPLAYON);                     //--turn on oled panel

    STDOUT.println("Display initialized.");
    screen_ = SCREEN_STARTUP;
    displayed_when_ = millis();
    
    while (true) {
      FillFrameBuffer();
      Send(COLUMNADDR);
      Send(0);   // Column start address (0 = reset)
      Send(WIDTH-1); // Column end address (127 = reset)

      Send(PAGEADDR);
      Send(0); // Page start address (0 = reset)
      switch (HEIGHT) {
        case 64:
          Send(7); // Page end address
          break;
        case 32:
          Send(3); // Page end address
          break;
        case 16:
          Send(1); // Page end address
          break;
        default:
          STDOUT.println("Unknown display height");
      }

      //STDOUT.println(TWSR & 0x3, DEC);
        
      // I2C
      for (i=0; i < WIDTH * HEIGHT / 8; ) {
        // send a bunch of data in one xmission
        Wire.beginTransmission(address_);
        Wire.write(0x40);
        for (uint8_t x=0; x<16; x++) {
          Wire.write(((unsigned char*)frame_buffer_)[i]);
          i++;
        }
        Wire.endTransmission();
        YIELD();
      }
      loop_counter_.Update();
    }
    
    STATE_MACHINE_END();
  }

private:
  uint16_t i;
  uint32_t frame_buffer_[WIDTH];
  LoopCounter loop_counter_;
  char message_[32];
  uint32_t displayed_when_;
  Screen screen_;
};

#endif
