#include <WiFi.h>
#include <DHT.h>
#include <WebSocketsServer.h>

// =====================================================
// WIFI
// =====================================================

const char* ssid = "  ";
const char* password = " ";

// =====================================================
// HARDWARE
// =====================================================

#define DHTPIN 5
#define DHTTYPE DHT11

const int LIVING_LED  = 16;
const int BEDROOM_LED = 17;
const int KITCHEN_LED = 18;
const int STUDY_LED   = 19;

DHT dht(DHTPIN, DHTTYPE);

WiFiServer server(80);
WebSocketsServer webSocket(81);

unsigned long lastSensorUpdate = 0;


// =====================================================
// ROOM STATE BROADCAST
// =====================================================

void broadcastRoomStates() {

  String states =
    String(digitalRead(LIVING_LED)) + "," +
    String(digitalRead(BEDROOM_LED)) + "," +
    String(digitalRead(KITCHEN_LED)) + "," +
    String(digitalRead(STUDY_LED));

  webSocket.broadcastTXT("ROOMS:" + states);
}


// =====================================================
// WEBSOCKET
// =====================================================

void webSocketEvent(
  uint8_t num,
  WStype_t type,
  uint8_t *payload,
  size_t length
) {

  if (type == WStype_CONNECTED) {

    Serial.println("Dashboard connected");

    String states =
      String(digitalRead(LIVING_LED)) + "," +
      String(digitalRead(BEDROOM_LED)) + "," +
      String(digitalRead(KITCHEN_LED)) + "," +
      String(digitalRead(STUDY_LED));

    webSocket.sendTXT(
      num,
      "ROOMS:" + states
    );
  }

  else if (type == WStype_DISCONNECTED) {

    Serial.println("Dashboard disconnected");
  }
}


// =====================================================
// WEBPAGE
// =====================================================

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>HomeOS</title>

<style>

/* =====================================================
   DESIGN TOKENS
===================================================== */

:root {

  --bg: #f5f3ef;
  --surface: #ffffff;
  --surface-soft: #faf9f7;

  --text: #282725;
  --muted: #99958e;
  --muted-dark: #716e68;

  --accent: #e96f5f;
  --accent-soft: #fff0ec;

  --green: #62b89b;
  --green-soft: #eaf7f2;

  --border: #e9e5df;

  --shadow:
    0 8px 30px rgba(44, 39, 32, 0.06);

  --radius-lg: 24px;
  --radius-md: 17px;
}


/* =====================================================
   RESET
===================================================== */

* {
  box-sizing: border-box;
}

html,
body {
  margin: 0;
  padding: 0;
  min-height: 100%;
}

body {

  background: var(--bg);

  color: var(--text);

  font-family:
    Inter,
    -apple-system,
    BlinkMacSystemFont,
    "Segoe UI",
    Arial,
    sans-serif;

  font-size: 14px;
}


/* =====================================================
   APP
===================================================== */

.app {

  display: flex;

  min-height: 100vh;
}


/* =====================================================
   SIDEBAR
===================================================== */

.sidebar {

  width: 258px;

  flex-shrink: 0;

  background: #ffffff;

  border-right: 1px solid var(--border);

  padding: 25px 18px;

  display: flex;

  flex-direction: column;
}


/* BRAND */

.brand {

  display: flex;

  align-items: center;

  gap: 12px;

  padding: 3px 10px 28px;
}


.brand-icon {

  width: 38px;
  height: 38px;

  border-radius: 11px;

  background: var(--accent);

  position: relative;

  box-shadow:
    0 6px 14px rgba(233,111,95,.2);
}


.brand-icon:before {

  content: "";

  position: absolute;

  width: 15px;
  height: 12px;

  left: 11px;
  top: 14px;

  border: 2px solid white;

  border-radius: 3px;
}


.brand-icon:after {

  content: "";

  position: absolute;

  width: 12px;
  height: 12px;

  left: 13px;
  top: 7px;

  border-left: 2px solid white;
  border-top: 2px solid white;

  transform: rotate(45deg);
}


.brand-title {

  font-size: 17px;

  font-weight: 750;

  letter-spacing: -.4px;
}


.brand-sub {

  margin-top: 3px;

  color: #aaa59e;

  font-size: 9px;

  letter-spacing: 1.4px;
}


/* CONTROLLER */

.controller {

  border: 1px solid var(--border);

  border-radius: 17px;

  padding: 13px;

  background: #fdfcfb;

  display: flex;

  align-items: center;

  gap: 10px;
}


.avatar {

  width: 37px;
  height: 37px;

  border-radius: 50%;

  background: #304255;

  color: white;

  display: flex;

  align-items: center;

  justify-content: center;

  font-size: 10px;

  font-weight: 700;
}


.controller-name {

  font-size: 12px;

  font-weight: 650;
}


.controller-sub {

  font-size: 9px;

  color: #aaa59e;

  margin-top: 4px;
}


/* SIDEBAR INFORMATION */

.sidebar-section {

  margin-top: 30px;
}


.sidebar-label {

  padding: 0 12px;

  color: #aaa59e;

  font-size: 9px;

  letter-spacing: 1.5px;

  font-weight: 700;

  margin-bottom: 10px;
}


.info-item {

  padding: 12px;

  border-radius: 12px;

  color: #716d67;

  font-size: 11px;

  display: flex;

  justify-content: space-between;
}


.info-item span {

  color: #aaa59e;
}


/* CONNECTION */

.connection-card {

  margin-top: auto;

  padding: 15px;

  background: #f8f7f4;

  border: 1px solid var(--border);

  border-radius: 15px;
}


.connection-label {

  color: #aaa59e;

  font-size: 9px;

  letter-spacing: 1px;

  margin-bottom: 9px;
}


