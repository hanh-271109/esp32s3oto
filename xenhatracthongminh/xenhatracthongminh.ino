#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ==== L298N Motor Driver Pins (ESP32-S3) - SỬA CHÂN GPIO ====
// Motor TRÁI (2 motor song song)
#define LEFT_MOTOR_PIN1   9   // ✅ IN1 trên L298N - GPIO9 có trên ESP32-S3
#define LEFT_MOTOR_PIN2   10  // ✅ IN2 trên L298N - GPIO10 có trên ESP32-S3
#define LEFT_MOTOR_PWM    11  // ✅ ENA trên L298N (PWM) - GPIO11 có trên ESP32-S3

// Motor PHẢI (2 motor song song)  
#define RIGHT_MOTOR_PIN1  35  // ✅ IN3 trên L298N - GPIO35 có trên ESP32-S3
#define RIGHT_MOTOR_PIN2  36  // ✅ IN4 trên L298N - GPIO36 có trên ESP32-S3
#define RIGHT_MOTOR_PWM   37  // ✅ ENB trên L298N (PWM) - GPIO37 có trên ESP32-S3

// ==== Extra pins ====
#define LIGHT_PIN  4    // Đèn thật - GPIO4 có trên ESP32-S3
#define HORN_PIN   5    // Còi thật - GPIO5 có trên ESP32-S3
#define HAZARD_PIN 8    // Đèn hazard thật - GPIO8 có trên ESP32-S3
#define STATUS_LED LED_BUILTIN

// ==== PWM parameters cho L298N ====
#define PWM_FREQ 5000
#define PWM_RES  8      // 8-bit (0-255)

// ==== Cấu hình hệ thống ====
const char* ssid = "ESP32-4Motor";
const char* password = "12345678";

// ==== Hệ thống quản lý lỗi ====
enum SystemStatus {
    SYS_OK = 0,
    SYS_ERROR_WIFI,
    SYS_ERROR_PWM,
    SYS_ERROR_MOTOR,
    SYS_WARNING_LOW_MEMORY
};

class ErrorManager {
private:
    bool systemHealthy = true;
    int errorCount = 0;
    
public:
    void logError(SystemStatus code, const char* message) {
        errorCount++;
        systemHealthy = false;
        Serial.printf("❌ ERROR [%d] %s\n", code, message);
        indicateError(code);
    }

    void logInfo(const char* message) {
        Serial.printf("ℹ️ INFO: %s\n", message);
    }

    void indicateError(SystemStatus code) {
        for(int i = 0; i < (code % 3) + 1; i++) {
            digitalWrite(STATUS_LED, HIGH);
            delay(300);
            digitalWrite(STATUS_LED, LOW);
            delay(300);
        }
    }
    
    bool isSystemHealthy() { return systemHealthy; }
    int getErrorCount() { return errorCount; }
    
    void systemCheck() {
        static unsigned long lastCheck = 0;
        if (millis() - lastCheck > 30000) {
            lastCheck = millis();
            int freeHeap = ESP.getFreeHeap();
            if (freeHeap < 15000) {
                logError(SYS_WARNING_LOW_MEMORY, "Bộ nhớ thấp");
            }
            Serial.printf("🔍 System Check: Heap=%dB, Errors=%d\n", freeHeap, errorCount);
        }
    }
};

ErrorManager errorManager;

// ==== Motor Controller cho L298N với 4 motor song song ====
class MotorController {
private:
    struct Motor {
        int pin1, pin2, pwmPin;
        int pwmChannel;
        const char* name;
        bool enabled;
    };
    
    Motor leftMotor = {LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, LEFT_MOTOR_PWM, 0, "Left Motors (2x)", false};
    Motor rightMotor = {RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2, RIGHT_MOTOR_PWM, 1, "Right Motors (2x)", false};

