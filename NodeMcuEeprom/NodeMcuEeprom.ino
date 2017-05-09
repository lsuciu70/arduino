#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#define SECONDS_AS_PICO (1000000)
#define SECONDS_AS_MILLIS (1000)

const uint32_t five_minutes = 5 * 60 * SECONDS_AS_MILLIS;

const uint16_t EEPROM_START =  512;
const uint16_t EEPROM_SIZE  = 2048;
const uint8_t BUFF_SIZE     =   16;

const uint8_t EEPROM_UNSET = 0xFF;

/* Memory mapping */
/*
op_mode  ->   0 (EEPROM_START)
0        ->   1
guard    ->   2
0        ->   3
u_name   ->   4
0        ->  20 (  4 + 16)
u_pwd    ->  21
0        ->  37 ( 21 + 16)
ap1_name ->  38
0        ->  54 ( 38 + 16)
ap1_pwd  ->  55
0        ->  71 ( 55 + 16)
ap2_name ->  72
0        ->  88 ( 72 + 16)
ap2_pwd  ->  89
0        -> 105 ( 89 + 16)
ap0_name -> 106               // host_name
0        -> 122 (106 + 16)    // 0
ap0_pwd  -> 123               // -
0        -> 139 (123 + 16)    // -
*/
const uint8_t ssid_len = 16;
const uint8_t usr_pwd_len = 16;
const uint8_t host_len = 8;

enum : uint8_t
{
  MODE_AP = 1,
  MODE_CLI = 2,
  MODE_UNSET = EEPROM_UNSET
};

uint8_t op_mode;

uint8_t guard;

const char* ap_ssid = "ap-ir";
const char* ap_pwd = "admin_1234";

const char* user_pwd = "admin";

char ap0_name[ssid_len + 1];
char ap0_pwd[usr_pwd_len + 1];

char host_name[host_len + 1];

char u_name[usr_pwd_len + 1];
char u_pwd[usr_pwd_len + 1];

char ap1_name[ssid_len + 1];
char ap1_pwd[usr_pwd_len + 1];
char ap2_name[ssid_len + 1];
char ap2_pwd[usr_pwd_len + 1];

ESP8266WebServer server(80);

// Romanian HTML Decimal Code
// Ă: &#258;
// ă: &#259;
// Â: &#194;
// â: &#226;
// Î: &#206;
// î: &#238;
// Ș: &#x218;
// ș: &#x219;
// Ț: &#538;
// ț: &#539;

String content_root =
"<html>"
"<head><title>Iriga&#539;ie</title></head>"
"<body>"
"<h1>Sistemul de iriga&#539;ie</h1>"
"<h2>Ac&#539;iuni:</h2>"
"<table>"
"<tr><td><form name='programming' method='POST' action='programming'><input type='submit' value='Programare' style='width:100%'></form></td></tr>"
"<tr><td><form name='reset' method='POST' action='reset'><input type='submit' value='Resetare' style='width:100%'></form></td></tr>"
"</table>"
"</body>"
"</html>";

String content_settings =
"<html>"
"<head><title>Iriga&#539;ie</title></head>"
"<body>"
"<h1>Sistemul de iriga&#539;ie</h1>"
"<h2>Setare mod lucru:</h2>"
"<table>"
"<tr><td><form name='ap_mode' method='POST' action='ap_mode'><input type='submit' value='Punct Acces WiFi' style='width:100%'></form></td></tr>"
"<tr><td><form name='cli_mode' method='POST' action='cli_mode'><input type='submit' value='Client WiFi' style='width:100%'></form></td></tr>"
"</table>"
"</body>"
"</html>";