.connection-status {

  display: flex;

  align-items: center;

  gap: 8px;

  font-size: 10px;

  font-weight: 650;
}


.connection-dot {

  width: 7px;
  height: 7px;

  border-radius: 50%;

  background: #d2cdc5;
}


/* =====================================================
   MAIN
===================================================== */

.main {

  flex: 1;

  min-width: 0;
}


/* =====================================================
   TOPBAR
===================================================== */

.topbar {

  height: 78px;

  border-bottom: 1px solid var(--border);

  background: rgba(255,255,255,.7);

  display: flex;

  align-items: center;

  padding: 0 34px;
}


.breadcrumb {

  color: #aaa59e;

  font-size: 11px;
}


.breadcrumb strong {

  color: #4c4944;

  font-weight: 650;
}


/* =====================================================
   CONTENT
===================================================== */

.content {

  padding: 35px 50px 45px;

  max-width: 1450px;

  margin: auto;
}


/* =====================================================
   GREETING
===================================================== */

.greeting {

  margin-bottom: 25px;
}


.date-label {

  color: #aaa59e;

  font-size: 10px;

  margin-bottom: 9px;
}


.date-label span {

  color: var(--green);

  margin-right: 7px;
}


h1 {

  margin: 0;

  font-size: 35px;

  letter-spacing: -1.5px;

  line-height: 1.05;
}


.greeting-sub {

  color: #98948e;

  font-size: 12px;

  margin-top: 9px;
}


/* =====================================================
   TOP GRID
===================================================== */

.top-grid {

  display: grid;

  grid-template-columns:
    1.25fr
    1fr
    1fr;

  gap: 18px;

  margin-bottom: 18px;
}


/* =====================================================
   CARD
===================================================== */

.card {

  background: var(--surface);

  border: 1px solid var(--border);

  border-radius: var(--radius-lg);

  box-shadow: var(--shadow);

  overflow: hidden;
}


.card-inner {

  padding: 24px;
}


.card-label {

  color: #aaa59e;

  font-size: 9px;

  font-weight: 750;

  letter-spacing: 1.5px;

  margin-bottom: 8px;
}


.card-title {

  font-size: 18px;

  font-weight: 650;

  letter-spacing: -.5px;
}


/* =====================================================
   HOME ACTIVITY
===================================================== */

.activity-card {

  min-height: 300px;
}


.activity-top {

  display: flex;

  align-items: center;

  gap: 24px;

  margin-top: 20px;
}


/* RING */

.ring {

  width: 125px;
  height: 125px;

  border-radius: 50%;

  background:
    conic-gradient(
      var(--accent) var(--progress, 0%),
      #eeeae5 0
    );

  position: relative;

  flex-shrink: 0;
}


.ring:after {

  content: "";

  position: absolute;

  inset: 10px;

  background: white;

  border-radius: 50%;
}


.ring-content {

  position: absolute;

  inset: 0;

  z-index: 2;

  display: flex;

  flex-direction: column;

  align-items: center;

  justify-content: center;
}


.ring-number {

  font-size: 29px;

  font-weight: 700;
}


.ring-label {

  color: #aaa59e;

  font-size: 8px;

  margin-top: 3px;
}


.activity-number {

  font-size: 29px;

  font-weight: 700;

  letter-spacing: -1px;
}


.activity-description {

  color: #8f8b84;

  font-size: 10px;

  line-height: 1.6;

  margin-top: 7px;
}


.activity-bottom {

  border-top: 1px solid #f0ede9;

  margin-top: 21px;

  padding-top: 14px;

  display: flex;

  justify-content: space-between;

  align-items: center;
}


.room-count {

  color: #96918b;

  font-size: 9px;
}


.room-count strong {

  color: #45423e;

  margin-left: 4px;
}


.live-label {

  color: #59a489;

  font-size: 8px;
}


/* =====================================================
   SENSOR CARDS
===================================================== */

.sensor-card {

  min-height: 300px;
}


.sensor-header {

  display: flex;

  align-items: flex-start;

  justify-content: space-between;
}


.sensor-value {

  display: flex;

  align-items: baseline;

  gap: 5px;

  margin-top: 16px;
}


.sensor-number {

  font-size: 34px;

  font-weight: 700;

  letter-spacing: -1px;
}


.sensor-unit {

  color: #aaa59e;

  font-size: 11px;
}


.sensor-description {

  color: #aaa59e;

  font-size: 9px;

  margin-top: 3px;
}


.sensor-bar {

  height: 4px;

  background: #eeeae5;

  border-radius: 10px;

  overflow: hidden;

  margin-top: 14px;
}


.sensor-bar-fill {

  width: 0%;

  height: 100%;

  background: var(--accent);

  transition: width .5s;
}


/* =====================================================
   GRAPH
===================================================== */

.graph-title-row {

  display: flex;

  justify-content: space-between;

  align-items: center;

  margin-top: 15px;

  margin-bottom: 7px;
}


.graph-label {

  font-size: 8px;

  color: #aaa59e;

  letter-spacing: .8px;
}


.graph-live {

  font-size: 8px;

  color: #c96b5e;
}


.sensor-graph {

  width: 100%;

  height: 82px;

  display: block;

  border-radius: 9px;

  background: #faf9f7;
}


/* =====================================================
   MAIN GRID
===================================================== */

.main-grid {

  display: grid;

  grid-template-columns:
    minmax(0, 1.65fr)
    minmax(280px, .8fr);

  gap: 18px;
}


/* =====================================================
   ROOMS
===================================================== */

.rooms-card {

  min-height: 520px;
}


.rooms-header {

  display: flex;

  justify-content: space-between;

  align-items: center;

  padding: 22px 25px;

  border-bottom: 1px solid #eeeae5;
}


.rooms-header-right {

  color: #aaa59e;

  font-size: 8px;

  letter-spacing: 1px;
}