    bool setupMotorPWM(Motor& motor) {
        Serial.printf("Setting up %s: IN1=%d, IN2=%d, PWM=%d\n", 
                     motor.name, motor.pin1, motor.pin2, motor.pwmPin);
        
        // Cấu hình digital pins trước
        pinMode(motor.pin1, OUTPUT);
        pinMode(motor.pin2, OUTPUT);
        digitalWrite(motor.pin1, LOW);
        digitalWrite(motor.pin2, LOW);
        
        // Sử dụng ledcAttach với API mới - FIXED
        int result = ledcAttach(motor.pwmPin, PWM_FREQ, PWM_RES);
        if (result < 0) {
            char errorMsg[80];
            snprintf(errorMsg, sizeof(errorMsg), "Lỗi PWM %s (pin %d) - code %d", 
                     motor.name, motor.pwmPin, result);
            errorManager.logError(SYS_ERROR_PWM, errorMsg);
            return false;
        }
        
        motor.pwmChannel = result;
        ledcWrite(motor.pwmChannel, 0); // Dừng motor
        
        Serial.printf("%s setup complete - Channel: %d\n", motor.name, motor.pwmChannel);
        return true;
    }

public:
    bool initialize() {
        bool success = true;
        
        Serial.println("🔄 Initializing 4 motors (2x parallel each side)...");
        
        // Khởi tạo motor trái (2 motor song song)
        if (!setupMotorPWM(leftMotor)) {
            errorManager.logError(SYS_ERROR_MOTOR, "Lỗi khởi tạo motor trái");
            success = false;
        } else {
            leftMotor.enabled = true;
            errorManager.logInfo("2 Motor trái song song khởi tạo thành công");
        }

        // Khởi tạo motor phải (2 motor song song)
        if (!setupMotorPWM(rightMotor)) {
            errorManager.logError(SYS_ERROR_MOTOR, "Lỗi khởi tạo motor phải");
            success = false;
        } else {
            rightMotor.enabled = true;
            errorManager.logInfo("2 Motor phải song song khởi tạo thành công");
        }

        stopAll();
        return success;
    }

    void setMotorSpeed(Motor& motor, int speed) {
        if (!motor.enabled) {
            Serial.printf("Motor %s disabled!\n", motor.name);
            return;
        }
        
        speed = constrain(speed, -255, 255);
        
        Serial.printf("Setting %s speed: %d\n", motor.name, speed);
        
        if (speed > 0) {
            // Tiến
            digitalWrite(motor.pin1, HIGH);
            digitalWrite(motor.pin2, LOW);
            ledcWrite(motor.pwmChannel, speed);
        } else if (speed < 0) {
            // Lùi
            digitalWrite(motor.pin1, LOW);
            digitalWrite(motor.pin2, HIGH);
            ledcWrite(motor.pwmChannel, -speed);
        } else {
            // Dừng
            digitalWrite(motor.pin1, LOW);
            digitalWrite(motor.pin2, LOW);
            ledcWrite(motor.pwmChannel, 0);
        }
    }

    void setSpeeds(int leftSpeed, int rightSpeed) {
        Serial.printf("Setting speeds - Left: %d, Right: %d\n", leftSpeed, rightSpeed);
        setMotorSpeed(leftMotor, leftSpeed);
        setMotorSpeed(rightMotor, rightSpeed);
    }

    void stopAll() {
        Serial.println("Stopping all motors");
        setSpeeds(0, 0);
    }

    bool areMotorsEnabled() {
        return leftMotor.enabled && rightMotor.enabled;
    }
    
    // Test riêng cho 4 motor song song
    void test4Motors() {
        if (!areMotorsEnabled()) return;
        
        Serial.println("Testing 4 motors (parallel configuration)...");
        
        // Test tiến
        setSpeeds(150, 150);
        delay(800);
        
        // Test lùi
        setSpeeds(-150, -150);
        delay(800);
        
        // Test trái
        setSpeeds(-150, 150);
        delay(800);
        
        // Test phải
        setSpeeds(150, -150);
        delay(800);
        
        // Dừng
        stopAll();
        Serial.println("4 Motor test completed");
    }
};

MotorController motorController;

// ==== Biến toàn cục ====
int currentSlider = 5;
int basePWM = 128;

// ===== Button debouncing =====
unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 50;

bool debounce() {
    unsigned long now = millis();
    if (now - lastButtonTime < debounceDelay) return false;
    lastButtonTime = now;
    return true;
}

// ===== Horn (non-blocking) =====
bool hornActive = false;
unsigned long hornStart = 0;
const unsigned long hornDuration = 200;

void horn() {
    if (hornActive) return;
    hornActive = true;
    hornStart = millis();
    digitalWrite(HORN_PIN, HIGH);
    errorManager.logInfo("Còi bật");
}