const char* content_set_ap_mode_fmt =
"<html>"
"<head>"
"<title>Iriga&#539;ie</title>"
"<style>"
"table {border-collapse: collapse;border-top: 2px solid black;border-bottom: 2px solid black;}"
"th, td {padding: 8px;border-bottom: 1px solid black;}"
"</style>"
"</head>"
"<body>"
"<h1>Sistemul de iriga&#539;ie</h1>"
"<h2>Setare mod lucru 'Punct Acces WiFi'</h2>"
"<table>"
"<form name='save_ap_mode' method='POST' action='save_ap_mode'>"
"<tr><td colspan=2><b>Setare sistem:</b></td></tr>"
"<tr><td>Nume AP:</td><td><input type='text' name='ap0_name' size='6' maxlength='6' value='ap-ir'></td></tr>"
"<tr><td>Parola AP:</td><td><input type='text' name='ap0_pwd_1' size='16' maxlength='16'></td></tr>"
"<tr><td>Confirmare parola:</td><td><input type='text' name='ap0_pwd_2' size='16' maxlength='16'></td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td>Nume utilizator:</td><td><input type='text' name='u_name' size='16' maxlength='16'></td></tr>"
"<tr><td>Parola utilizator:</td><td><input type='text' name='u_pwd_1' size='16' maxlength='16'></td></tr>"
"<tr><td>Confirmare parola:</td><td><input type='text' name='u_pwd_2' size='16' maxlength='16'></td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td colspan=2><b>Setare conexiune Internet:</b></td></tr>"
"<tr><td>Nume AP principal:</td><td><input type='text' name='ap1_name' size='16' maxlength='16' value='%s'></td></tr>"
"<tr><td>Parola AP principal:</td><td><input type='text' name='ap1_pwd' size='16' maxlength='16'></td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td>Nume AP secundar:</td><td><input type='text' name='ap2_name' size='16' maxlength='16'></td></tr>"
"<tr><td>Parola AP secundar:</td><td><input type='text' name='ap2_pwd' size='16' maxlength='16'></td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td colspan=2><b>Puncte acces WiFi g&#259;site:</b></td></tr>"
"<tr><td><input type='text' size='16' maxlength='16' value='%s' disabled='disabled'></td><td><input type='text' size='5' maxlength='5' value='%d' disabled='disabled'> [dBm]</td></tr>"
"<tr><td><input type='text' size='16' maxlength='16' value='%s' disabled='disabled'></td><td><input type='text' size='5' maxlength='5' value='%d' disabled='disabled'> [dBm]</td></tr>"
"<tr><td><input type='text' size='16' maxlength='16' value='%s' disabled='disabled'></td><td><input type='text' size='5' maxlength='5' value='%d' disabled='disabled'> [dBm]</td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td colspan=2 align='center'><input type='submit' value='Salvare'></td></tr>"
"</form>"
"<form name='cancel_ap_mode' method='GET' action='.'>"
"<tr><td colspan=2 align='center'><input type='submit' value='&#206;napoi'></td></tr>"
"</form>"
"</table>"
"</body>"
"</html>";

const char* content_set_cli_mode_fmt =
"<html>"
"<head>"
"<title>Iriga&#539;ie</title>"
"<style>"
"table {border-collapse: collapse;border-top: 2px solid black;border-bottom: 2px solid black;}"
"th, td {padding: 8px;border-bottom: 1px solid black;}"
"</style>"
"</head>"
"<body>"
"<h1>Sistemul de iriga&#539;ie</h1>"
"<h2>Setare mod lucru 'WiFi Client'</h2>"
"<table>"
"<form name='save_cli_mode' method='POST' action='save_cli_mode'>"
"<tr><td colspan=2><b>Setare sistem:</b></td></tr>"
"<tr><td>Nume sistem:</td><td><input type='text' name='host_name' size='8' maxlength='8' value='irigatie'></td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td>Nume utilizator:</td><td><input type='text' name='u_name' size='16' maxlength='16'></td></tr>"
"<tr><td>Parola utilizator:</td><td><input type='text' name='u_pwd_1' size='16' maxlength='16'></td></tr>"
"<tr><td>Confirmare parola:</td><td><input type='text' name='u_pwd_2' size='16' maxlength='16'></td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td colspan=2><b>Setare conexiune Internet:</b></td></tr>"
"<tr><td>Nume AP principal:</td><td><input type='text' name='ap1_name' size='16' maxlength='16' value='%s'></td></tr>"
"<tr><td>Parola AP principal:</td><td><input type='text' name='ap1_pwd' size='16' maxlength='16'></td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td>Nume AP secundar:</td><td><input type='text' name='ap2_name' size='16' maxlength='16'></td></tr>"
"<tr><td>Parola AP secundar:</td><td><input type='text' name='ap2_pwd' size='16' maxlength='16'></td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td colspan=2><b>Puncte acces WiFi g&#259;site:</b></td></tr>"
"<tr><td><input type='text' size='16' maxlength='16' value='%s' disabled='disabled'></td><td><input type='text' size='5' maxlength='5' value='%d' disabled='disabled'> [dBm]</td></tr>"
"<tr><td><input type='text' size='16' maxlength='16' value='%s' disabled='disabled'></td><td><input type='text' size='5' maxlength='5' value='%d' disabled='disabled'> [dBm]</td></tr>"
"<tr><td><input type='text' size='16' maxlength='16' value='%s' disabled='disabled'></td><td><input type='text' size='5' maxlength='5' value='%d' disabled='disabled'> [dBm]</td></tr>"
"<tr><td colspan=2></td></tr>"
"<tr><td colspan=2 align='center'><input type='submit' value='Salvare'></td></tr>"
"</form>"
"<form name='cancel_cli_mode' method='GET' action='.'>"
"<tr><td colspan=2></td></tr>"
"<tr><td colspan=2 align='center'><input type='submit' value='&#206;napoi'></td></tr>"
"</form>"
"</table>"
"</body>"
"</html>";

