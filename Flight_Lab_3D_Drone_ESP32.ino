/*
  ESP32 + MPU6050 3D Drone Visualization
  Board: ESP32 DevKit V1
  Sensor: MPU6050 (I2C)
  Default I2C pins: SDA=GPIO21, SCL=GPIO22

  IMPORTANT:
  - Change WIFI_PASSWORD before use. Use a password of at least 8 characters.
  - This is a visualization/demo, NOT a flight controller.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

const char* WIFI_SSID = "Drone-ESP32";
const char* WIFI_PASSWORD = "CHANGE_ME_123";  // Change this before use/public upload

WebServer server(80);

static const uint8_t MPU_ADDR = 0x68;
static const int SDA_PIN = 21;
static const int SCL_PIN = 22;

float rollDeg = 0.0f;
float pitchDeg = 0.0f;
float yawDeg = 0.0f;
float gyroXOffset = 0.0f, gyroYOffset = 0.0f, gyroZOffset = 0.0f;
unsigned long lastUpdateUs = 0;
bool mpuConnected = false;

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Drone Flight Lab</title>
<style>
:root{color-scheme:dark;--bg:#08111f;--panel:#111e31;--line:#263852;--cyan:#54d8e8;--yellow:#ffd166;--muted:#9db0c9}
*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at top,#152946,#08111f 65%);font-family:Arial,Helvetica,sans-serif;color:#eef6ff}
header{padding:22px 18px 12px;text-align:center}h1{margin:0;font-size:clamp(22px,4vw,34px);letter-spacing:.4px}header p{color:var(--muted);margin:8px 0}
main{max-width:1050px;margin:auto;padding:14px;display:grid;grid-template-columns:1.35fr 1fr;gap:14px}
.card{background:#111e31eF;border:1px solid var(--line);border-radius:18px;padding:16px;box-shadow:0 12px 35px #0003}
.card h2{font-size:16px;margin:0 0 14px;color:var(--cyan)}
.stage{height:330px;display:flex;align-items:center;justify-content:center;perspective:800px;overflow:hidden;border-radius:12px;background:radial-gradient(ellipse,#1b3451,#0b1728 70%)}
.drone{position:relative;width:230px;height:150px;transform-style:preserve-3d;transition:transform .08s linear}
.body{position:absolute;left:80px;top:49px;width:70px;height:53px;border-radius:16px;background:linear-gradient(145deg,#d7e5f3,#607d9c 55%,#253c59);border:2px solid #e8f7ff;box-shadow:0 8px 18px #0008;transform:translateZ(14px)}
.body:after{content:"ESP32";position:absolute;inset:18px 0 auto;text-align:center;font-size:10px;font-weight:bold;color:#0c1b2b}
.arm{position:absolute;left:108px;top:70px;width:14px;height:135px;border-radius:9px;background:linear-gradient(90deg,#607c9b,#d7e5f3,#506983);transform-origin:7px 7px;box-shadow:0 2px 5px #0008}
.arm.a{transform:rotate(45deg) translateY(-67px)}.arm.b{transform:rotate(-45deg) translateY(-67px)}
.motor{position:absolute;width:46px;height:46px;border:4px solid var(--cyan);border-radius:50%;background:#10283b;box-shadow:0 0 14px #54d8e866}
.m1{left:20px;top:0}.m2{right:20px;top:0}.m3{left:20px;bottom:0}.m4{right:20px;bottom:0}
.motor:after{content:"";position:absolute;left:5px;right:5px;top:17px;height:4px;border-radius:5px;background:#d8f9ff;transform:rotate(-25deg)}
.metrics{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}.metric{padding:14px 10px;border:1px solid var(--line);border-radius:12px;background:#0b1728;text-align:center}.metric span{display:block;color:var(--muted);font-size:12px;margin-bottom:7px}.metric strong{font-size:clamp(20px,3vw,28px);color:var(--yellow);font-variant-numeric:tabular-nums}.unit{font-size:12px;color:var(--muted)}
.status{display:flex;align-items:center;gap:8px;color:var(--muted);font-size:13px;margin-top:14px}.dot{width:9px;height:9px;border-radius:50%;background:#f87171}.dot.ok{background:#4ade80}
.note{font-size:12px;line-height:1.5;color:var(--muted);margin-top:12px}
@media(max-width:720px){main{grid-template-columns:1fr}.stage{height:280px}.metrics{gap:6px}.metric{padding:12px 5px}}
</style>
</head>
<body>
<header><h1>FLIGHT LAB <span style="color:var(--cyan)">/ 3D</span></h1><p>ESP32 + MPU6050 live attitude visualization</p></header>
<main>
<section class="card"><h2>3D DRONE MODEL</h2>
<div class="stage"><div class="drone" id="drone"><div class="arm a"></div><div class="arm b"></div><div class="motor m1"></div><div class="motor m2"></div><div class="motor m3"></div><div class="motor m4"></div><div class="body"></div></div></div>
<div class="status"><span class="dot" id="dot"></span><span id="connection">Connecting to sensor...</span></div>
<p class="note">Move the MPU6050 board gently to see the model tilt. Roll and pitch use accelerometer + gyroscope filtering; yaw is gyro-integrated and may drift.</p>
</section>
<section class="card"><h2>LIVE TELEMETRY</h2><div class="metrics">
<div class="metric"><span>ROLL</span><strong id="roll">0.0</strong><div class="unit">degrees</div></div>
<div class="metric"><span>PITCH</span><strong id="pitch">0.0</strong><div class="unit">degrees</div></div>
<div class="metric"><span>YAW</span><strong id="yaw">0.0</strong><div class="unit">degrees</div></div>
</div>
<div style="margin-top:18px;border-top:1px solid var(--line);padding-top:14px;color:var(--muted);font-size:13px;line-height:1.8">
<div>Sensor: <strong id="sensor" style="color:#eef6ff">Checking</strong></div>
<div>Update rate: <strong style="color:#eef6ff">~10 Hz</strong></div>
<div>Connection: <strong style="color:#eef6ff">ESP32 Access Point</strong></div>
</div>
<p class="note">Open this page while connected to the ESP32 Wi-Fi network. Keep your phone's mobile data enabled only if it doesn't interrupt the local connection.</p>
</section>
</main>
<script>
const $=id=>document.getElementById(id);
async function update(){
 try{
  const r=await fetch('/data',{cache:'no-store'}); if(!r.ok) throw new Error('HTTP '+r.status);
  const d=await r.json();
  $('roll').textContent=Number(d.roll).toFixed(1);
  $('pitch').textContent=Number(d.pitch).toFixed(1);
  $('yaw').textContent=Number(d.yaw).toFixed(1);
  $('drone').style.transform=`rotateX(${Number(d.pitch)}deg) rotateY(${Number(d.yaw)}deg) rotateZ(${-Number(d.roll)}deg)`;
  $('sensor').textContent=d.connected?'MPU6050 connected':'Not detected';
  $('connection').textContent=d.connected?'Live data received':'Sensor not detected';
  $('dot').className='dot'+(d.connected?' ok':'');
 }catch(e){$('connection').textContent='Connection lost — retrying';$('dot').className='dot';$('sensor').textContent='Offline';}
}
setInterval(update,150);update();
</script>
</body></html>
)HTML";

bool writeMPU(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readMPU(int16_t &ax, int16_t &ay, int16_t &az,
             int16_t &gx, int16_t &gy, int16_t &gz) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)MPU_ADDR, 14, true) != 14) return false;

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  (void)Wire.read(); (void)Wire.read(); // temperature
  gx = (Wire.read() << 8) | Wire.read();
  gy = (Wire.read() << 8) | Wire.read();
  gz = (Wire.read() << 8) | Wire.read();
  return true;
}

void calibrateGyro() {
  const int samples = 500;
  long sx = 0, sy = 0, sz = 0;
  int16_t ax, ay, az, gx, gy, gz;
  for (int i = 0; i < samples; i++) {
    if (readMPU(ax, ay, az, gx, gy, gz)) {
      sx += gx; sy += gy; sz += gz;
    }
    delay(3);
  }
  gyroXOffset = (float)sx / samples;
  gyroYOffset = (float)sy / samples;
  gyroZOffset = (float)sz / samples;
}

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  String json = "{\"roll\":" + String(rollDeg, 2) +
                ",\"pitch\":" + String(pitchDeg, 2) +
                ",\"yaw\":" + String(yawDeg, 2) +
                ",\"connected\":" + String(mpuConnected ? "true" : "false") + "}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  mpuConnected = writeMPU(0x6B, 0x00); // wake MPU6050
  if (mpuConnected) {
    writeMPU(0x1B, 0x00); // gyro +/- 250 degrees/sec
    writeMPU(0x1C, 0x00); // accel +/- 2g
    Serial.println("MPU6050 CONNECTED");
    Serial.println("Keep sensor still: calibrating gyro...");
    calibrateGyro();
    Serial.println("Gyro calibration complete");
  } else {
    Serial.println("MPU6050 NOT FOUND - check wiring/address");
  }

  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(apStarted ? "Wi-Fi AP READY" : "Wi-Fi AP FAILED");
  Serial.print("SSID: "); Serial.println(WIFI_SSID);
  Serial.print("Dashboard: http://"); Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.begin();
  Serial.println("WEB SERVER READY");
  lastUpdateUs = micros();
}

void loop() {
  server.handleClient();

  unsigned long nowUs = micros();
  float dt = (nowUs - lastUpdateUs) / 1000000.0f;
  if (dt < 0.01f) return;
  lastUpdateUs = nowUs;
  if (dt > 0.1f) dt = 0.01f;

  int16_t axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw;
  mpuConnected = readMPU(axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw);

  if (mpuConnected) {
    float ax = axRaw / 16384.0f;
    float ay = ayRaw / 16384.0f;
    float az = azRaw / 16384.0f;

    float accelRoll = atan2f(ay, az) * 180.0f / PI;
    float accelPitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / PI;

    float gx = (gxRaw - gyroXOffset) / 131.0f;
    float gy = (gyRaw - gyroYOffset) / 131.0f;
    float gz = (gzRaw - gyroZOffset) / 131.0f;

    const float alpha = 0.96f;
    rollDeg = alpha * (rollDeg + gx * dt) + (1.0f - alpha) * accelRoll;
    pitchDeg = alpha * (pitchDeg + gy * dt) + (1.0f - alpha) * accelPitch;
    yawDeg += gz * dt;
    if (yawDeg > 180.0f) yawDeg -= 360.0f;
    if (yawDeg < -180.0f) yawDeg += 360.0f;
  }
}