.rooms-grid {

  display: grid;

  grid-template-columns: 1fr 1fr;

  gap: 9px;

  padding: 12px;
}


/* =====================================================
   DARK ROOM UI
===================================================== */

.room {

  min-height: 225px;

  position: relative;

  overflow: hidden;

  cursor: pointer;

  background:
    linear-gradient(
      145deg,
      #0b151c,
      #101b21
    );

  border: 1px solid #253841;

  border-radius: 17px;

  transition:
    transform .25s ease,
    border .25s ease,
    box-shadow .25s ease;
}


.room:hover {

  transform: translateY(-2px);

  border-color: #3c515a;
}


.room.active {

  border-color: #a88432;

  background:
    radial-gradient(
      circle at 50% 35%,
      rgba(230,185,76,.17),
      transparent 55%
    ),
    linear-gradient(
      145deg,
      #111a20,
      #151b1d
    );

  box-shadow:
    inset 0 0 45px rgba(230,185,76,.05),
    0 10px 25px rgba(0,0,0,.08);
}


.room-top {

  position: absolute;

  left: 18px;

  right: 18px;

  top: 16px;

  display: flex;

  justify-content: space-between;

  z-index: 5;
}


.room-name {

  color: #f2f3f2;

  font-size: 14px;

  font-weight: 650;
}


.room-description {

  color: #627681;

  font-size: 8px;

  margin-top: 4px;
}


/* GOLD LIGHT */

.room-light {

  width: 10px;
  height: 10px;

  border-radius: 50%;

  background: #34424a;

  transition: .35s;
}


.room.active .room-light {

  background: #ffe18a;

  box-shadow:
    0 0 9px #ffe18a,
    0 0 23px rgba(255,214,110,.7),
    0 0 45px rgba(255,202,80,.3);
}


/* =====================================================
   ROOM OBJECTS
===================================================== */

.room-object {

  position: absolute;

  left: 50%;
  top: 53%;

  transform: translate(-50%,-50%);
}


/* TV */

.tv {

  width: 110px;
  height: 63px;

  background: #070b0e;

  border: 4px solid #2b3940;

  border-radius: 5px;
}


.tv-screen {

  position: absolute;

  inset: 5px;

  background:
    linear-gradient(
      135deg,
      #0c2029,
      #15313d
    );

  transition: .35s;
}


.room.active .tv-screen {

  background:
    radial-gradient(
      circle,
      #c99d43,
      #513f20
    );

  box-shadow:
    0 0 20px rgba(230,190,90,.18);
}


/* BED */

.bed {

  width: 135px;
  height: 82px;

  background: #27363d;

  border-radius: 7px;

  position: relative;
}


.bed:before {

  content: "";

  position: absolute;

  left: 9px;
  right: 9px;

  top: 9px;

  height: 30px;

  background: #b6c0c2;

  border-radius: 5px;
}


.bed:after {

  content: "";

  position: absolute;

  left: 0;
  right: 0;

  bottom: -8px;

  height: 9px;

  background: #172329;

  border-radius: 0 0 7px 7px;
}


/* KITCHEN */

.counter {

  width: 170px;
  height: 43px;

  background: #29383e;

  border-top: 5px solid #53676d;

  position: relative;
}


.counter:before {

  content: "";

  position: absolute;

  width: 48px;
  height: 22px;

  left: 18px;

  top: -28px;

  background: #172329;

  border: 2px solid #52656b;
}


.counter:after {

  content: "";

  position: absolute;

  width: 31px;
  height: 31px;

  right: 20px;

  top: -37px;

  border-radius: 50%;

  background: #090e11;

  border: 4px solid #29383e;
}


/* STUDY */

.study-desk {

  width: 170px;
  height: 12px;

  background: #34474f;

  position: relative;

  top: 33px;
}


.monitor {

  width: 90px;
  height: 54px;

  position: absolute;

  left: 40px;

  bottom: 8px;

  background: #070c0f;

  border: 4px solid #2d3c43;

  border-radius: 4px;
}


.monitor:after {

  content: "";

  position: absolute;

  width: 25px;
  height: 4px;

  background: #53666c;

  bottom: -12px;

  left: 29px;
}


/* =====================================================
   ROOM FOOTER
===================================================== */

.room-footer {

  position: absolute;

  left: 18px;

  right: 18px;

  bottom: 15px;

  display: flex;

  justify-content: space-between;

  align-items: center;

  z-index: 5;
}


.room-status {

  color: #536974;

  font-size: 8px;

  letter-spacing: 1px;

  font-weight: 650;
}


.room.active .room-status {

  color: #e6bd5d;
}


/* SWITCH */

.switch {

  width: 41px;
  height: 22px;

  border-radius: 20px;

  background: #18252c;

  border: 1px solid #30434b;

  padding: 2px;

  transition: .3s;
}


.switch-knob {

  width: 16px;
  height: 16px;

  border-radius: 50%;

  background: #5d6d74;

  transition: .3s;
}


.room.active .switch {

  background: #5e481a;

  border-color: #a98532;
}


.room.active .switch-knob {

  transform: translateX(18px);

  background: #ffe18a;

  box-shadow:
    0 0 9px rgba(255,220,120,.7);
}


/* =====================================================
   RIGHT COLUMN
===================================================== */

.side-column {

  display: flex;

  flex-direction: column;

  gap: 18px;
}


/* =====================================================
   MOODS
===================================================== */

.mood-card {

  min-height: 300px;
}


.mood-grid {

  display: grid;

  grid-template-columns: 1fr 1fr;

  gap: 9px;

  margin-top: 20px;
}


.mood {

  border: 1px solid var(--border);

  background: #fbfaf8;

  border-radius: 13px;

  padding: 15px 12px;

  text-align: left;

  cursor: pointer;

  transition: .2s;
}