void handleRoot()
{
  if(guard)
  {
    // reset guard
    guard = 0;
    EEPROM.write(EEPROM_START + 2, guard);
    EEPROM.commit();
    Serial.println("Guard was reset.");
  }
  switch (op_mode)
  {
    case MODE_UNSET:
      server.send(200, "text/html", content_settings);
      break;
    default:
      if(!server.authenticate(u_name, u_pwd))
        server.requestAuthentication();
      else
        server.send(200, "text/html", content_root);
      break;
  }
}

void handleSetupApMode()
{
  if(!server.authenticate(u_name, u_pwd))
    server.requestAuthentication();
  else
  {
    int8_t nets = WiFi.scanNetworks();
    // 0) cls-router  Signal: -52 dBm Channel: 5  Encryption: Auto
    // 1) cristibcd  Signal: -85 dBm Channel: 6  Encryption: WPA
    // 2) cls-ap Signal: -80 dBm Channel: 11 Encryption: WPA2
    int8_t net1_pwr = -120, net2_pwr = -120, net3_pwr = -120;
    char net1_ssid[ssid_len + 1] = {'\0'}, net2_ssid[ssid_len + 1] = {'\0'}, net3_ssid[ssid_len + 1] = {'\0'};
    // sort them
    for (uint8_t net = 0; net < nets; ++net)
    {
      int8_t pwr = WiFi.RSSI(net);
      if(pwr > net1_pwr)
      {
        net3_pwr = net2_pwr;
        strcpy(net3_ssid, net2_ssid);
        net2_pwr = net1_pwr;
        strcpy(net2_ssid, net1_ssid);
        net1_pwr = pwr;
        strcpy(net1_ssid, WiFi.SSID(net).c_str());
      }
      else if(pwr > net2_pwr)
      {
        net3_pwr = net2_pwr;
        strcpy(net3_ssid, net2_ssid);
        net2_pwr = pwr;
        strcpy(net2_ssid, WiFi.SSID(net).c_str());
      }
      else if(pwr > net2_pwr)
      {
        net3_pwr = pwr;
        strcpy(net3_ssid, WiFi.SSID(net).c_str());
      }
    }
    char content_set_ap_mode[strlen(content_set_ap_mode_fmt) + (4 * 14) + (3 * 2) + 1];
    sprintf(content_set_ap_mode, content_set_ap_mode_fmt, net1_ssid, net1_ssid, net1_pwr, net2_ssid, net2_pwr, net3_ssid, net3_pwr);
  	server.send(200, "text/html", content_set_ap_mode);
  }
}

