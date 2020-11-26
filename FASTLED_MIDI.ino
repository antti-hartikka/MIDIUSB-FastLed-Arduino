#include <FastLED.h>
#include "MIDIUSB.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    0
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    446
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96

void setup() {
  // for MIDIUSB
  Serial.begin(115200);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


boolean started = false;
int hue = 0;
int sat = 255;
int count = 0;
int waveSpeed = 24;  // midi clock pulses at 24 times per quarter note. how long do you want your pulse to be?


// First parameter, rx.header, is the event type (0x0B = control change).
// Second parameter, rx.byte1, is the event type, combined with the channel.
// Third parameter, rx.byte2, is the control number number (0-119).
// Fourth parameter, rx.byte3, is the control value (0-127).

midiEventPacket_t rx;


void loop()
{

  do {

    //read midi data
    rx = MidiUSB.read();

    // check if note available
    if (rx.header != 0) {

      // if midi == clock, update led strip
      if (started && rx.byte1 == 0xF8) {
        // fills entire strip with HSV colours, cubicwave8() forms wave almost as sine
        fill_solid(&(leds[0]), NUM_LEDS, CHSV(hue, sat, cubicwave8(127 + (255 * count / waveSpeed))));
        FastLED.show();

        count++;
        if (count == 192) {
          count = 0;
        }

        continue;
      }

      // if midi == note on
      if (rx.header == 0x9) {

        // if midi-note is between C3 and B3, change pulse color hue
        if (rx.byte2 >= 0x3C && rx.byte2 <= 0x47) {
          int note = int(rx.byte2 - 0x3C);
          hue = note * 22;
          continue;
        }

        // if midi-note is between C4 and B4, change pulse color saturation
        if (rx.byte2 >= 0x48 && rx.byte2 <= 0x53) {
          int note = int(rx.byte2 - 0x48);
          sat = note * 23;
          continue;
        }

        // if midi-note is between C2 and B2, change pulse speed
        if (rx.byte2 >= 0x30 && rx.byte2 <= 0x3B) {
          int note = int(rx.byte2 - 0x30);
          switch (note) {
            case 0:         // C
              waveSpeed = 3;   // 32th
              break;
            case 1:         // C#
              waveSpeed = 4;   // 24th
              break;
            case 2:         // D
              waveSpeed = 6;   // 16th
              break;
            case 3:         // D#
              waveSpeed = 8;   // 12th
              break;
            case 4:         // E
            case 5:         // F
              waveSpeed = 12;  // 8th
              break;
            case 6:         // F#
              waveSpeed = 16;  // 6th
              break;
            case 7:         // G
              waveSpeed = 24;  // 4th
              break;
            case 8:         // G#
              waveSpeed = 36;  // 3th
              break;
            case 9:         // A
              waveSpeed = 48;  // half note
              break;
            case 10:        // A#
              waveSpeed = 96;  // whole note
              break;
            case 11:        // B
              waveSpeed = 192; // two wholes
              break;
          }
        }
      }

      // if midi == start, fill strip with colour, set counter to zero
      if (rx.byte1 == 0xFA) {
        started = true;

        fill_solid(&(leds[0]), NUM_LEDS, CHSV(hue, sat, 255));
        FastLED.show();

        count = 0;
        continue;
      }

      // if midi == stop, fill strip with zero brightness
      if (rx.byte1 == 0xFC) {
        started = false;

        fill_solid(&(leds[0]), NUM_LEDS, CHSV(hue, sat, 0));
        FastLED.show();

        continue;
      }

    }
  } while (rx.header != 0);
}