.mood:hover {

  border-color: #e5b5ab;

  background: #fff5f2;

  transform: translateY(-1px);
}


.mood-name {

  color: #494640;

  font-size: 10px;

  font-weight: 700;

  letter-spacing: .6px;
}


.mood-description {

  color: #aaa59e;

  font-size: 8px;

  margin-top: 5px;
}


/* =====================================================
   LIGHT STATUS
===================================================== */

.status-card {

  flex: 1;
}


.status-list {

  display: flex;

  flex-direction: column;

  gap: 13px;

  margin-top: 17px;
}


.status-row {

  display: flex;

  align-items: center;

  gap: 10px;
}


.status-dot {

  width: 7px;
  height: 7px;

  border-radius: 50%;

  background: #d0ccc5;

  flex-shrink: 0;
}


.status-row.active .status-dot {

  background: #e3b74f;

  box-shadow:
    0 0 0 4px rgba(227,183,79,.12),
    0 0 9px rgba(227,183,79,.35);
}


.status-name {

  flex: 1;

  color: #55514b;

  font-size: 10px;
}


.status-value {

  color: #aaa59e;

  font-size: 8px;
}


.status-row.active .status-value {

  color: #bf9340;
}


/* =====================================================
   BOTTOM STATS
===================================================== */

.stats {

  display: grid;

  grid-template-columns:
    1fr 1fr 1fr;

  gap: 18px;

  margin-top: 18px;
}


.stat {

  background: white;

  border: 1px solid var(--border);

  border-radius: 18px;

  padding: 19px 22px;

  box-shadow: var(--shadow);
}


.stat-number {

  font-size: 25px;

  font-weight: 700;

  letter-spacing: -.8px;
}


.stat-label {

  color: #aaa59e;

  font-size: 9px;

  margin-top: 4px;

  letter-spacing: .3px;
}


.stat-note {

  color: #59a489;

  font-size: 8px;

  margin-top: 8px;
}


/* =====================================================
   RESPONSIVE
===================================================== */

@media(max-width:1150px) {

  .sidebar {

    width: 220px;
  }

  .content {

    padding-left: 28px;

    padding-right: 28px;
  }

}


@media(max-width:950px) {

  .sidebar {

    display: none;
  }

  .top-grid {

    grid-template-columns: 1fr;
  }

  .main-grid {

    grid-template-columns: 1fr;
  }

}


@media(max-width:650px) {

  .content {

    padding: 25px 15px;
  }

  .topbar {

    padding: 0 18px;
  }

  h1 {

    font-size: 28px;
  }

  .rooms-grid {

    grid-template-columns: 1fr;
  }

  .stats {

    grid-template-columns: 1fr;
  }

  .activity-top {

    flex-direction: column;

    align-items: flex-start;
  }

}

</style>

</head>


<body>


<div class="app">


<!-- ===================================================
     SIDEBAR
=================================================== -->

<aside class="sidebar">


  <div class="brand">

    <div class="brand-icon"></div>

    <div>

      <div class="brand-title">
        homeOS
      </div>

      <div class="brand-sub">
        SMART HOME
      </div>

    </div>

  </div>


  <div class="controller">

    <div class="avatar">
      HM
    </div>

    <div>

      <div class="controller-name">
        Home Manager
      </div>

      <div class="controller-sub">
        ESP32 Controller
      </div>

    </div>

  </div>


  <div class="sidebar-section">

    <div class="sidebar-label">
      HOME
    </div>

    <div class="info-item">
      Rooms
      <span>4</span>
    </div>

    <div class="info-item">
      Connected devices
      <span id="deviceCount">--</span>
    </div>

    <div class="info-item">
      Sensor
      <span>DHT11</span>
    </div>

  </div>


  <div class="connection-card">

    <div class="connection-label">
      CONNECTION
    </div>

    <div class="connection-status">

      <div
        id="connectionDot"
        class="connection-dot">
      </div>

      <span id="connectionText">
        Connecting...
      </span>

    </div>

  </div>


</aside>


<!-- ===================================================
     MAIN
=================================================== -->

<main class="main">


<header class="topbar">

  <div class="breadcrumb">

    Home

    <span style="margin:0 8px">
      >
    </span>

    <strong>
      Overview
    </strong>

  </div>

</header>


<div class="content">


<!-- ===================================================
     GREETING
=================================================== -->

<div class="greeting">

  <div class="date-label">

    <span>●</span>

    <span id="todayDate">
      TODAY
    </span>

  </div>


  <h1>
    Good morning, Home.
  </h1>


  <div class="greeting-sub">

    Here's your home's pulse for today.
    Everything important, at a glance.

  </div>

</div>


<!-- ===================================================
     TOP CARDS
=================================================== -->

<div class="top-grid">


<!-- HOME ACTIVITY -->

<div class="card activity-card">

  <div class="card-inner">

    <div class="card-label">
      HOME ACTIVITY
    </div>

    <div class="card-title">
      Your home is doing well
    </div>


    <div class="activity-top">


      <div
        class="ring"
        id="activityRing"
        style="--progress:0%"
      >

        <div class="ring-content">

          <div
            class="ring-number"
            id="ringNumber"
          >
            0
          </div>

          <div class="ring-label">
            ACTIVE
          </div>

        </div>

      </div>


      <div>

        <div
          class="activity-number"
          id="activeCount"
        >
          0 / 4
        </div>

        <div class="activity-description">

          Connected room lighting is
          synchronized with the ESP32
          controller.

        </div>

      </div>


    </div>


    <div class="activity-bottom">

      <div class="room-count">
        Lights active
        <strong id="activeRooms">
          0
        </strong>
      </div>

      <div class="live-label">
        LIVE SYSTEM
      </div>

    </div>

  </div>

</div>


<!-- TEMPERATURE -->