void handleSetupCliMode()
{
  if(!server.authenticate(u_name, u_pwd))
    server.requestAuthentication();
  else
  {
    int8_t nets = WiFi.scanNetworks();
    // 0) cls-router  Signal: -52 dBm Channel: 5  Encryption: Auto
    // 1) cristibcd  Signal: -85 dBm Channel: 6  Encryption: WPA
    // 2) cls-ap Signal: -80 dBm Channel: 11 Encryption: WPA2
    int8_t net1_pwr = -120, net2_pwr = -120, net3_pwr = -120;
    char net1_ssid[ssid_len + 1] = {'\0'}, net2_ssid[ssid_len + 1] = {'\0'}, net3_ssid[ssid_len + 1] = {'\0'};
    // sort them
    for (uint8_t net = 0; net < nets; ++net)
    {
      int8_t pwr = WiFi.RSSI(net);
      if(pwr > net1_pwr)
      {
        net3_pwr = net2_pwr;
        strcpy(net3_ssid, net2_ssid);
        net2_pwr = net1_pwr;
        strcpy(net2_ssid, net1_ssid);
        net1_pwr = pwr;
        strcpy(net1_ssid, WiFi.SSID(net).c_str());
      }
      else if(pwr > net2_pwr)
      {
        net3_pwr = net2_pwr;
        strcpy(net3_ssid, net2_ssid);
        net2_pwr = pwr;
        strcpy(net2_ssid, WiFi.SSID(net).c_str());
      }
      else if(pwr > net2_pwr)
      {
        net3_pwr = pwr;
        strcpy(net3_ssid, WiFi.SSID(net).c_str());
      }
    }
    char content_set_cli_mode[strlen(content_set_cli_mode_fmt) + (4 * 14) + (3 * 2) + 1];
    sprintf(content_set_cli_mode, content_set_cli_mode_fmt, net1_ssid, net1_ssid, net1_pwr, net2_ssid, net2_pwr, net3_ssid, net3_pwr);
  	server.send(200, "text/html", content_set_cli_mode);
  }
}

void eepromReset(bool doCommit = true)
{
  for(uint16_t addr = EEPROM_START; addr < EEPROM_SIZE; ++addr)
  {
    if(EEPROM.read(addr) == EEPROM_UNSET)
      break;
    EEPROM.write(addr, EEPROM_UNSET);
  }
  if(doCommit)
    EEPROM.commit();
}

uint16_t eepromWrite(uint16_t addr, const char* value)
{
  uint8_t i = 0;
  char c;
  while((c = *(value + (i++))))
    EEPROM.write(addr++, c);
  EEPROM.write(addr++, 0);
  return addr;
}

uint16_t eepromRead(uint16_t addr, char* value)
{
  uint8_t i = 0;
  char c;
  while((c = EEPROM.read(addr++)))
    *(value + (i++)) = c;
  *(value + (i++)) = c;
  return addr;
}

const char* content_save_ap_mode_response_fmt =
"<html>"
"<head><title>Iriga&#539;ie</title></head>"
"<body>"
"<h1>Sistemul de iriga&#539;ie</h1>"
"<p><b>"
"Salvarea set&#259;rilor pentul modul 'Punct Acces WiFi' s-a f&#259;cut cu succes; sistemul va reporni cu noile set&#259;ri."
"</b></p>"
"<p><b>"
"Va trebui s&#259; v&#259; reconecta&#539;i folosind noile valori pentru punctul acces WiFi:"
"</b></p>"
"<p><b>"
"Nume AP:   %s<br>"
"Parola AP: %s"
"</b></p>"
"<p><b>"
"Pentru sistem folosi&#539;i:"
"</b></p>"
"<p><b>"
"Utilizator: %s<br>"
"Parola:     %s"
"</b></p>"
"<p style='color:red'><b>"
"!!! Asigura&#539;i-v&#259; ca le-a&#539;i notat &#238;ntr-un loc sigur; f&#259;r&#259; ele nu v&#259; ve&#539;i mai putea conecta la sistem !!!"
"</b></p>"
"<p style='color:red'><b>"
"!!! Dac&#259; nu v&#259; conecta&#539;i &#238;n urm&#259;toarele 5 minute sistemul va reveni la set&#259;rile ini&#539;iale. !!!"
"</b></p>"
"<p>"
"Dup&#259; reconectare deschide&#539;i pagina principal&#259; butonul de mai jos."
"</p>"
"<p>"
"<form name='save_ap_response' method='GET' action='.'>"
"<input type='submit' value='Pagina principal&#259;'>"
"</form>"
"</p>"
"</body>"
"</html>";

