#include <TFT_eSPI.h>
#include <SPI.h>
#include "Preferences.h"

// Pin Definitions
#define PH_PIN           34   // pH sensor pin
#define TURBIDITY_PIN    35   // Turbidity sensor pin
#define FLOW_PIN         21   // Flow sensor pin
#define RESET_BUTTON_PIN 12   // Push button pin


#define BUTTON_PIN 0     // Define your button pin

// Timing variables
unsigned long buttonPressStartTime = 0; // To track when the button was pressed
bool buttonPressed = false;               // Current button state
bool longPressDetected = false;           // Long press state
const unsigned long LONG_PRESS_DURATION = 5000; // 8 seconds for long press


const int numSamples = 50;  // Number of samples to average
float phBuffer[numSamples];  // Array to hold the latest readings
float sum = 0;               // Current sum of the samples
int bufferIndex = 0;        // Current index for the buffer
bool isBufferFilled = false; // Flag to indicate if the buffer is filled

// Initialize TFT and Sprite
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite for off-screen rendering

// Flow Sensor Variables
volatile int flowPulseCount = 0;
float totalLiters = 0;

// Flash storage for flow value
Preferences preferences; // To handle flash storage

// Debounce Variables for the Reset Button
unsigned long lastButtonPress = 0;
bool buttonPressedRst = false;

// Save Counter
int saveCounter = 0; // Counter to track number of saves
const int maxSaves = 15; // Maximum saves allowed

// Array of random malfunction messages
const char* malfunctionMessages[] = {
    "@$∙╠L3ù1¡┌987ù╒",
    "()_98&()*5☻♥☺♦",
    "@$∙╠L3ù1¡┌987ù╒",
    "12`1",
    "~!@#$%^&*()_98&()*5☻♥☺♦",
    "a;sd",
    "a789"
};
// pH Scale Dimensions
const int scaleX = 12;
const int scaleY = 5;
const int scaleWidth = 130;
const int scaleHeight = 10;
// Function to map voltage to pH value based on calibration
float mapVoltageToPH(float voltage) {
    float pH_7_voltage = 2.59; // Voltage for pH 7
    float pH_4_voltage = 3.12; // Voltage for pH 4 (acidic)
    float pH_10_voltage = 2.14; // Voltage for pH 10 (alkaline)

    if (voltage >= pH_7_voltage) {
        return 7.0 + (voltage - pH_7_voltage) * ((4.0 - 7.0) / (pH_4_voltage - pH_7_voltage));
    } else {
        return 7.0 - (pH_7_voltage - voltage) * ((7.0 - 10.0) / (pH_7_voltage - pH_10_voltage));
    }
}

// Flow pulse interrupt function
void IRAM_ATTR flowPulse() {
    flowPulseCount++; // Increment pulse count on interrupt
}

// Function to save data
void saveData() {
    // Retrieve save counter or default to 0
    saveCounter = preferences.getInt("saveCounter", 0); 
    if (saveCounter < maxSaves) { // Check if we haven't reached the maximum saves
        preferences.putFloat("totalLiters", totalLiters); // Save total liters to flash
        saveCounter++; // Increment saveCounter
        preferences.putInt("saveCounter", saveCounter); // Save the incremented counter
        //Serial.println(saveCounter); // Debugging output
    } else {
        // Introduce malfunction after 15 saves
        Serial.println("[ERROR]-chip-monk-agsser");
        
        // Select a random malfunction message to display
        int randomIndex = random(0, sizeof(malfunctionMessages) / sizeof(malfunctionMessages[0]));
        spr.fillSprite(TFT_BLACK); // Clear the sprite
        spr.setTextColor(TFT_RED, TFT_BLACK); // Set text color to red
        spr.setTextSize(2); // Larger font size for message
        spr.setCursor(0, 40); // Adjusted Y-position for message
        spr.printf("%s", malfunctionMessages[randomIndex]); // Display random message
        spr.pushSprite(0, 0); // Push the sprite to the screen at position (0, 0)
        tft.setRotation(randomIndex); // Rotate screen 90 degrees
        spr.createSprite(0, 0); // Same size as the display
        while(true);
        delay(2000); // Show the message for 2 seconds
    }
}

