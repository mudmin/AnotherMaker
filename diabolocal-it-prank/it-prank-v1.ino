/* AnotherMaker IT Prank
   Requires FabGL Library in the Arduino IDE
   While this can be done on any ESP32 board, the ones with the built in VGA are cleaner
   https://www.banggood.com/custlink/DvKRbJiRWl
   https://amzn.to/3pe9fQa
/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <http://www.fabgl.com>
  Copyright (c) 2019-2021 Fabrizio Di Vittorio.
  All rights reserved.


* Please contact fdivitto2013@gmail.com if you need a commercial license.


* This library and related software is available under GPL v3.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "fabgl.h"

#include <WiFi.h>

#include "network/ICMP.h"




char const * AUTOEXEC = "info\r"
                        "keyb us\r"
                        "scan\r";


enum class State { Prompt,
                   PromptInput,
                   UnknownCommand,
                   Help,
                   Info,
                   Wifi,
                   TelnetInit,
                   Telnet,
                   Scan,
                   Ping,
                   Reset,
                   Keyb
                 };


State        state = State::Prompt;
WiFiClient   client;
char const * currentScript = nullptr;
bool         error = false;


fabgl::VGATextController DisplayController;
fabgl::PS2Controller     PS2Controller;
fabgl::Terminal          Terminal;
fabgl::LineEditor        LineEditor(&Terminal);


void exe_info()
{
  Terminal.write("\e[95m* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \r\n");
  Terminal.write("\e[95m* * *\e[92m                                                                     \e[95m* * *\r\n");
  Terminal.write("\e[95m* * *\e[92m              You have been INFECTED with BIOS Root Kit              \e[95m* * *\r\n");
  Terminal.write("\e[95m* * *\e[92m                                                                     \e[95m* * *\r\n");
  Terminal.write("\e[95m* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \r\n\r\n");
  Terminal.write("\e[93m All your files are belong to us!\r\n\r\n");
  Terminal.write("\e[94m You can not turn off\r\n\r\n");
  Terminal.write("\e[96m You can not reboot\r\n\r\n");
  Terminal.write("\e[91m To decrypt system you must buy one ESP Crypto at the following link \r\n\r\n");
  Terminal.write("\e[97m https://bit.ly/3J5nvlZ\r\n\r\n");

  error = false;
  state = State::Prompt;
}



void exe_help()
{
  error = false;
  state = State::Prompt;
}


void decode_command()
{
  auto inputLine = LineEditor.get();
  if (*inputLine == 0)
    state = State::Prompt;
  else if (strncmp(inputLine, "help", 4) == 0)
    state = State::Help;
  else if (strncmp(inputLine, "info", 4) == 0)
    state = State::Info;
  else if (strncmp(inputLine, "wifi", 4) == 0)
    state = State::Wifi;
  else if (strncmp(inputLine, "telnet", 6) == 0)
    state = State::TelnetInit;
  else if (strncmp(inputLine, "scan", 4) == 0)
    state = State::Scan;
  else if (strncmp(inputLine, "ping", 4) == 0)
    state = State::Ping;
  else if (strncmp(inputLine, "reboot", 6) == 0)
    state = State::Reset;
  else if (strncmp(inputLine, "keyb", 4) == 0)
    state = State::Keyb;
  else
    state = State::UnknownCommand;
}


void exe_prompt()
{
  if (currentScript) {
    // process commands from script
    if (*currentScript == 0 || error) {
      // end of script, return to prompt
      currentScript = nullptr;
      state = State::Prompt;
    } else {
      // execute current line and move to the next one
      int linelen = strchr(currentScript, '\r') - currentScript;
      LineEditor.setText(currentScript, linelen);
      currentScript += linelen + 1;
      decode_command();
    }
  } else {
    // process commands from keyboard
    Terminal.write(">");
    state = State::PromptInput;
  }
}


void exe_promptInput()
{
  LineEditor.setText("");
  LineEditor.edit();
  decode_command();
}


void exe_scan()
{
  static char const * ENC2STR[] = { "Open", "WEP", "WPA-PSK", "WPA2-PSK", "WPA/WPA2-PSK", "WPA-ENTERPRISE" };
  Terminal.write("\e[93mWe are actively exploiting the following networks:\r\n\r\n");
  Terminal.flush();
  int networksCount = WiFi.scanNetworks();
  if (networksCount) {
    Terminal.write   ("\e[90m #\e[4GSSID\e[45GRSSI\e[55GCh\e[60GEncryption\e[92m\r\n");
    for (int i = 0; i < networksCount; ++i)
      Terminal.printf("\e[93m %d\e[4G%s\e[93m\e[45G%d dBm\e[55G%d\e[60G%s\e[92m\r\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i), ENC2STR[WiFi.encryptionType(i)]);
  }
  Terminal.write("\e[97m\r\n Your defenses are useless against our attacks!!!!!!!!!!!!!!!!\r\n\r\n");
  WiFi.scanDelete();
  error = false;
  state = State::Prompt;
}


void exe_wifi()
{
  static const int MAX_SSID_SIZE = 32;
  static const int MAX_PSW_SIZE  = 32;
  char ssid[MAX_SSID_SIZE + 1];
  char psw[MAX_PSW_SIZE + 1] = {0};
  error = true;
  auto inputLine = LineEditor.get();
  if (sscanf(inputLine, "wifi %32s %32s", ssid, psw) >= 1) {
    // Terminal.write("Connecting WiFi...");
    // Terminal.flush();
    WiFi.disconnect(true, true);
    for (int i = 0; i < 2; ++i) {
      WiFi.begin(ssid, psw);
      if (WiFi.waitForConnectResult() == WL_CONNECTED)
        break;
      WiFi.disconnect(true, true);
    }
    if (WiFi.status() == WL_CONNECTED) {
      // Terminal.printf("connected to %s, IP is %s\r\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      error = false;
    } else {
      // Terminal.write("failed!\r\n");
    }
  }
  state = State::Prompt;
}


void exe_telnetInit()
{
  static const int MAX_HOST_SIZE = 32;
  char host[MAX_HOST_SIZE + 1];
  int port;
  error = true;
  auto inputLine = LineEditor.get();
  int pCount = sscanf(inputLine, "telnet %32s %d", host, &port);
  if (pCount > 0) {
    if (pCount == 1)
      port = 23;
    Terminal.printf("Trying %s...\r\n", host);
    if (client.connect(host, port)) {
      Terminal.printf("Connected to %s\r\n", host);
      error = false;
      state = State::Telnet;
    } else {
      Terminal.write("Unable to connect to remote host\r\n");
      state = State::Prompt;
    }
  } else {
    Terminal.write("Mistake\r\n");
    state = State::Prompt;
  }
}


int clientWaitForChar()
{
  // not so good...:-)
  while (!client.available())
    ;
  return client.read();
}


void exe_telnet()
{
  // process data from remote host (up to 1024 codes at the time)
  for (int i = 0; client.available() && i < 1024; ++i) {
    int c = client.read();
    if (c == 0xFF) {
      // IAC (Interpret As Command)
      uint8_t cmd = clientWaitForChar();
      uint8_t opt = clientWaitForChar();
      if (cmd == 0xFD && opt == 0x1F) {
        // DO WINDOWSIZE
        client.write("\xFF\xFB\x1F", 3); // IAC WILL WINDOWSIZE
        client.write("\xFF\xFA\x1F" "\x00\x50\x00\x19" "\xFF\xF0", 9);  // IAC SB WINDOWSIZE 0 80 0 25 IAC SE
      } else if (cmd == 0xFD && opt == 0x18) {
        // DO TERMINALTYPE
        client.write("\xFF\xFB\x18", 3); // IAC WILL TERMINALTYPE
      } else if (cmd == 0xFA && opt == 0x18) {
        // SB TERMINALTYPE
        c = clientWaitForChar();  // bypass '1'
        c = clientWaitForChar();  // bypass IAC
        c = clientWaitForChar();  // bypass SE
        client.write("\xFF\xFA\x18\x00" "wsvt25" "\xFF\xF0", 12); // IAC SB TERMINALTYPE 0 "...." IAC SE
      } else {
        uint8_t pck[3] = {0xFF, 0, opt};
        if (cmd == 0xFD)  // DO -> WONT
          pck[1] = 0xFC;
        else if (cmd == 0xFB) // WILL -> DO
          pck[1] = 0xFD;
        client.write(pck, 3);
      }
    } else {
      Terminal.write(c);
    }
  }
  // process data from terminal (keyboard)
  while (Terminal.available()) {
    client.write( Terminal.read() );
  }
  // return to prompt?
  if (!client.connected()) {
    client.stop();
    state = State::Prompt;
  }
}


void exe_ping()
{
  char host[64];
  auto inputLine = LineEditor.get();
  int pcount = sscanf(inputLine, "ping %s", host);
  if (pcount > 0) {
    int sent = 0, recv = 0;
    fabgl::ICMP icmp;
    while (true) {

      // CTRL-C ?
      if (Terminal.available() && Terminal.read() == 0x03)
        break;

      int t = icmp.ping(host);
      if (t >= 0) {
        Terminal.printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\r\n", icmp.receivedBytes(), icmp.hostIP().toString().c_str(), icmp.receivedSeq(), icmp.receivedTTL(), (double)t/1000.0);
        delay(1000);
        ++recv;
      } else if (t == -2) {
        Terminal.printf("Cannot resolve %s: Unknown host\r\n", host);
        break;
      } else {
        Terminal.printf("Request timeout for icmp_seq %d\r\n", icmp.receivedSeq());
      }
      ++sent;

    }
    if (sent > 0) {
      Terminal.printf("--- %s ping statistics ---\r\n", host);
      Terminal.printf("%d packets transmitted, %d packets received, %.1f%% packet loss\r\n", sent, recv, (double)(sent - recv) / sent * 100.0);
    }
  }
  state = State::Prompt;
}


void exe_keyb()
{
  if (PS2Controller.keyboard()->isKeyboardAvailable()) {
    char layout[3];
    auto inputLine = LineEditor.get();
    if (sscanf(inputLine, "keyb %2s", layout) == 1) {
      if (strcasecmp(layout, "US") == 0)
        Terminal.keyboard()->setLayout(&fabgl::USLayout);
      else if (strcasecmp(layout, "UK") == 0)
        Terminal.keyboard()->setLayout(&fabgl::UKLayout);
      else if (strcasecmp(layout, "DE") == 0)
        Terminal.keyboard()->setLayout(&fabgl::GermanLayout);
      else if (strcasecmp(layout, "IT") == 0)
        Terminal.keyboard()->setLayout(&fabgl::ItalianLayout);
      else if (strcasecmp(layout, "ES") == 0)
        Terminal.keyboard()->setLayout(&fabgl::SpanishLayout);
      else if (strcasecmp(layout, "FR") == 0)
        Terminal.keyboard()->setLayout(&fabgl::FrenchLayout);
      else if (strcasecmp(layout, "BE") == 0)
        Terminal.keyboard()->setLayout(&fabgl::BelgianLayout);
      else {
        Terminal.printf("Error! Invalid keyboard layout.\r\n");
        state = State::Prompt;
        return;
      }
    }
    Terminal.printf("\r\nKeyboard layout is : \e[93m%s\e[92m\r\n\r\n", Terminal.keyboard()->getLayout()->desc);
  } else {
    // Terminal.printf("No keyboard present\r\n");
  }
  state = State::Prompt;
}


void setup()
{
  //Serial.begin(115200); // DEBUG ONLY

  PS2Controller.begin(PS2Preset::KeyboardPort0);

  DisplayController.begin();
  DisplayController.setResolution();

  Terminal.begin(&DisplayController);
  Terminal.connectLocally();      // to use Terminal.read(), available(), etc..
  //Terminal.setLogStream(Serial);  // DEBUG ONLY

  Terminal.setBackgroundColor(Color::Black);
  Terminal.setForegroundColor(Color::BrightGreen);
  Terminal.clear();

  Terminal.enableCursor(true);

  currentScript = AUTOEXEC;
}


void loop()
{
  switch (state) {

    case State::Prompt:
      exe_prompt();
      break;

    case State::PromptInput:
      exe_promptInput();
      break;

    case State::Help:
      exe_help();
      break;

    case State::Info:
      exe_info();
      break;

    case State::Wifi:
      exe_wifi();
      break;

    case State::TelnetInit:
      exe_telnetInit();
      break;

    case State::Telnet:
      exe_telnet();
      break;

    case State::Scan:
      exe_scan();
      break;

    case State::Ping:
      exe_ping();
      break;

    case State::Reset:
      ESP.restart();
      break;

    case State::Keyb:
      exe_keyb();
      break;

    case State::UnknownCommand:
      Terminal.write("\r\nMistake\r\n");
      state = State::Prompt;
      break;

    default:
      Terminal.write("\r\nNot Implemeted\r\n");
      state = State::Prompt;
      break;

  }
}
