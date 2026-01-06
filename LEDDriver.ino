#include <FastLED.h>

#define LED_PIN     15
#define NUM_LEDS    12
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB

#define BUTTON_PIN  13
#define DEBOUNCE_MS 50

CRGB leds[NUM_LEDS];

// Button state tracking
bool lastReading = HIGH;
bool buttonPressed = false;

unsigned long lastDebounceTime = 0;
unsigned long pressStartTime = 0;
unsigned long buttonCount = 0;

// Animation state
uint8_t hue = 0;
uint8_t wipeIndex = 0;
uint8_t breathePhase = 0;

void setup() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
           .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.begin(9600);
}

void loop() {
    checkButtonState();

    uint8_t mode = (buttonCount % 20) + 1;

    runLedMode(mode);

    FastLED.show();
    delay(20);
}

void checkButtonState() {
    bool reading = digitalRead(BUTTON_PIN);

    if (reading != lastReading) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {

        // Button pressed
        if (!buttonPressed && reading == LOW) {
            buttonPressed = true;
            pressStartTime = millis();
        }

        // Button released
        if (buttonPressed && reading == HIGH) {
            unsigned long pressDuration = millis() - pressStartTime;

            if (pressDuration >= DEBOUNCE_MS) {
                buttonCount++;
                Serial.print("Mode: ");
                Serial.println((buttonCount % 20) + 1);
            }

            buttonPressed = false;
        }
    }

    lastReading = reading;
}

void runLedMode(uint8_t mode) {

    // Modes 1–16: solid HSV colors
    if (mode >= 1 && mode <= 16) {
        uint8_t colorHue = map(mode - 1, 0, 15, 0, 255);
        fill_solid(leds, NUM_LEDS, CHSV(colorHue, 255, 255));
        return;
    }

    switch (mode) {

        // 17: Rainbow
        case 17:
            fill_rainbow(leds, NUM_LEDS, hue++, 5);
            break;

        // 18: Color wipe
        case 18:
            fadeToBlackBy(leds, NUM_LEDS, 40);
            leds[wipeIndex] = CHSV(hue, 255, 255);
            wipeIndex = (wipeIndex + 1) % NUM_LEDS;
            hue += 2;
            break;

        // 19: Breathing white
        case 19: {
            uint8_t brightness = sin8(breathePhase);
            fill_solid(leds, NUM_LEDS, CRGB(brightness, brightness, brightness));
            breathePhase += 2;
            break;
        }

        // 20: Confetti
        case 20:
            fadeToBlackBy(leds, NUM_LEDS, 20);
            leds[random8(NUM_LEDS)] += CHSV(random8(), 200, 255);
            break;
    }
}