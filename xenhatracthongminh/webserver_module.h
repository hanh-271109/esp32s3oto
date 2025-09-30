#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H

#include <WebServer.h>
extern WebServer server;

// ================= THÔNG TIN CÁ NHÂN HOÀNG ANH =================
#define OWNER_NAME "Hoang Anh"
#define OWNER_BIRTHDAY "27/11/2009"
#define OWNER_SCHOOL "THPT Phuc Tho"
#define OWNER_CLASS "11A3"
#define OWNER_FACEBOOK "Nguyen Hoang Anh"
#define PROJECT_NAME "ESP32-S3 Robot với L298N"
#define VERSION "2.0"
#define CREATION_DATE "12/2024"
#define OWNER_SIGNATURE "HA_" OWNER_CLASS "_" CREATION_DATE

// Khai báo các biến và hàm từ file .ino
extern bool forwardPressed;
extern bool backwardPressed;
extern bool leftPressed;
extern bool rightPressed;

// Khai báo các hàm điều khiển
void setForward(bool state);
void setBackward(bool state);
void setLeft(bool state);
void setRight(bool state);
void stopCar();
void toggleLight();
void horn();
void toggleHazard();
void setSpeed(int val);

// Khai báo các hàm mới từ file .ino
extern int currentSlider;
extern int basePWM;
extern bool hazardOn;
extern bool hornActive;

