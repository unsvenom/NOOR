#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WIFI :
#define WIFI_SSID "Memoire hors theme"
#define WIFI_PASSWORD "chaennie"

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

// braille mapping :

uint8_t arabic_to_braille(uint16_t letter) {
    switch(letter) {
        case 0x0621: return 0b000100; // ء
        case 0x0622: return 0b011100; // آ
        case 0x0623: return 0b001100; // أ
        case 0x0624: return 0b110011; // ؤ
        case 0x0625: return 0b101000; // إ
        case 0x0626: return 0b111101; // ئ
        case 0x0627: return 0b000001; // ا
        case 0x0628: return 0b000011; // ب
        case 0x0629: return 0b100001; // ة
        case 0x062A: return 0b011110; // ت
        case 0x062B: return 0b111001; // ث
        case 0x062C: return 0b011010; // ج
        case 0x062D: return 0b110001; // ح
        case 0x062E: return 0b101101; // خ
        case 0x062F: return 0b011001; // د
        case 0x0630: return 0b101110; // ذ
        case 0x0631: return 0b010111; // ر
        case 0x0632: return 0b110101; // ز
        case 0x0633: return 0b001110; // س
        case 0x0634: return 0b101001; // ش
        case 0x0635: return 0b101111; // ص
        case 0x0636: return 0b101011; // ض
        case 0x0637: return 0b111110; // ط
        case 0x0638: return 0b111111; // ظ
        case 0x0639: return 0b110111; // ع
        case 0x063A: return 0b100011; // غ
        case 0x0641: return 0b001011; // ف
        case 0x0642: return 0b011111; // ق
        case 0x0643: return 0b000101; // ك
        case 0x0644: return 0b000111; // ل
        case 0x0645: return 0b001101; // م
        case 0x0646: return 0b011101; // ن
        case 0x0647: return 0b010011; // ه
        case 0x0648: return 0b111010; // و
        case 0x064A: return 0b001010; // ي
        case 0x0649: return 0b010101; // ى
        default: return 0b000000;  // bash nlekidi any other letter
    }
}

void testServos() {
    for (int i = 0; i < 6; i++) {
        servos[i].write(90); 
    }
    delay(500);
    for (int i = 0; i < 6; i++) {
        servos[i].write(0); 
    }
    delay(500);
}

void translateAndMoveServos(String text) {
    for (int i = 0; i < text.length(); i++) {
        uint16_t letter = text[i];
        byte braille = arabic_to_braille(letter);

        // hna la base ta3 khedma 
        for (int j = 0; j < 6; j++) {
            if ((braille >> j) & 1) {
                servos[j].write(90); 
            } else {
                servos[j].write(0);
            }
        }
        delay(500); // bin letter w letter
    }

    // outro ya l7bibiba 
    for (int i = 0; i < 6; i++) {
        servos[i].write(0);
    }
}

void setup() {
    Serial.begin(115200);
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

    // hna sign up ll firebase : 
    if (Firebase.signUp(&config, &auth, "", "")){
        Serial.println("ok");
        signupOK = true;
    }
    else{
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    // Assign the callback function for the long running token generation task
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    //declaration ta3 les servos :

    for (int i = 0; i < 6; i++) {
        servos[i].attach(servoPins[i]);
    }

    // testi les servos mn lwl bash ybda lkhdma :
    testServos();
}

void loop() {
    if (Firebase.ready() && signupOK) {

        // Read lstring mn Firebase 

        if (Firebase.RTDB.getString(&fbdo, "/text")) {
            if (fbdo.dataType() == "string") {
                String payload = fbdo.stringData();
                Serial.println(payload);
                translateAndMoveServos(payload);
            } else {
                Serial.println("Failed to get string data from Firebase");
            }
        } else {
            Serial.println("Firebase getString failed");
            Serial.println(fbdo.errorReason());
        }
    }
    delay(10000); // Wait before checking again
}