void handleSaveApMode()
{
  int8_t argCnt = server.args();
  Serial.print("argCnt = ");
  Serial.print(argCnt);
  Serial.println();
  for(int8_t i = 0; i < argCnt; ++i)
  {
    Serial.print(server.argName(i));
    Serial.print(" = ");
    Serial.println(server.arg(i));
  }

  // fetch the input
  strcpy(ap0_pwd, server.arg("ap0_pwd_1").c_str());
  char ap0_pwd_2[usr_pwd_len + 1];
  strcpy(ap0_pwd_2, server.arg("ap0_pwd_2").c_str());
  if((strlen(ap0_pwd) > 0 || strlen(ap0_pwd_2) > 0) && strcmp(ap0_pwd, ap0_pwd_2) != 0)
  {
    server.sendHeader("refresh", "5;url=/ap_mode");
    server.send(400, "text/html", "<h1>Eroare setare Acces Point:</h1><br><h2>Parola si confirmarea parolei nu sunt identice!</h2>");
    return;
  }

  strcpy(u_pwd, server.arg("u_pwd_1").c_str());
  char u_pwd_2[usr_pwd_len + 1];
  strcpy(u_pwd_2, server.arg("u_pwd_2").c_str());
  if((strlen(u_pwd) > 0 || strlen(u_pwd_2) > 0) && strcmp(u_pwd, u_pwd_2) != 0)
  {
    server.sendHeader("refresh", "5;url=/ap_mode");
    server.send(400, "text/html", "<h1>Eroare setare utilizator:</h1><br><h2>Parola si confirmarea parolei nu sunt identice!</h2>");
    return;
  }

  strncpy(ap0_name, server.arg("ap0_name").c_str(), ssid_len);
  ap0_name[ssid_len] = '\0';
  strncpy(u_name, server.arg("u_name").c_str(), usr_pwd_len);
  u_name[usr_pwd_len] = '\0';
  strncpy(ap1_name, server.arg("ap1_name").c_str(), ssid_len);
  ap1_name[ssid_len] = '\0';
  strncpy(ap1_pwd, server.arg("ap1_pwd").c_str(), usr_pwd_len);
  ap1_pwd[usr_pwd_len] = '\0';
  strncpy(ap2_name, server.arg("ap2_name").c_str(), ssid_len);
  ap2_name[ssid_len] = '\0';
  strncpy(ap2_pwd, server.arg("ap2_pwd").c_str(), usr_pwd_len);
  ap2_pwd[usr_pwd_len] = '\0';

  if(op_mode != MODE_AP)
  {
    eepromReset(false);
  }
  uint16_t addr = EEPROM_START;
  op_mode = MODE_AP;
  guard = 1;
  EEPROM.write(addr++, op_mode);
  EEPROM.write(addr++, 0);
  EEPROM.write(addr++, guard);
  EEPROM.write(addr++, 0);

  addr = eepromWrite(addr, u_name);
  addr = eepromWrite(addr, u_pwd);
  addr = eepromWrite(addr, ap1_name);
  addr = eepromWrite(addr, ap1_pwd);
  addr = eepromWrite(addr, ap2_name);
  addr = eepromWrite(addr, ap2_pwd);
  addr = eepromWrite(addr, ap0_name);
  addr = eepromWrite(addr, ap0_pwd);
  EEPROM.commit();

  // print it
  // didn't saved pwd confirmations (two of them)
  argCnt -= 2;
  // print mode
  addr = EEPROM_START;
  Serial.println("handleSaveApMode - new EEPROM values:");
  Serial.print("Start address: ");
  Serial.print(addr, HEX);
  Serial.print("; value: ");
  Serial.println((int) EEPROM.read(addr++));
  Serial.print("Start address: ");
  Serial.print(addr, HEX);
  Serial.print("; value: ");
  Serial.println((int) EEPROM.read(addr++));
  addr += 1;
  char value[ssid_len];
  for(int8_t i = 0; i < argCnt; ++i)
  {
  	Serial.print("Start address: ");
  	Serial.print(addr, HEX);
  	Serial.print("; value: ");
    addr = eepromRead(addr, value);
    Serial.println(value);
  }

  char content_save_ap_mode_response[strlen(content_save_ap_mode_response_fmt) + ssid_len + usr_pwd_len - 3];
  sprintf(content_save_ap_mode_response, content_save_ap_mode_response_fmt, ap0_name, ap0_pwd, u_name, u_pwd);
  server.send(200, "text/html", content_save_ap_mode_response);
  ESP.deepSleep(SECONDS_AS_PICO);
}