void handleHorn() {
    if (hornActive && (millis() - hornStart >= hornDuration)) {
        digitalWrite(HORN_PIN, LOW);
        hornActive = false;
        errorManager.logInfo("Còi tắt");
    }
}

// ===== Hazard =====
bool hazardOn = false;
unsigned long lastHazardBlink = 0;
const unsigned long hazardInterval = 500;

void toggleHazard() {
    hazardOn = !hazardOn;
    String message = hazardOn ? "Hazard bật" : "Hazard tắt";
    errorManager.logInfo(message.c_str());
    if (!hazardOn) digitalWrite(HAZARD_PIN, LOW);
}

// ===== Light toggle =====
void toggleLight() {
    static bool state = false;
    state = !state;
    digitalWrite(LIGHT_PIN, state ? HIGH : LOW);
    String message = state ? "Đèn bật" : "Đèn tắt";
    errorManager.logInfo(message.c_str());
}

// ===== Multi-key state =====
bool forwardPressed = false, backwardPressed = false, leftPressed = false, rightPressed = false;

enum MovementState {
    MS_STOP, MS_FORWARD, MS_BACKWARD, MS_LEFT, MS_RIGHT, 
    MS_FORWARD_LEFT, MS_FORWARD_RIGHT
};
MovementState lastState = MS_STOP;

// ===== Button handlers =====
void setForward(bool state) {
    if (!debounce()) return;
    forwardPressed = state;
}

void setBackward(bool state) {
    if (!debounce()) return;
    backwardPressed = state;
}

void setLeft(bool state) {
    if (!debounce()) return;
    leftPressed = state;
}

void setRight(bool state) {
    if (!debounce()) return;
    rightPressed = state;
}

// ===== Motor actions cho 4 motor song song =====
void stopCar() {
    motorController.stopAll();
    if (lastState != MS_STOP) {
        errorManager.logInfo("Dừng xe");
    }
}

void forward() {
    // 4 motor cùng tiến (2 trái + 2 phải)
    motorController.setSpeeds(basePWM, basePWM);
}

void backward() {
    // 4 motor cùng lùi (2 trái + 2 phải)
    motorController.setSpeeds(-basePWM, -basePWM);
}

void leftTurn() {
    // 2 motor trái lùi, 2 motor phải tiến → quay trái
    motorController.setSpeeds(-basePWM, basePWM);
}

void rightTurn() {
    // 2 motor trái tiến, 2 motor phải lùi → quay phải
    motorController.setSpeeds(basePWM, -basePWM);
}

void forwardLeft() {
    int leftVal = (basePWM * 60) / 100;
    motorController.setSpeeds(leftVal, basePWM);
}

void forwardRight() {
    int rightVal = (basePWM * 60) / 100;
    motorController.setSpeeds(basePWM, rightVal);
}

// ===== Speed control =====
void setSpeed(int val) {
    currentSlider = constrain(val, 0, 10);
    basePWM = map(currentSlider, 0, 10, 0, 255);
    String message = "Điều chỉnh tốc độ: " + String(currentSlider) + "/10 (PWM: " + String(basePWM) + ")";
    errorManager.logInfo(message.c_str());
}

// ===== Handle movement logic =====
void handleMovement() {
    MovementState state = MS_STOP;

    if (forwardPressed && rightPressed) state = MS_FORWARD_RIGHT;
    else if (forwardPressed && leftPressed) state = MS_FORWARD_LEFT;
    else if (forwardPressed) state = MS_FORWARD;
    else if (backwardPressed) state = MS_BACKWARD;
    else if (rightPressed) state = MS_RIGHT;
    else if (leftPressed) state = MS_LEFT;
    else state = MS_STOP;

    if (state != lastState) {
        const char* stateNames[] = {
            "DỪNG", "TIẾN", "LÙI", "TRÁI", "PHẢI", "TIẾN TRÁI", "TIẾN PHẢI"
        };
        String message = "Trạng thái: " + String(stateNames[state]);
        errorManager.logInfo(message.c_str());
    }

    switch (state) {
        case MS_FORWARD: forward(); break;
        case MS_BACKWARD: backward(); break;
        case MS_LEFT: leftTurn(); break;
        case MS_RIGHT: rightTurn(); break;
        case MS_FORWARD_LEFT: forwardLeft(); break;
        case MS_FORWARD_RIGHT: forwardRight(); break;
        default: stopCar(); break;
    }
    
    lastState = state;
}

