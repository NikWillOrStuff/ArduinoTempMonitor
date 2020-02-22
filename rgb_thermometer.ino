#include <Arduino_HTS221.h>

int redPin = 22;
int greenPin = 23;
int bluePin = 24;

bool verbose_debugging = false;

float neutral_temperature = 21; //in celcius. when at neutral, will appear green
float temperature_scale = 5; //in celcius. degrees between neutral and hot, or neutral and cold

float brightness = 0.25; //LED brightness, 0 is off and 1 is max

void setup() {
  Serial.begin(9600);
  //while (!Serial);
  delay(2000);
  Serial.print("Hello, world!");
  
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1){
      analogWrite(redPin, round((1.0 - brightness)*256)); //on
      delay(1000);
      analogWrite(redPin, 256); //off
      delay(1000);
    }
  }

}

void loop() {
  float temperature = HTS.readTemperature(); // measured in degrees celcius

  temperature = temperature - neutral_temperature; //making the temperature zero-indexed around our target neutral temperature

  temperature = temperature / temperature_scale; // now "hot" is +1 and "cold" is -1

  Serial.print("Temperature:");
  Serial.print(temperature);

  //hue-related calculations
  float hue = 0 - temperature; //inverting because of the direction of the hue color scale
  hue = (hue + 1) / 3; //adjusting the range of -1 to 0, into 0 to 0.66

  float rgb[3];
  hsv2rgb(hue, 1.0, brightness, rgb);

  Serial.print("\tHue:");
  Serial.print(hue);

  setColor(rgb);

  Serial.println();
  delay(500);
}

void setColor(float *colors) { //expects float values between 0 and 1

  //inverting inputs because the lights are controlled with inverted numbers
  float r = 1.0 - colors[0];
  float g = 1.0 - colors[1];
  float b = 1.0 - colors[2];

  //256 is off, 255 is slightly glowing, 0 is full bright (why there are 257 possible states, I have no idea)
  //the values are different for each color to correct for the different brightness of each LED
  int r2 = round(r * 192.0) + 64;
  int g2 = round(g * 128.0) + 128;
  int b2 = round(b * 256.0) + 0;

  if (verbose_debugging){
    Serial.print("after inverting: ");
    Serial.print(r);
    Serial.print(" ");
    Serial.print(g);
    Serial.print(" ");
    Serial.print(b);
    Serial.println();

    Serial.print("after multiplying: ");
    Serial.print(r2);
    Serial.print(" ");
    Serial.print(g2);
    Serial.print(" ");
    Serial.print(b2);
    Serial.println();
  }
  
  analogWrite(redPin, r2);
  analogWrite(greenPin, g2);
  analogWrite(bluePin, b2);
}

// HSV->RGB conversion based on GLSL version
// expects hsv channels defined in 0.0 .. 1.0 interval
float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}

float* rgb2hsv(float r, float g, float b, float* hsv) {
  float s = step(b, g);
  float px = mix(b, g, s);
  float py = mix(g, b, s);
  float pz = mix(-1.0, 0.0, s);
  float pw = mix(0.6666666, -0.3333333, s);
  s = step(px, r);
  float qx = mix(px, r, s);
  float qz = mix(pw, pz, s);
  float qw = mix(r, px, s);
  float d = qx - min(qw, py);
  hsv[0] = abs(qz + (qw - py) / (6.0 * d + 1e-10));
  hsv[1] = d / (qx + 1e-10);
  hsv[2] = qx;
  return hsv;
}
