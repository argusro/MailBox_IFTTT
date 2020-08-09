MailBox notification with ESP-01 and IFTTT

This code will make a single webhook call, and then enters in deep sleep.<br/>
Every time that the ESP-01 is reset will repeat this process.<br/>
Electrical connections are simple, supply the ESP with 3.3V and a switch between<br/>
reset pin and ground.<br/>
OTA (Over The Air) firmware upgrade process was also implemented, just place<br/>
the new bin file in a web server and a file including the firmware version.<br/>
If differ from the one stored on the local virtual EEPROM the new firmware<br/>
will be pulled and installed.<br/>
Use ESP-01 with memory > 1MB<br/>

This program is free software; you can redistribute it and/or<br/>
modify it under the terms of the GNU General Public License<br/>
version 2 as published by the Free Software Foundation.<br/>

Firmware: 1.0 - 08/2020<br/>
Autor: ArgusR <@argusro>