<div class="card sensor-card">

  <div class="card-inner">

    <div class="sensor-header">

      <div>

        <div class="card-label">
          TEMPERATURE
        </div>

        <div class="sensor-description">
          DHT11 environment sensor
        </div>

      </div>

    </div>


    <div class="sensor-value">

      <div
        class="sensor-number"
        id="temperature"
      >
        --.-
      </div>

      <div class="sensor-unit">
        °C
      </div>

    </div>


    <div class="sensor-bar">

      <div
        class="sensor-bar-fill"
        id="tempBar"
      ></div>

    </div>


    <div class="graph-title-row">

      <div class="graph-label">
        TEMPERATURE HISTORY
      </div>

      <div class="graph-live">
        LIVE
      </div>

    </div>


    <canvas
      id="temperatureGraph"
      class="sensor-graph"
    ></canvas>


  </div>

</div>


<!-- HUMIDITY -->

<div class="card sensor-card">

  <div class="card-inner">

    <div class="sensor-header">

      <div>

        <div class="card-label">
          HUMIDITY
        </div>

        <div class="sensor-description">
          Relative humidity
        </div>

      </div>

    </div>


    <div class="sensor-value">

      <div
        class="sensor-number"
        id="humidity"
      >
        --.-
      </div>

      <div class="sensor-unit">
        %
      </div>

    </div>


    <div class="sensor-bar">

      <div
        class="sensor-bar-fill"
        id="humidityBar"
      ></div>

    </div>


    <div class="graph-title-row">

      <div class="graph-label">
        HUMIDITY HISTORY
      </div>

      <div class="graph-live">
        LIVE
      </div>

    </div>


    <canvas
      id="humidityGraph"
      class="sensor-graph"
    ></canvas>


  </div>

</div>


</div>


<!-- ===================================================
     MAIN AREA
=================================================== -->

<div class="main-grid">


<!-- ROOMS -->

<div class="card rooms-card">


  <div class="rooms-header">

    <div>

      <div
        class="card-label"
        style="margin-bottom:5px"
      >
        YOUR HOME
      </div>

      <div class="card-title">
        Room controls
      </div>

    </div>


    <div class="rooms-header-right">
      SELECT A ROOM
    </div>

  </div>


  <div class="rooms-grid">


    <!-- LIVING -->

    <div
      class="room"
      id="livingRoom"
      onclick="toggleRoom('living')"
    >

      <div class="room-top">

        <div>

          <div class="room-name">
            Living Room
          </div>

          <div class="room-description">
            Entertainment & relaxation
          </div>

        </div>

        <div
          class="room-light"
          id="livingLight"
        ></div>

      </div>


      <div class="room-object">

        <div class="tv">

          <div class="tv-screen"></div>

        </div>

      </div>


      <div class="room-footer">

        <div
          class="room-status"
          id="livingState"
        >
          LIGHT OFF
        </div>

        <div class="switch">

          <div class="switch-knob"></div>

        </div>

      </div>

    </div>


    <!-- BEDROOM -->

    <div
      class="room"
      id="bedroomRoom"
      onclick="toggleRoom('bedroom')"
    >

      <div class="room-top">

        <div>

          <div class="room-name">
            Bedroom
          </div>

          <div class="room-description">
            Rest & sleep
          </div>

        </div>

        <div
          class="room-light"
          id="bedroomLight"
        ></div>

      </div>


      <div class="room-object">

        <div class="bed"></div>

      </div>


      <div class="room-footer">

        <div
          class="room-status"
          id="bedroomState"
        >
          LIGHT OFF
        </div>

        <div class="switch">

          <div class="switch-knob"></div>

        </div>

      </div>

    </div>


    <!-- KITCHEN -->

    <div
      class="room"
      id="kitchenRoom"
      onclick="toggleRoom('kitchen')"
    >

      <div class="room-top">

        <div>

          <div class="room-name">
            Kitchen
          </div>

          <div class="room-description">
            Cooking & preparation
          </div>

        </div>

        <div
          class="room-light"
          id="kitchenLight"
        ></div>

      </div>


      <div class="room-object">

        <div class="counter"></div>

      </div>


      <div class="room-footer">

        <div
          class="room-status"
          id="kitchenState"
        >
          LIGHT OFF
        </div>

        <div class="switch">

          <div class="switch-knob"></div>

        </div>

      </div>

    </div>


    <!-- STUDY -->

    <div
      class="room"
      id="studyRoom"
      onclick="toggleRoom('study')"
    >

      <div class="room-top">

        <div>

          <div class="room-name">
            Study Room
          </div>

          <div class="room-description">
            Focus & productivity
          </div>

        </div>

        <div
          class="room-light"
          id="studyLight"
        ></div>

      </div>


      <div class="room-object">

        <div class="study-desk">

          <div class="monitor"></div>

        </div>

      </div>


      <div class="room-footer">

        <div
          class="room-status"
          id="studyState"
        >
          LIGHT OFF
        </div>

        <div class="switch">

          <div class="switch-knob"></div>

        </div>

      </div>

    </div>


  </div>

</div>


<!-- ===================================================
     RIGHT SIDE
=================================================== -->

<div class="side-column">


<!-- MOODS -->

<div class="card mood-card">

  <div class="card-inner">

    <div class="card-label">
      SMART MOODS
    </div>

    <div class="card-title">
      Set the mood
    </div>


    <div class="mood-grid">


      <button
        class="mood"
        onclick="sceneMorning()"
      >

        <div class="mood-name">
          MORNING
        </div>

        <div class="mood-description">
          Start the day
        </div>

      </button>


      <button
        class="mood"
        onclick="sceneNight()"
      >

        <div class="mood-name">
          NIGHT
        </div>

        <div class="mood-description">
          Wind things down
        </div>

      </button>


      <button
        class="mood"
        onclick="sceneFocus()"
      >

        <div class="mood-name">
          FOCUS
        </div>

        <div class="mood-description">
          Study mode
        </div>

      </button>


      <button
        class="mood"
        onclick="sceneAway()"
      >

        <div class="mood-name">
          AWAY
        </div>

        <div class="mood-description">
          Secure the home
        </div>

      </button>


    </div>

  </div>

