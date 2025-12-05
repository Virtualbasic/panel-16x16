#include <Adafruit_NeoPixel.h>

#define PIN 2
#define NUMPIXELS 256

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int V[4];


int mapIndex(int index) {
    int row = index / 16;
    int col = 15 -(index % 16) ;

    if (row % 2 == 0) {

        return row * 16 + col ;
    }


    return row * 16 + (15 - col);
}


void setup() {
    Serial.begin(9600);
    pixels.begin();
    setPanelColor(200, 55, 0); // startowy kolor
    pixels.setPixelColor(0, pixels.Color(255,25,25));
    pixels.setPixelColor(15, pixels.Color(255,25,25));
    pixels.setPixelColor(255, pixels.Color(255,25,25));
    pixels.setPixelColor(31, pixels.Color(255,25,25));
    pixels.show();
}

void loop() {
    if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        data.trim();

        if (data.length() == 0) return;

        parseData(data);

        if (V[0] == 999) {
            setPanelColor(V[1], V[2], V[3]);
        }
        else if (V[0] >= 0 && V[0] < NUMPIXELS) {
            int physIndex = mapIndex(V[0]);
            pixels.setPixelColor(physIndex, pixels.Color(V[1], V[2], V[3]));
            pixels.show();
        }
    }
}

void setPanelColor(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(r, g, b));
    }
    pixels.show();
}

void parseData(String input) {
    int index = 0;
    int lastIndex = 0;

    for (int i = 0; i < 4; i++)
        V[i] = 0;

    while (index < 4) {
        int commaIndex = input.indexOf(',', lastIndex);
        if (commaIndex == -1) commaIndex = input.length();

        String part = input.substring(lastIndex, commaIndex);
        V[index] = part.toInt();

        lastIndex = commaIndex + 1;
        index++;
    }
}