void setup() {
    // Initialize Serial Monitor for Debugging (optional)
    Serial.begin(115200);

    // Sensor Pin Modes
    pinMode(BUTTON_PIN,INPUT_PULLUP);
    pinMode(PH_PIN, INPUT);
    pinMode(TURBIDITY_PIN, INPUT);
    pinMode(FLOW_PIN, INPUT_PULLUP);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP); // Button for resetting

    // Attach Interrupt for Flow Sensor
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowPulse, FALLING); // Flow sensor interrupt

    // Initialize TFT Display
    tft.init();
    tft.setRotation(3);  // Set initial rotation to 3 (normal position)
    tft.fillScreen(TFT_BLACK);

    // Initialize Sprite
    spr.createSprite(160, 128); // Same size as the display

    // Initialize Flash Storage
    preferences.begin("flow_data", false);  // "flow_data" is the namespace
    totalLiters = preferences.getFloat("totalLiters", 0.0);  // Retrieve saved value or default to 0.0
    saveCounter = preferences.getInt("saveCounter", 0); // Retrieve save counter or default to 0
  // Initialize the buffer
    for (int i = 0; i < numSamples; i++) {
        phBuffer[i] = 0;  // Initialize buffer values
    }
}

void loop() {

     // Read the button state
    int buttonState = digitalRead(BUTTON_PIN);

    // Button is pressed (active low)
    if (buttonState == LOW) {
        if (!buttonPressed) { // Button was not previously pressed
            buttonPressed = true; // Update the button state
            buttonPressStartTime = millis(); // Record the time when button is pressed
            longPressDetected = false; // Reset long press flag
        }

        // Check if the button is held down long enough
        if (!longPressDetected && (millis() - buttonPressStartTime >= LONG_PRESS_DURATION)) {
            longPressDetected = true; // Long press detected
            //Serial.println("Long press detected!");
            saveCounter=0;
            preferences.putInt("saveCounter",saveCounter);
            // Perform actions for long press here
        }
    } else { // Button is released
        if (buttonPressed) { // Button was previously pressed
            buttonPressed = false; // Update the button state
            if (!longPressDetected) {
                //Serial.println("Short press detected!");
                // Perform actions for short press here
            }
        }
    }
    // Read the new analog value
    float newReading = analogRead(PH_PIN);
    
    // Subtract the oldest sample from the sum
    sum -= phBuffer[bufferIndex];
    
    // Update the buffer with the new reading
    phBuffer[bufferIndex] = newReading;
    
    // Add the new reading to the sum
    sum += newReading;
    
    // Increment the index and wrap around if necessary
    bufferIndex = (bufferIndex + 1) % numSamples;
    
    // Check if the buffer is filled for the first time
    if (!isBufferFilled && bufferIndex == 0) {
        isBufferFilled = true; // Set the flag when the buffer is filled
    }
    
    // Calculate the average only when the buffer is filled
    float averagePH = isBufferFilled ? (sum / numSamples) : (sum / bufferIndex);

    // Optionally convert the average voltage to pH if needed
    float voltage = averagePH * (3.3 / 4095.0); // Assuming a 5V reference
    float phValue = mapVoltageToPH(voltage);     // Function to map voltage to pH (define separately)


    // Read the turbidity sensor and convert it to voltage
    float voltageTurbidity = analogRead(TURBIDITY_PIN) * (3.3 / 4095.0);
    // Assuming voltage range maps to turbidity range (0 - 1.47)
    float turbidityValue = voltageTurbidity; // Directly using voltage as turbidity

    // Reverse the percentage calculation for turbidity
    float turbidityPercentage = (1 - (turbidityValue / 1.47)) * 100; 
    turbidityPercentage = constrain(turbidityPercentage, 0, 100); // Constrain percentage between 0 and 100

    // Calculate Flow Rate and Total Liters
    float flowRate = (flowPulseCount / 7.5); // Example conversion to L/min (adjust based on flow sensor)
    totalLiters += flowRate / 60.0; // Add to total in liters
    flowPulseCount = 0;

    // Check Reset Button (with simple debounce)
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        if (!buttonPressedRst) { // Button just pressed
            buttonPressedRst = true;
            lastButtonPress = millis();
            // Save total liters to flash when button is pressed
            saveData();
        } else { // Button is already pressed
            // Check for long press
            if (millis() - lastButtonPress > 3000) { // Long press duration (4 seconds)
                totalLiters = 0; // Reset flow volume
                saveData();  // Reset the stored value
                buttonPressedRst = false; // Reset the button pressed state
            }
        }
    } else {
        buttonPressedRst = false; // Reset button pressed state when released
    }

    // Clear the Sprite
    spr.fillSprite(TFT_BLACK); // Clear the sprite to black
    // Draw pH Color Scale in sprite
    drawPHScale(scaleX, scaleY, scaleWidth, scaleHeight);
    // Draw pH Cursor in sprite
    drawPHCursor(phValue, scaleX, scaleY, scaleWidth, scaleHeight);
    // Draw the cursor on the pH scale based on the current pH value
    int cursorX = 15 + (int)((phValue / 14.0) * 130); // Map pH value to cursor position
    spr.fillRect(cursorX, 3 + 20, 2, 8, TFT_RED); // Draw a red cursor
    // Display pH, Turbidity, and Flow values in a larger font size
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextSize(2); // Larger font size for values
    spr.setCursor(3, 40); // Adjusted Y-position for pH
    spr.printf("pH: %.2f", phValue);    // Display pH value with 2 decimal places
    spr.setCursor(3, 75); // Adjusted Y-position for Turbidity
    spr.printf("Turb: %.2f%%", turbidityPercentage); // Display turbidity percentage
    spr.setCursor(3, 100); // Adjusted Y-position for Flow

    // Determine color for flow value based on comparison
    uint16_t flowColor = (totalLiters != preferences.getFloat("totalLiters", 0.0)) ? TFT_RED : TFT_WHITE; // Red if different, white if the same
    spr.setTextColor(flowColor, TFT_BLACK); // Set text color based on flow value
    spr.printf("Flow: %.1f L", totalLiters);  // Display total liters flowed

    

    // Push the Sprite to the Display
    spr.pushSprite(0, 0); // Push the entire sprite to the screen at position (0, 0)

    // Shorter delay for increased responsiveness
    delay(100); // Update every 0.1 seconds

}
// Function to Draw pH Cursor (in sprite)
void drawPHCursor(float ph, int x, int y, int width, int height) {
  // Clamp pH value between 0 and 14
  ph = constrain(ph, 0.0, 14.0);
  // Calculate cursor position
  int cursorX = x + (ph / 14.0) * width;
  // Draw cursor line
  spr.drawLine(cursorX, y - 5, cursorX, y + height + 5, TFT_WHITE);
}
// Function to Draw pH Color Scale (in sprite)
void drawPHScale(int x, int y, int width, int height) {
    // pH Range from 0 to 14
    for (int i = 0; i <= width; i++) {
        float ph = (i / (float)width) * 14.0; // Map pixel to pH value
        uint16_t color = getColorForPH(ph); // Get color for pH value
        spr.drawFastHLine(x + i, y, 1, color); // Draw a line with the corresponding color
    }
}

// Function to Get Color Based on pH
uint16_t getColorForPH(float ph) {
    if (ph < 7.0) {
        // Acidic: Red to Green
        float ratio = (ph + 14.0) / 7.0;
        uint8_t r = 255 * ratio;
        uint8_t g = 255 * (1 - ratio);
        uint8_t b = 0;
        return spr.color565(r, g, b);
    } else {
        // Alkaline: Green to Blue
        float ratio = (ph - 7.0) / 7.0;
        uint8_t r = 255 * (1 - ratio);
        uint8_t g = 255;
        uint8_t b = 255 * ratio;
        return spr.color565(r, g, b);
    }
}


