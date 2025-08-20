#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>

const char* ssid = "SSID:	Sakhib’s iPhone";
const char* password = "sakhib1414";

ESP8266WebServer server(80);
Ticker ticker;

const int carRed = 5;
const int carYellow = 4;
const int carGreen = 14;
const int pedGreen = 12;

bool yellowBlinking = false;
bool yellowState = false;

void setup() {
  Serial.begin(9600);

  pinMode(carRed, OUTPUT);
  pinMode(carYellow, OUTPUT);
  pinMode(carGreen, OUTPUT);
  pinMode(pedGreen, OUTPUT);

  digitalWrite(carRed, LOW);
  digitalWrite(carYellow, LOW);
  digitalWrite(carGreen, LOW);
  digitalWrite(pedGreen, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }

  Serial.println("\nConnected to WiFi:");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/start", []() {
    stopYellowBlink();
    runTrafficSequence();
    server.send(200, "text/plain", "Sequence Started");
  });

  server.on("/toggle/red", []() {
    static bool redState = false;
    redState = !redState;
    digitalWrite(carRed, redState ? HIGH : LOW);
    server.send(200, "text/plain", redState ? "ON" : "OFF");
  });

  server.on("/toggle/green", []() {
    static bool greenState = false;
    greenState = !greenState;
    digitalWrite(carGreen, greenState ? HIGH : LOW);
    server.send(200, "text/plain", greenState ? "ON" : "OFF");
  });

  server.on("/toggle/yellow-blink", []() {
    yellowBlinking = !yellowBlinking;
    if (yellowBlinking) {
      ticker.attach_ms(500, []() {
        yellowState = !yellowState;
        digitalWrite(carYellow, yellowState ? HIGH : LOW);
      });
      server.send(200, "text/plain", "Blinking");
    } else {
      ticker.detach();
      digitalWrite(carYellow, LOW);
      server.send(200, "text/plain", "Stopped");
    }
  });

  server.begin();
}

void loop() {
  server.handleClient();
}

void stopYellowBlink() {
  yellowBlinking = false;
  ticker.detach();
  digitalWrite(carYellow, LOW);
}

void runTrafficSequence() {
  digitalWrite(pedGreen, HIGH); delay(3000);
  digitalWrite(pedGreen, LOW); delay(500);

  digitalWrite(carRed, HIGH); delay(2000);
  digitalWrite(carYellow, HIGH); delay(1000);

  digitalWrite(carRed, LOW);
  digitalWrite(carYellow, LOW);
  digitalWrite(carGreen, HIGH); delay(3000);

  digitalWrite(carGreen, LOW);
  digitalWrite(carYellow, HIGH); delay(1500);

  digitalWrite(carYellow, LOW);
  digitalWrite(carRed, HIGH);
  digitalWrite(pedGreen, HIGH);
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Traffic Light</title>
  <style>
    body {
      margin: 0;
      font-family: sans-serif;
      background: url('https://i.imgur.com/HWL2qmI.jpg') no-repeat center center fixed;
      background-size: cover;
      overflow: hidden;
    }

    .traffic-wrapper {
      position: fixed;
      bottom: 10px;
      left: 10px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }

    .light-box, .ped-box {
      background: #222;
      padding: 10px;
      border-radius: 10px;
      display: flex;
      flex-direction: column;
      gap: 10px;
      box-shadow: 0 0 8px rgba(0,0,0,0.6);
      transform: scale(0.85);
    }

    .light, .ped-light {
      width: 50px;
      height: 50px;
      border-radius: 50%;
      border: 2px solid white;
      background: #444;
      font-size: 0;
      cursor: pointer;
      margin: 4px auto;
    }

    .ped-light {
      font-size: 24px;
      line-height: 50px;
      text-align: center;
      color: white;
    }

    .red.on { background: red; }
    .yellow.on { background: yellow; }
    .green.on { background: limegreen; }

    .pole, .ground-pole {
      width: 8px;
      background: gray;
      border-radius: 4px;
      margin: 4px 0;
    }

    .pole { height: 50px; }
    .ground-pole { height: 120px; }

    button {
      margin-top: 10px;
      padding: 10px 20px;
      font-size: 14px;
      background: #333;
      color: white;
      border: none;
      border-radius: 8px;
      cursor: pointer;
    }

    #car {
      width: 120px;
      height: 60px;
      background: crimson;
      position: absolute;
      bottom: 25px;
      right: -150px;
      border-radius: 12px 12px 5px 5px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.5);
      transition: right 0.5s ease;
    }

    #car::before, #car::after {
      content: "";
      position: absolute;
      bottom: -12px;
      width: 25px;
      height: 25px;
      background: black;
      border-radius: 50%;
    }

    #car::before { left: 15px; }
    #car::after { right: 15px; }

    #pedestrian {
      position: absolute;
      bottom: 25px;
      left: -60px;
      width: 40px;
      height: 70px;
      transition: left 0.5s ease;
    }

    #pedestrian .head {
      width: 30px;
      height: 30px;
      background: #fff4dd;
      border-radius: 50%;
      margin: 0 auto;
    }

    #pedestrian .body {
      width: 24px;
      height: 40px;
      background: #007bff;
      margin: 2px auto 0 auto;
      border-radius: 4px;
    }

    #pedestrian.walking {
      animation: walk-across 5s linear forwards;
    }

    @keyframes walk-across {
      0% { left: -60px; }
      100% { left: 120%; }
    }
  </style>