</div>


<!-- LIGHT STATUS -->

<div class="card status-card">

  <div class="card-inner">

    <div class="card-label">
      LIGHT STATUS
    </div>

    <div class="card-title">
      Connected rooms
    </div>


    <div class="status-list">


      <div
        class="status-row"
        id="livingStatus"
      >

        <div class="status-dot"></div>

        <div class="status-name">
          Living Room
        </div>

        <div
          class="status-value"
          id="livingValue"
        >
          OFF
        </div>

      </div>


      <div
        class="status-row"
        id="bedroomStatus"
      >

        <div class="status-dot"></div>

        <div class="status-name">
          Bedroom
        </div>

        <div
          class="status-value"
          id="bedroomValue"
        >
          OFF
        </div>

      </div>


      <div
        class="status-row"
        id="kitchenStatus"
      >

        <div class="status-dot"></div>

        <div class="status-name">
          Kitchen
        </div>

        <div
          class="status-value"
          id="kitchenValue"
        >
          OFF
        </div>

      </div>


      <div
        class="status-row"
        id="studyStatus"
      >

        <div class="status-dot"></div>

        <div class="status-name">
          Study Room
        </div>

        <div
          class="status-value"
          id="studyValue"
        >
          OFF
        </div>

      </div>


    </div>

  </div>

</div>


</div>


</div>


<!-- ===================================================
     STATS
=================================================== -->

<div class="stats">


  <div class="stat">

    <div
      class="stat-number"
      id="statActive"
    >
      0
    </div>

    <div class="stat-label">
      ACTIVE ROOMS
    </div>

    <div class="stat-note">
      Live from ESP32
    </div>

  </div>


  <div class="stat">

    <div
      class="stat-number"
      id="clock"
    >
      --:--
    </div>

    <div class="stat-label">
      LOCAL TIME
    </div>

    <div
      class="stat-note"
      id="clockDate"
    >
      ---
    </div>

  </div>


  <div class="stat">

    <div
      class="stat-number"
      id="lastUpdate"
    >
      --
    </div>

    <div class="stat-label">
      LAST SENSOR UPDATE
    </div>

    <div class="stat-note">
      DHT11
    </div>

  </div>


</div>


</div>

</main>

</div>


<script>

/* =====================================================
   ROOM STATE
===================================================== */

let roomState = {

  living: false,

  bedroom: false,

  kitchen: false,

  study: false

};


const routes = {

  living: "yellow",

  bedroom: "red",

  kitchen: "green",

  study: "blue"

};


/* =====================================================
   SENSOR HISTORY
===================================================== */

let temperatureHistory = [];

let humidityHistory = [];


/* =====================================================
   ROOM UI
===================================================== */

function updateRoomUI(room, state) {

  roomState[room] = state;


  const roomElement =
    document.getElementById(
      room + "Room"
    );


  roomElement.classList.toggle(
    "active",
    state
  );


  document.getElementById(
    room + "State"
  ).textContent =
    state
      ? "LIGHT ON"
      : "LIGHT OFF";


  const status =
    document.getElementById(
      room + "Status"
    );


  const value =
    document.getElementById(
      room + "Value"
    );


  status.classList.toggle(
    "active",
    state
  );


  value.textContent =
    state
      ? "ON"
      : "OFF";


  updateActivity();

}


/* =====================================================
   ACTIVITY
===================================================== */

function updateActivity() {

  let active = 0;


  Object.values(roomState)
    .forEach(
      state => {

        if (state)
          active++;

      }
    );


  document.getElementById(
    "activeCount"
  ).textContent =
    active + " / 4";


  document.getElementById(
    "activeRooms"
  ).textContent =
    active;


  document.getElementById(
    "statActive"
  ).textContent =
    active;


  document.getElementById(
    "ringNumber"
  ).textContent =
    active;


  const percentage =
    (active / 4) * 100;


  document.getElementById(
    "activityRing"
  ).style.setProperty(
    "--progress",
    percentage + "%"
  );

}


/* =====================================================
   ROOM CONTROL
===================================================== */

function toggleRoom(room) {

  const newState =
    !roomState[room];


  fetch(
    "/" +
    routes[room] +
    "/" +
    (
      newState
        ? "on"
        : "off"
    )
  );

}


/* =====================================================
   MOODS
===================================================== */

function sceneMorning() {

  fetch("/yellow/on");

  fetch("/green/on");

  fetch("/red/off");

  fetch("/blue/off");

}


function sceneNight() {

  fetch("/yellow/on");

  fetch("/red/on");

  fetch("/green/off");

  fetch("/blue/off");

}


function sceneFocus() {

  fetch("/blue/on");

  fetch("/yellow/off");

  fetch("/red/off");

  fetch("/green/off");

}


function sceneAway() {

  fetch("/yellow/off");

  fetch("/red/off");

  fetch("/green/off");

  fetch("/blue/off");

}


/* =====================================================
   WEBSOCKET
===================================================== */

let socket;