void handleSaveCliMode()
{
  int8_t argCnt = server.args();
  Serial.print("argCnt = ");
  Serial.print(argCnt);
  Serial.println();
  for(int8_t i = 0; i < argCnt; ++i)
  {
    Serial.print(server.argName(i));
    Serial.print(" = ");
    Serial.println(server.arg(i));
  }

  // fetch the input
  strcpy(u_pwd, server.arg("u_pwd_1").c_str());
  char u_pwd_2[usr_pwd_len + 1];
  strcpy(u_pwd_2, server.arg("u_pwd_2").c_str());
  if((strlen(u_pwd) > 0 || strlen(u_pwd_2) > 0) && strcmp(u_pwd, u_pwd_2) != 0)
  {
    server.sendHeader("refresh", "5;url=/cli_mode");
    server.send(400, "text/html", "<h1>Eroare setare utilizator:</h1><br><h2>Parola si confirmarea parolei nu sunt identice!</h2>");
    return;
  }

  strncpy(host_name, server.arg("host_name").c_str(), ssid_len);
  host_name[ssid_len] = '\0';
  strncpy(u_name, server.arg("u_name").c_str(), usr_pwd_len);
  u_name[usr_pwd_len] = '\0';
  strncpy(ap1_name, server.arg("ap1_name").c_str(), ssid_len);
  ap1_name[ssid_len] = '\0';
  strncpy(ap1_pwd, server.arg("ap1_pwd").c_str(), usr_pwd_len);
  ap1_pwd[usr_pwd_len] = '\0';
  strncpy(ap2_name, server.arg("ap2_name").c_str(), ssid_len);
  ap2_name[ssid_len] = '\0';
  strncpy(ap2_pwd, server.arg("ap2_pwd").c_str(), usr_pwd_len);
  ap2_pwd[usr_pwd_len] = '\0';

  if(op_mode != MODE_CLI)
  {
    eepromReset(false);
  }
  uint16_t addr = EEPROM_START;
  op_mode = MODE_CLI;
  guard = 1;
  EEPROM.write(addr++, op_mode);
  EEPROM.write(addr++, 0);
  EEPROM.write(addr++, guard);
  EEPROM.write(addr++, 0);

  addr = eepromWrite(addr, u_name);
  addr = eepromWrite(addr, u_pwd);
  addr = eepromWrite(addr, ap1_name);
  addr = eepromWrite(addr, ap1_pwd);
  addr = eepromWrite(addr, ap2_name);
  addr = eepromWrite(addr, ap2_pwd);
  addr = eepromWrite(addr, host_name);
  EEPROM.commit();

  // print it
  // didn't saved pwd confirmation
  argCnt -= 1;
  // print mode
  addr = EEPROM_START;
  Serial.println("handleSaveCliMode - new EEPROM values:");
  Serial.print("Start address: ");
  Serial.print(addr, HEX);
  Serial.print("; value: ");
  Serial.println((int) EEPROM.read(addr++));
  Serial.print("Start address: ");
  Serial.print(addr, HEX);
  Serial.print("; value: ");
  Serial.println((int) EEPROM.read(addr++));
  addr += 1;
  char value[ssid_len];
  for(int8_t i = 0; i < argCnt; ++i)
  {
    Serial.print("Start address: ");
    Serial.print(addr, HEX);
    Serial.print("; value: ");
    addr = eepromRead(addr, value);
    Serial.println(value);
  }


/*
host_name = irigatie
u_name = u
u_pwd_1 = up
u_pwd_2 = up
ap1_name = cls-router
ap1_pwd = aa
ap2_name = cls-ap
ap2_pwd = bb
*/
  server.send(200, "text/html", "<h1>handleSaveCliMode done</h1>");
}

