#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WIFI :
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// FIREBASE :
#define API_KEY "AIzaSyAAYqQGyH4EmPTP1LHxen3j_CA1kl_b0po"
#define DATABASE_URL "https://noorupthedoor-default-rtdb.europe-west1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
// les servos :
const int servoPins[6] = {2, 4, 5, 18, 19, 21};
Servo servos[6];

struct BraillePattern {
    int pattern[6];
};

// braille mapping : letter[i] >> switch(letter[i])
BraillePattern arabic_to_braille(int letter) {
    switch (letter) {
        case 0x0621: return {{0,0,0,1,0,0}}; // ء
        case 0x0622: return {{0,1,1,1,0,0}}; // آ
        case 0x0623: return {{0,0,1,1,0,0}}; // أ
        case 0x0624: return {{1,1,0,0,1,1}}; // ؤ
        case 0x0625: return {{1,0,1,0,0,0}}; // إ
        case 0x0626: return {{1,1,1,1,0,1}}; // ئ
        case 0x0627: return {{0,0,0,0,0,1}}; // ا
        case 0x0628: return {{0,0,0,0,1,1}}; // ب
        case 0x0629: return {{1,0,0,0,0,1}}; // ة
        case 0x062A: return {{0,1,1,1,1,0}}; // ت
        case 0x062B: return {{1,1,1,0,0,1}}; // ث
        case 0x062C: return {{0,1,1,0,1,0}}; // ج
        case 0x062D: return {{1,1,0,0,0,1}}; // ح
        case 0x062E: return {{1,0,1,1,0,1}}; // خ
        case 0x062F: return {{0,1,1,0,0,1}}; // د
        case 0x0630: return {{1,0,1,1,1,0}}; // ذ
        case 0x0631: return {{0,1,0,1,1,1}}; // ر
        case 0x0632: return {{1,1,0,1,0,1}}; // ز
        case 0x0633: return {{0,0,1,1,1,0}}; // س
        case 0x0634: return {{1,0,1,0,0,1}}; // ش
        case 0x0635: return {{1,0,1,1,1,1}}; // ص
        case 0x0636: return {{1,0,1,0,1,1}}; // ض
        case 0x0637: return {{1,1,1,1,1,0}}; // ط
        case 0x0638: return {{1,1,1,1,1,1}}; // ظ
        case 0x0639: return {{1,1,0,1,1,1}}; // ع
        case 0x063A: return {{1,0,0,0,1,1}}; // غ
        case 0x0641: return {{0,0,1,0,1,1}}; // ف
        case 0x0642: return {{0,1,1,1,1,1}}; // ق
        case 0x0643: return {{0,0,0,1,0,1}}; // ك
        case 0x0644: return {{0,0,0,1,1,1}}; // ل
        case 0x0645: return {{0,0,1,1,0,1}}; // م
        case 0x0646: return {{0,1,1,1,0,1}}; // ن
        case 0x0647: return {{0,1,0,0,1,1}}; // ه
        case 0x0648: return {{1,1,1,0,1,0}}; // و
        case 0x064A: return {{0,0,1,0,1,0}}; // ي
        case 0x0649: return {{0,1,0,1,0,1}}; // ى
        default: return {{0,0,0,0,0,0}};  // default pattern
    }
}

void translateAndMoveServos(String text) {
    for (int i = 0; i < text.length();) {
        uint16_t letter;
        // Decode UTF-8 to get the Unicode code point
        if ((text[i] & 0xF0) == 0xE0) {
            // 3-byte character (shouldn't be necessary for Arabic, but just in case)
            letter = ((text[i] & 0x0F) << 12) | ((text[i + 1] & 0x3F) << 6) | (text[i + 2] & 0x3F);
            i += 3;
        } else if ((text[i] & 0xE0) == 0xC0) {
            // 2-byte character (most likely case for Arabic)
            letter = ((text[i] & 0x1F) << 6) | (text[i + 1] & 0x3F);
            i += 2;
        } else {
            // 1-byte character (unlikely for Arabic, but handle it)
            letter = text[i];
            i++;
        }

        Serial.print("Translating letter: ");
        Serial.println(letter, HEX);

        BraillePattern braille = arabic_to_braille(letter);

        Serial.print("Braille pattern: ");
        for (int j = 0; j < 6; j++) {
            Serial.print(braille.pattern[j]);
            Serial.print(" ");
        }
        Serial.println();

        // Move servos based on Braille pattern
        for (int j = 0; j < 6; j++) {
            if (braille.pattern[j]) {
                servos[j].write(90); 
            } else {
                servos[j].write(0);
            }
        }
        delay(1000); // Wait between letters
    }

    // Reset servos
    for (int i = 0; i < 6; i++) {
        servos[i].write(0);
    }
}


void setup() {
    Serial.begin(115200);
    Serial.println("Initializing...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    if (Firebase.signUp(&config, &auth, "", "")){
        Serial.println("Firebase sign-up successful");
        signupOK = true;
    } else {
        Serial.printf("Firebase sign-up failed: %s\n", config.signer.signupError.message.c_str());
    }

    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    for (int i = 0; i < 6; i++) {
        servos[i].attach(servoPins[i]);
    }

    Serial.println("Setup completed.");
}

void loop() {
    if (Firebase.RTDB.getString(&fbdo, "/text")) {
        if (fbdo.dataType() == "string") {
            String payload = fbdo.stringData();
            Serial.println("Received text: ");
            Serial.println(payload);
            translateAndMoveServos(payload);
        } else {
            Serial.println("Failed to get string data from Firebase");
        }
    } else {
        Serial.println("Firebase getString failed");
        Serial.println(fbdo.errorReason());
    }
    delay(10000); // Wait before checking again
}