// ================= HTML CÁ NHÂN HÓA CHO HOÀNG ANH =================
String htmlPage() {
  String page = R"rawliteral(
<!doctype html>
<html lang="vi">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width,initial-scale=1" />
<meta name="author" content=")rawliteral" + String(OWNER_NAME) + R"rawliteral(">
<meta name="description" content="ESP32-S3 Robot Control với L298N designed by )rawliteral" + String(OWNER_NAME) + R"rawliteral( - )rawliteral" + String(OWNER_SCHOOL) + R"rawliteral(">
<title>ESP32-S3 Robot Control - )rawliteral" + String(OWNER_NAME) + R"rawliteral(</title>
<style>
  /* Nền cờ Việt Nam đơn giản */
  body {
    margin: 0;
    font-family: sans-serif;
    background: linear-gradient(to bottom, #DA251D 0%, #DA251D 50%, #FFFF00 50%, #FFFF00 100%);
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 18px;
  }

  /* Card điều khiển giữ nguyên */
  .card {
    width: 720px;
    height: 140px;
    background: rgba(255, 255, 255, 0.95);
    display: grid;
    grid-template-columns: 1fr 1fr 90px 90px;
    grid-template-rows: 1fr 1fr;
    gap: 12px;
    padding: 12px;
    border-radius: 6px;
    box-shadow: 0 6px 18px rgba(0,0,0,0.1);
  }
  
  /* Giữ nguyên tất cả các style cũ */
  .box { background: #f0f0f0; border-radius: 6px; display: flex; align-items: center; justify-content: center; }
  .up { grid-column: 1/2; grid-row: 1/2; }
  .down { grid-column: 1/2; grid-row: 2/3; }
  .controls { grid-column: 2/3; grid-row: 1/2; padding: 8px; gap: 8px; display: flex; align-items: center; justify-content: flex-start; }
  .slider-wrap { display: flex; align-items: center; gap: 8px; }
  .park { grid-column: 2/3; grid-row: 2/3; display: flex; align-items: center; justify-content: center; }
  .right-1 { grid-column: 3/4; grid-row: 1/3; display: flex; align-items: center; justify-content: center; }
  .right-2 { grid-column: 4/5; grid-row: 1/3; display: flex; align-items: center; justify-content: center; }
  .arrow-box { width: 70%; height: 70%; background: #e0e0e0; border-radius: 6px; display: flex; align-items: center; justify-content: center; cursor: pointer; transition: background 0.2s; }
  .arrow-svg { width: 32px; height: 32px; fill: #2f2f2f; }
  .up .arrow-box, .down .arrow-box { width: 76px; height: 76px; }
  .right-1 .arrow-box, .right-2 .arrow-box { width: 68px; height: 120px; }
  .p-btn { width: 56px; height: 56px; background: #e0e0e0; border-radius: 6px; display: flex; align-items: center; justify-content: center; font-weight: 700; color: #2f2f2f; font-size: 20px; cursor: pointer; }
  .small-icon { width: 36px; height: 36px; background: #f8f8f8; border-radius: 6px; display: flex; align-items: center; justify-content: center; cursor: pointer; }
  .active { background: #7ac0ff !important; }
  
  /* Badge chủ sở hữu đơn giản */
  .owner-badge {
    position: fixed;
    bottom: 10px;
    right: 10px;
    background: rgba(0,0,0,0.8);
    color: white;
    padding: 6px 10px;
    border-radius: 12px;
    font-size: 11px;
    font-family: Arial, sans-serif;
  }
  .owner-name {
    font-weight: bold;
    color: #7ac0ff;
  }
  .school-info {
    font-size: 10px;
    opacity: 0.8;
  }

  /* Hiển thị trạng thái motor */
  .status-panel {
    position: fixed;
    top: 10px;
    left: 10px;
    background: white;
    padding: 10px;
    border-radius: 5px;
    font-size: 12px;
    box-shadow: 0 2px 5px rgba(0,0,0,0.2);
  }
</style>
</head>
<body>
<div class="status-panel">
  <div>Status: <span id="status">Ready</span></div>
  <div>Speed: <span id="speed">5</span>/10</div>
  <div>Motor: <span id="motorStatus">L298N Driver</span></div>
</div>

<div class="card">
  <!-- Up -->
  <div class="box up" title="Up">
    <div class="arrow-box" id="upBtn"
      onmousedown="press('forward')"
      onmouseup="release('forward')"
      ontouchstart="press('forward')"
      ontouchend="release('forward')"
      onmouseleave="release('forward')">
      <svg class="arrow-svg" viewBox="0 0 24 24"><path d="M12 4l-8 8h5v8h6v-8h5z"/></svg>
    </div>
  </div>
  
  <!-- Controls -->
  <div class="box controls">
    <div class="slider-wrap">
      <span style="font-size:12px;">Speed:</span>
      <input type="range" min="0" max="10" value="5" id="range1" oninput="setSpeed(this.value)" />
      <div class="small-icon" title="Horn" onclick="sendCommand('horn')">🔊</div>
      <div class="small-icon" title="Light" onclick="sendCommand('light')">💡</div>
      <div class="small-icon" title="Hazard" onclick="sendCommand('hazard')">🚨</div>
    </div>
  </div>
  
  <!-- Parking -->
  <div class="box park"><div class="p-btn" onclick="sendCommand('stop')">(P)</div></div>
  
  <!-- Down -->
  <div class="box down" title="Down">
    <div class="arrow-box" id="downBtn"
      onmousedown="press('backward')"
      onmouseup="release('backward')"
      ontouchstart="press('backward')"
      ontouchend="release('backward')"
      onmouseleave="release('backward')">
      <svg class="arrow-svg" viewBox="0 0 24 24"><path d="M12 20l8-8h-5V4h-6v8H4z"/></svg>
    </div>
  </div>
  
  <!-- Left -->
  <div class="box right-1" title="Left">
    <div class="arrow-box" id="leftBtn"
      onmousedown="press('left')"
      onmouseup="release('left')"
      ontouchstart="press('left')"
      ontouchend="release('left')"
      onmouseleave="release('left')">
      <svg class="arrow-svg" viewBox="0 0 24 24"><path d="M15 6l-6 6 6 6"/></svg>
    </div>
  </div>
  
  <!-- Right -->
  <div class="box right-2" title="Right">
    <div class="arrow-box" id="rightBtn"
      onmousedown="press('right')"
      onmouseup="release('right')"
      ontouchstart="press('right')"
      ontouchend="release('right')"
      onmouseleave="release('right')">
      <svg class="arrow-svg" viewBox="0 0 24 24"><path d="M9 6l6 6-6 6"/></svg>
    </div>
  </div>
</div>

<!-- Badge chủ sở hữu -->
<div class="owner-badge">
    <div>Designed by <span class="owner-name">)rawliteral" + String(OWNER_NAME) + R"rawliteral(</span></div>
    <div class="school-info">)rawliteral" + String(OWNER_SCHOOL) + R"rawliteral( - )rawliteral" + String(OWNER_CLASS) + R"rawliteral(</div>
    <div class="school-info">ESP32-S3 + L298N</div>
</div>

<script>
let activeButtons = new Set();

function updateButtonAppearance() {
  document.getElementById('upBtn').classList.toggle('active', activeButtons.has('forward'));
  document.getElementById('downBtn').classList.toggle('active', activeButtons.has('backward'));
  document.getElementById('leftBtn').classList.toggle('active', activeButtons.has('left'));
  document.getElementById('rightBtn').classList.toggle('active', activeButtons.has('right'));
}

function press(cmd) {
  console.log('Press:', cmd);
  activeButtons.add(cmd);
  updateButtonAppearance();
  sendCommand(cmd, '1');
  updateStatus();
}

function release(cmd) {
  console.log('Release:', cmd);
  activeButtons.delete(cmd);
  updateButtonAppearance();
  sendCommand(cmd, '0');
  updateStatus();
}

function updateStatus() {
  const status = activeButtons.size > 0 ? Array.from(activeButtons).join('+') : 'STOP';
  document.getElementById('status').textContent = status;
}

function sendCommand(cmd, val = '1') {
  const url = `/control?cmd=${cmd}&val=${val}`;
  console.log('Sending:', url);
  
  fetch(url)
    .then(response => response.text())
    .then(data => {
      console.log('Response:', data);
    })
    .catch(err => {
      console.error('Error:', err);
    });
}

function setSpeed(val) {
  console.log('Set speed:', val);
  document.getElementById('speed').textContent = val;
  sendCommand('speed', val);
}

// Auto-release khi rời khỏi trang
window.addEventListener('blur', function() {
  console.log('Page lost focus - releasing all buttons');
  activeButtons.forEach(cmd => {
    sendCommand(cmd, '0');
  });
  activeButtons.clear();
  updateButtonAppearance();
  updateStatus();
});

// Auto-release khi tải trang
window.addEventListener('beforeunload', function() {
  activeButtons.forEach(cmd => {
    fetch(`/control?cmd=${cmd}&val=0`, {method: 'GET', keepalive: true});
  });
});

console.log('ESP32-S3 Robot Control loaded - Designed by )rawliteral" + String(OWNER_NAME) + R"rawliteral(');
</script>
</body>
</html>
)rawliteral";
  return page;
}

// ================= ROUTES CÁ NHÂN HÓA =================
void setupRoutes() {
  server.on("/", [](){ 
    Serial.println("📱 Serving HTML page to: " + server.client().remoteIP().toString());
    Serial.println("   👤 Owner: " + String(OWNER_NAME));
    Serial.println("   🔧 Hardware: ESP32-S3 + L298N");
    server.send(200,"text/html",htmlPage()); 
  });

  // Route thông tin cá nhân
  server.on("/info", [](){
    String info = "🤖 === ESP32-S3 ROBOT CONTROLLER ===\n";
    info += "🔧 Hardware: ESP32-S3 + L298N Motor Driver\n";
    info += "👤 Owner: " + String(OWNER_NAME) + "\n";
    info += "🎂 Birthday: " + String(OWNER_BIRTHDAY) + "\n";
    info += "🏫 School: " + String(OWNER_SCHOOL) + "\n";
    info += "📚 Class: " + String(OWNER_CLASS) + "\n";
    info += "📘 Facebook: " + String(OWNER_FACEBOOK) + "\n";
    info += "🔖 Project: " + String(PROJECT_NAME) + "\n";
    info += "🔄 Version: " + String(VERSION) + "\n";
    info += "📅 Created: " + String(CREATION_DATE) + "\n";
    info += "🔐 Signature: " + String(OWNER_SIGNATURE) + "\n";
    info += "⏰ Uptime: " + String(millis()/1000) + " seconds\n";
    info += "🌐 Client IP: " + server.client().remoteIP().toString() + "\n";
    info += "🚀 Motor Driver: L298N (PWM Controlled)\n";
    info += "📊 Current Speed: " + String(currentSlider) + "/10\n";
    info += "💡 Light: " + String(digitalRead(4) ? "ON" : "OFF") + "\n";
    info += "🚨 Hazard: " + String(hazardOn ? "ON" : "OFF") + "\n";
    info += "📯 Horn: " + String(hornActive ? "ACTIVE" : "OFF") + "\n";
    
    Serial.println("🔍 Info requested by: " + server.client().remoteIP().toString());
    server.send(200, "text/plain", info);
  });

  // Route chính cho điều khiển
  server.on("/control", [](){
    if(server.hasArg("cmd") && server.hasArg("val")) {
      String command = server.arg("cmd");
      String value = server.arg("val");
      
      Serial.printf("🎮 Control by %s (%s): %s=%s\n", 
                   server.client().remoteIP().toString().c_str(),
                   String(OWNER_NAME).c_str(),
                   command.c_str(), 
                   value.c_str());
      
      if(command == "forward") {
        setForward(value == "1");
      } else if(command == "backward") {
        setBackward(value == "1");
      } else if(command == "left") {
        setLeft(value == "1");
      } else if(command == "right") {
        setRight(value == "1");
      } else if(command == "speed") {
        setSpeed(value.toInt());
      } else if(command == "horn") {
        horn();
      } else if(command == "light") {
        toggleLight();
      } else if(command == "hazard") {
        toggleHazard();
      } else if(command == "stop") {
        setForward(false);
        setBackward(false);
        setLeft(false);
        setRight(false);
        stopCar();
      }
      
      server.send(200, "text/plain", "OK:" + command + "=" + value + " - " + String(OWNER_NAME));
    } else {
      server.send(400, "text/plain", "Missing cmd or val");
    }
  });

  // Route signature
  server.on("/signature", [](){
    String signature = "✨ === PERSONAL SIGNATURE ===\n";
    signature += "Designed by: " + String(OWNER_NAME) + "\n";
    signature += "Student at: " + String(OWNER_SCHOOL) + "\n"; 
    signature += "Class: " + String(OWNER_CLASS) + "\n";
    signature += "Hardware: ESP32-S3 + L298N\n";
    signature += "Unique ID: " + String(OWNER_SIGNATURE) + "\n";
    signature += "Project: " + String(PROJECT_NAME) + "\n";
    
    server.send(200, "text/plain", signature);
  });

  // Route trạng thái hệ thống
  server.on("/status", [](){
    String status = "📊 === SYSTEM STATUS ===\n";
    status += "Forward: " + String(forwardPressed ? "ON" : "OFF") + "\n";
    status += "Backward: " + String(backwardPressed ? "ON" : "OFF") + "\n";
    status += "Left: " + String(leftPressed ? "ON" : "OFF") + "\n";
    status += "Right: " + String(rightPressed ? "ON" : "OFF") + "\n";
    status += "Speed: " + String(currentSlider) + "/10\n";
    status += "PWM Value: " + String(basePWM) + "/255\n";
    status += "Hazard: " + String(hazardOn ? "ON" : "OFF") + "\n";
    status += "Horn: " + String(hornActive ? "ACTIVE" : "OFF") + "\n";
    status += "Light: " + String(digitalRead(4) ? "ON" : "OFF") + "\n";
    
    server.send(200, "text/plain", status);
  });

  // Route dự phòng
  server.on("/stop", [](){
    Serial.println("🛑 Stop route called by " + String(OWNER_NAME));
    setForward(false);
    setBackward(false);
    setLeft(false);
    setRight(false);
    stopCar();
    server.send(200,"text/plain","Stop - " + String(OWNER_NAME));
  });

  server.onNotFound([](){
    Serial.printf("❌ 404 by %s: %s\n", String(OWNER_NAME).c_str(), server.uri().c_str());
    server.send(404,"text/plain","Not Found - Designed by " + String(OWNER_NAME));
  });
}

// ================= HIỂN THỊ THÔNG TIN KHI KHỞI ĐỘNG =================
void displayOwnerInfo() {
  Serial.println("\n");
  Serial.println("┌─────────────────────────────────────────────┐");
  Serial.println("│        " + String(PROJECT_NAME) + "         │");
  Serial.println("├─────────────────────────────────────────────┤");
  Serial.println("│ Designed by: " + String(OWNER_NAME) + "               │");
  Serial.println("│ School: " + String(OWNER_SCHOOL) + "                  │");
  Serial.println("│ Class: " + String(OWNER_CLASS) + "                       │");
  Serial.println("│ Hardware: ESP32-S3 + L298N Motor Driver    │");
  Serial.println("│ Signature: " + String(OWNER_SIGNATURE) + "           │");
  Serial.println("│ Web Server: http://esp32-robot.local       │");
  Serial.println("│ Routes: /, /info, /status, /control        │");
  Serial.println("└─────────────────────────────────────────────┘");
  Serial.println();
}

#endif