// ===== WebServer =====
WebServer server(80);
#include "webserver_module.h"  // Giữ nguyên file header của bạn

// ===== Khởi tạo WiFi =====
bool initializeWiFi() {
    String message1 = "Đang khởi động WiFi AP: " + String(ssid);
    errorManager.logInfo(message1.c_str());
    
    if (!WiFi.softAP(ssid, password)) {
        errorManager.logError(SYS_ERROR_WIFI, "Không thể khởi động AP");
        return false;
    }
    
    delay(1000);
    
    IPAddress ip = WiFi.softAPIP();
    if (ip == IPAddress(0,0,0,0)) {
        errorManager.logError(SYS_ERROR_WIFI, "AP không có IP");
        return false;
    }

    String message2 = "WiFi AP started: " + String(ssid) + ", IP: " + ip.toString();
    errorManager.logInfo(message2.c_str());
    
    return true;
}

// ===== Test hệ thống an toàn =====
void safeSystemTest() {
    errorManager.logInfo("Kiểm tra hệ thống 4 motor song song...");
    
    // Test đèn phụ trợ
    digitalWrite(LIGHT_PIN, HIGH);
    delay(500);
    digitalWrite(LIGHT_PIN, LOW);
    
    digitalWrite(HORN_PIN, HIGH);
    delay(200);
    digitalWrite(HORN_PIN, LOW);
    
    digitalWrite(HAZARD_PIN, HIGH);
    delay(500);
    digitalWrite(HAZARD_PIN, LOW);

    // Test motor với tốc độ thấp - DÙNG HÀM TEST MỚI
    if (motorController.areMotorsEnabled()) {
        errorManager.logInfo("Testing 4 motors parallel...");
        motorController.test4Motors();
        errorManager.logInfo("4 Motor test hoàn thành");
    } else {
        errorManager.logInfo("Motors disabled - skipping motor test");
    }
    
    errorManager.logInfo("Kiểm tra hoàn tất");
}

// ===== Setup & Loop =====
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    String message = "🚀 Khởi động ESP32-S3 Robot - 1 L298N + 4 Motor Song Song";
    errorManager.logInfo(message.c_str());

    // Khởi tạo GPIO
    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(HORN_PIN, OUTPUT);
    pinMode(HAZARD_PIN, OUTPUT);
    pinMode(STATUS_LED, OUTPUT);

    // Đảm bảo tất cả output ở LOW
    digitalWrite(LIGHT_PIN, LOW);
    digitalWrite(HORN_PIN, LOW);
    digitalWrite(HAZARD_PIN, LOW);
    digitalWrite(STATUS_LED, LOW);

    // Khởi tạo motor controller với API mới
    if (!motorController.initialize()) {
        errorManager.logError(SYS_ERROR_MOTOR, "Lỗi khởi tạo động cơ");
    } else {
        errorManager.logInfo("Motor controller khởi tạo thành công - 4 motor song song");
    }

    // Khởi tạo WiFi
    if (!initializeWiFi()) {
        errorManager.logError(SYS_ERROR_WIFI, "Lỗi khởi tạo WiFi");
    }

    // Khởi tạo web server
    setupRoutes();
    server.begin();
    errorManager.logInfo("Web server started");

    // Test hệ thống
    safeSystemTest();

    // Đặt tốc độ mặc định
    setSpeed(currentSlider);

    errorManager.logInfo("✅ Hệ thống 4 motor song song sẵn sàng!");
    errorManager.logInfo("📡 Kết nối WiFi: ESP32-4Motor");
    errorManager.logInfo("🌐 URL: http://192.168.4.1");
}

void loop() {
    server.handleClient();
    handleHorn();
    
    // Xử lý hazard
    if (hazardOn) {
        unsigned long now = millis();
        if (now - lastHazardBlink >= hazardInterval) {
            lastHazardBlink = now;
            digitalWrite(HAZARD_PIN, !digitalRead(HAZARD_PIN));
        }
    }

    handleMovement();
    errorManager.systemCheck();
}