function connectSocket() {

  socket = new WebSocket(
    "ws://" +
    location.hostname +
    ":81/"
  );


  socket.onopen = function() {

    document.getElementById(
      "connectionText"
    ).textContent =
      "ESP32 connected";


    document.getElementById(
      "connectionDot"
    ).style.background =
      "#62b89b";


    document.getElementById(
      "deviceCount"
    ).textContent =
      "Online";

  };


  socket.onclose = function() {

    document.getElementById(
      "connectionText"
    ).textContent =
      "Reconnecting...";


    document.getElementById(
      "connectionDot"
    ).style.background =
      "#d7a944";


    document.getElementById(
      "deviceCount"
    ).textContent =
      "Offline";


    setTimeout(
      connectSocket,
      2000
    );

  };


  socket.onmessage =
    function(event) {


      /* ==============================================
         ROOM STATE
      ============================================== */

      if (
        event.data.startsWith(
          "ROOMS:"
        )
      ) {

        const states =
          event.data
            .substring(6)
            .split(",");


        updateRoomUI(
          "living",
          states[0] === "1"
        );


        updateRoomUI(
          "bedroom",
          states[1] === "1"
        );


        updateRoomUI(
          "kitchen",
          states[2] === "1"
        );


        updateRoomUI(
          "study",
          states[3] === "1"
        );


        return;

      }


      /* ==============================================
         SENSOR DATA
      ============================================== */

      const data =
        event.data.split(",");


      if (
        data.length < 2
      )
        return;


      const temperature =
        parseFloat(data[0]);


      const humidity =
        parseFloat(data[1]);


      if (
        isNaN(temperature) ||
        isNaN(humidity)
      )
        return;


      /* TEMPERATURE */

      document.getElementById(
        "temperature"
      ).textContent =
        temperature.toFixed(1);


      /* HUMIDITY */

      document.getElementById(
        "humidity"
      ).textContent =
        humidity.toFixed(1);


      /* BARS */

      document.getElementById(
        "tempBar"
      ).style.width =
        Math.min(
          100,
          temperature * 2
        ) + "%";


      document.getElementById(
        "humidityBar"
      ).style.width =
        Math.min(
          100,
          humidity
        ) + "%";


      /* HISTORY */

      temperatureHistory.push(
        temperature
      );


      humidityHistory.push(
        humidity
      );


      if (
        temperatureHistory.length > 30
      ) {

        temperatureHistory.shift();

      }


      if (
        humidityHistory.length > 30
      ) {

        humidityHistory.shift();

      }


      drawTemperatureGraph();

      drawHumidityGraph();


      document.getElementById(
        "lastUpdate"
      ).textContent =
        new Date()
          .toLocaleTimeString(
            [],
            {
              hour: "2-digit",
              minute: "2-digit",
              second: "2-digit"
            }
          );

    };

}


connectSocket();


/* =====================================================
   GENERIC GRAPH DRAWER
===================================================== */

function drawGraph(
  canvasId,
  values,
  lineColor,
  fillColor
) {

  const canvas =
    document.getElementById(
      canvasId
    );


  const rect =
    canvas.getBoundingClientRect();


  const dpr =
    window.devicePixelRatio || 1;


  canvas.width =
    rect.width * dpr;


  canvas.height =
    rect.height * dpr;


  const ctx =
    canvas.getContext("2d");


  ctx.setTransform(
    dpr,
    0,
    0,
    dpr,
    0,
    0
  );


  const width =
    rect.width;


  const height =
    rect.height;


  ctx.clearRect(
    0,
    0,
    width,
    height
  );


  /* GRID */

  ctx.strokeStyle =
    "rgba(100,100,100,.10)";

  ctx.lineWidth = 1;


  for (
    let y = 15;
    y < height;
    y += 20
  ) {

    ctx.beginPath();

    ctx.moveTo(
      0,
      y
    );

    ctx.lineTo(
      width,
      y
    );

    ctx.stroke();

  }


  if (
    values.length < 2
  )
    return;


  let min =
    Math.min(
      ...values
    );


  let max =
    Math.max(
      ...values
    );


  if (
    max === min
  ) {

    max += 1;

    min -= 1;

  }


  const padding = 8;


  min -= (
    max - min
  ) * .15;


  max += (
    max - min
  ) * .15;


  /* AREA */

  ctx.beginPath();


  values.forEach(
    (value, index) => {

      const x =
        index *
        (
          width /
          (values.length - 1)
        );


      const y =
        height -
        padding -
        (
          (
            value - min
          ) /
          (
            max - min
          )
        ) *
        (
          height -
          padding * 2
        );


      if (
        index === 0
      )
        ctx.moveTo(x,y);

      else
        ctx.lineTo(x,y);

    }
  );


  ctx.lineTo(
    width,
    height
  );


  ctx.lineTo(
    0,
    height
  );


  ctx.closePath();


  ctx.fillStyle =
    fillColor;


  ctx.fill();


  /* LINE */

  ctx.beginPath();


  values.forEach(
    (value, index) => {

      const x =
        index *
        (
          width /
          (values.length - 1)
        );


      const y =
        height -
        padding -
        (
          (
            value - min
          ) /
          (
            max - min
          )
        ) *
        (
          height -
          padding * 2
        );


      if (
        index === 0
      )
        ctx.moveTo(x,y);

      else
        ctx.lineTo(x,y);

    }
  );


  ctx.strokeStyle =
    lineColor;


  ctx.lineWidth = 2;


  ctx.lineJoin =
    "round";


  ctx.lineCap =
    "round";


  ctx.stroke();


  /* CURRENT POINT */

  const last =
    values[
      values.length - 1
    ];


  const lastX =
    width;


  const lastY =
    height -
    padding -
    (
      (
        last - min
      ) /
      (
        max - min
      )
    ) *
    (
      height -
      padding * 2
    );


  ctx.beginPath();

  ctx.arc(
    lastX - 1,
    lastY,
    3,
    0,
    Math.PI * 2
  );


  ctx.fillStyle =
    lineColor;


  ctx.fill();

}


/* =====================================================
   TEMPERATURE GRAPH
===================================================== */

function drawTemperatureGraph() {

  drawGraph(
    "temperatureGraph",
    temperatureHistory,
    "#df806f",
    "rgba(223,128,111,.08)"
  );

}