String content_reset_response =
"<html>"
"<head><title>Iriga&#539;ie</title></head>"
"<body>"
"<h1>Sistemul de iriga&#539;ie</h1>"
"<p><b>"
"Resetarea s-a f&#259;cut cu succes; sistemul va reporni cu set&#259;rile implicite."
"</b></p>"
"<p><b>"
"Pentru conectare folosi&#539;i:"
"</b></p>"
"<p><b>"
"Nume AP:   ap-ir<br>"
"Parola AP: admin_1234"
"</b></p>"
"<p><b>"
"Pentru sistem folosi&#539;i:"
"</b></p>"
"<p><b>"
"Utilizator: admin<br>"
"Parola:     admin"
"</b></p>"
"<p>"
"Dup&#259; reconectare deschide&#539;i pagina principal&#259; butonul de mai jos."
"</p>"
"<p>"
"<form name='reset_response' method='GET' action='.'>"
"<input type='submit' value='Pagina principal&#259;'>"
"</form>"
"</p>"
"</body>"
"</html>";

void handleReset()
{
  eepromReset();

  server.send(200, "text/html", content_reset_response);
  ESP.deepSleep(SECONDS_AS_PICO);
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  uint16_t addr = EEPROM_START;
  op_mode = EEPROM.read(addr++);
  guard = EEPROM.read(addr++);
  if(guard == EEPROM_UNSET)
    guard = 0;
  if(op_mode == MODE_CLI)
  {
    // read EEPROM
    addr += 2;
    addr = eepromRead(addr, u_name);
    addr = eepromRead(addr, u_pwd);
    addr = eepromRead(addr, ap1_name);
    addr = eepromRead(addr, ap1_pwd);
    addr = eepromRead(addr, ap2_name);
    addr = eepromRead(addr, ap2_pwd);
    addr = eepromRead(addr, host_name);
    if(!WiFi.mode(WIFI_AP_STA))
    {
      Serial.println("Error setting WiFi mode!");
      return;
    }
    if(!WiFi.hostname(host_name))
    {
      Serial.println("Error setting hostname!");
      return;
    }
    WiFi.begin(ap1_name, ap1_pwd);
  }
  else
  {
    if(op_mode == MODE_UNSET)
    {
      strcpy(u_name, user_pwd);
      strcpy(u_pwd, user_pwd);
      strcpy(ap0_name, ap_ssid);
      strcpy(ap0_pwd, ap_pwd);
    }
    else if(op_mode == MODE_AP)
    {
      // read EEPROM
      addr += 2;
      addr = eepromRead(addr, u_name);
      addr = eepromRead(addr, u_pwd);
      addr = eepromRead(addr, ap1_name);
      addr = eepromRead(addr, ap1_pwd);
      addr = eepromRead(addr, ap2_name);
      addr = eepromRead(addr, ap2_pwd);
      addr = eepromRead(addr, ap0_name);
      addr = eepromRead(addr, ap0_pwd);
    }
    else
    {
      Serial.print("\nError, unknown operation mode: ");
      Serial.println(op_mode);
      return;
    }
  	if(!WiFi.mode(WIFI_AP_STA))
  	{
  	  Serial.println("Error setting WiFi mode!");
  	  return;
  	}
  	if(!WiFi.softAPConfig(IPAddress(10, 10, 10, 10), IPAddress(127, 0, 0, 1), IPAddress(255, 0, 0, 0)))
  	{
  	  Serial.println("Error setting AP IPs!");
  	  return;
  	}
  	if(!WiFi.softAP(ap0_name, ap0_pwd))
  	{
  	  Serial.println("Error starting AP!");
  	  return;
  	}
    Serial.print("\nAP started, local IP: ");
    Serial.println(WiFi.softAPIP());
  }
  server.on("/", handleRoot);
  server.on("/ap_mode", handleSetupApMode);
  server.on("/cli_mode", handleSetupCliMode);
  server.on("/save_ap_mode", handleSaveApMode);
  server.on("/save_cli_mode", handleSaveCliMode);
  server.on("/reset", handleReset);
  server.begin();
  Serial.println("Web server started");
}

void loop()
{
  if(guard && (millis() > five_minutes))
  {
    Serial.println("Guard reset!");
    eepromReset();
    ESP.deepSleep(SECONDS_AS_PICO);
  }
  server.handleClient();
}