</head>
<body>
  <div class="traffic-wrapper">
    <div class="light-box">
      <div id="car_red" class="light red" onclick="toggleLight('car_red')"></div>
      <div id="car_yellow" class="light yellow" onclick="toggleYellowBlink()"></div>
      <div id="car_green" class="light green" onclick="toggleLight('car_green')"></div>
    </div>

    <div class="pole"></div>

    <div class="ped-box">
      <div id="ped_red" class="ped-light red">🚷</div>
      <div id="ped_green" class="ped-light green">🚶</div>
    </div>

    <div class="ground-pole"></div>

    <button onclick="startCycle()">Start Sequence</button>
  </div>

  <div id="car"></div>
  <div id="pedestrian">
    <div class="head"></div>
    <div class="body"></div>
  </div>

  <script>
    function set(id, on) {
      document.getElementById(id).classList.toggle('on', on);
    }

    function delay(ms) {
      return new Promise(resolve => setTimeout(resolve, ms));
    }

    async function startCycle() {
      fetch('/start');

      set("ped_green", true);
      set("ped_red", false);
      startWalking();
      await delay(3000);

      set("ped_green", false);
      set("ped_red", true);
      await delay(500);

      set("car_red", true);
      moveCar(false);
      await delay(2000);

      set("car_yellow", true);
      await delay(1000);

      set("car_red", false);
      set("car_yellow", false);
      set("car_green", true);
      moveCar(true);
      await delay(3000);

      set("car_green", false);
      set("car_yellow", true);
      moveCar(false);
      await delay(1500);

      set("car_yellow", false);
      set("car_red", true);
      set("ped_green", true);
      set("ped_red", false);
      startWalking();
    }

    function toggleLight(id) {
      const el = document.getElementById(id);
      el.classList.toggle('on');

      if (id === "car_red") fetch("/toggle/red");
      if (id === "car_green") {
        fetch("/toggle/green");
        moveCar(el.classList.contains("on"));
      }
    }

    function toggleYellowBlink() {
      const el = document.getElementById("car_yellow");
      el.classList.toggle('on');
      fetch("/toggle/yellow-blink");
    }

    function moveCar(active) {
      const car = document.getElementById("car");
      if (active) {
        car.style.transition = "right 5s linear";
        car.style.right = "120%";
      } else {
        car.style.transition = "none";
        car.style.right = "-150px";
      }
    }

    function startWalking() {
      const ped = document.getElementById("pedestrian");
      ped.classList.remove("walking");
      void ped.offsetWidth;
      ped.classList.add("walking");
    }
  </script>
</body>
</html>
)rawliteral"
);
}