/* =====================================================
   HUMIDITY GRAPH
===================================================== */

function drawHumidityGraph() {

  drawGraph(
    "humidityGraph",
    humidityHistory,
    "#78a58f",
    "rgba(120,165,143,.08)"
  );

}


/* =====================================================
   RESIZE
===================================================== */

window.addEventListener(
  "resize",
  function() {

    drawTemperatureGraph();

    drawHumidityGraph();

  }
);


/* =====================================================
   CLOCK
===================================================== */

function updateClock() {

  const now =
    new Date();


  document.getElementById(
    "clock"
  ).textContent =
    now.toLocaleTimeString(
      [],
      {
        hour: "2-digit",
        minute: "2-digit"
      }
    );


  document.getElementById(
    "clockDate"
  ).textContent =
    now.toLocaleDateString(
      [],
      {
        weekday: "long",
        day: "2-digit",
        month: "short"
      }
    );


  document.getElementById(
    "todayDate"
  ).textContent =
    now.toLocaleDateString(
      [],
      {
        weekday: "long",
        day: "2-digit",
        month: "long",
        year: "numeric"
      }
    );

}


updateClock();

setInterval(
  updateClock,
  1000
);

</script>

</body>

</html>

)rawliteral";


// =====================================================
// SETUP
// =====================================================

void setup() {

  pinMode(
    LIVING_LED,
    OUTPUT
  );

  pinMode(
    BEDROOM_LED,
    OUTPUT
  );

  pinMode(
    KITCHEN_LED,
    OUTPUT
  );

  pinMode(
    STUDY_LED,
    OUTPUT
  );


  digitalWrite(
    LIVING_LED,
    LOW
  );

  digitalWrite(
    BEDROOM_LED,
    LOW
  );

  digitalWrite(
    KITCHEN_LED,
    LOW
  );

  digitalWrite(
    STUDY_LED,
    LOW
  );


  Serial.begin(9600);

  delay(1000);


  dht.begin();


  Serial.println();

  Serial.println(
    "Connecting to WiFi..."
  );


  WiFi.begin(
    ssid,
    password
  );


  while (
    WiFi.status() != WL_CONNECTED
  ) {

    delay(500);

    Serial.print(".");

  }


  Serial.println();

  Serial.println(
    "WiFi connected!"
  );


  Serial.print(
    "IP Address: "
  );

  Serial.println(
    WiFi.localIP()
  );


  server.begin();


  webSocket.begin();

  webSocket.onEvent(
    webSocketEvent
  );


  Serial.println(
    "WebSocket started"
  );

}


// =====================================================
// LOOP
// =====================================================

void loop() {

  webSocket.loop();


  // ===================================================
  // DHT11
  // ===================================================

  if (
    millis() -
    lastSensorUpdate >=
    2000
  ) {

    lastSensorUpdate =
      millis();


    float temperature =
      dht.readTemperature();


    float humidity =
      dht.readHumidity();


    if (
      !isnan(temperature) &&
      !isnan(humidity)
    ) {

      String data =
        String(
          temperature,
          1
        ) +
        "," +
        String(
          humidity,
          1
        );


      webSocket.broadcastTXT(
        data
      );

    }

  }


  // ===================================================
  // HTTP
  // ===================================================

  WiFiClient client =
    server.available();


  if (!client)
    return;


  String request = "";

  unsigned long timeout =
    millis();


  while (
    client.connected() &&
    millis() - timeout < 1000
  ) {

    if (client.available()) {

      char c =
        client.read();


      request += c;


      if (c == '\n') {


        // =============================================
        // LIVING ROOM
        // =============================================

        if (
          request.indexOf(
            "GET /yellow/on"
          ) >= 0
        ) {

          digitalWrite(
            LIVING_LED,
            HIGH
          );

          broadcastRoomStates();

        }


        if (
          request.indexOf(
            "GET /yellow/off"
          ) >= 0
        ) {

          digitalWrite(
            LIVING_LED,
            LOW
          );

          broadcastRoomStates();

        }


        // =============================================
        // BEDROOM
        // =============================================

        if (
          request.indexOf(
            "GET /red/on"
          ) >= 0
        ) {

          digitalWrite(
            BEDROOM_LED,
            HIGH
          );

          broadcastRoomStates();

        }


        if (
          request.indexOf(
            "GET /red/off"
          ) >= 0
        ) {

          digitalWrite(
            BEDROOM_LED,
            LOW
          );

          broadcastRoomStates();

        }


        // =============================================
        // KITCHEN
        // =============================================

        if (
          request.indexOf(
            "GET /green/on"
          ) >= 0
        ) {

          digitalWrite(
            KITCHEN_LED,
            HIGH
          );

          broadcastRoomStates();

        }


        if (
          request.indexOf(
            "GET /green/off"
          ) >= 0
        ) {

          digitalWrite(
            KITCHEN_LED,
            LOW
          );

          broadcastRoomStates();

        }


        // =============================================
        // STUDY
        // =============================================

        if (
          request.indexOf(
            "GET /blue/on"
          ) >= 0
        ) {

          digitalWrite(
            STUDY_LED,
            HIGH
          );

          broadcastRoomStates();

        }


        if (
          request.indexOf(
            "GET /blue/off"
          ) >= 0
        ) {

          digitalWrite(
            STUDY_LED,
            LOW
          );

          broadcastRoomStates();

        }


        // =============================================
        // WEB PAGE
        // =============================================

        client.println(
          "HTTP/1.1 200 OK"
        );

        client.println(
          "Content-Type: text/html"
        );

        client.println(
          "Connection: close"
        );

        client.println();

        client.write(
          webpage,
          strlen_P(webpage)
        );

        break;

      }

    }

  }


  delay(2);

  client.stop();

}
