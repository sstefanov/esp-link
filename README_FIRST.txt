* Python 2.7 is mandatory! ESP chain is based on it.

* If receive error:  Embedded: not found
set PATH to not include spaces, for example:
old PATH=/media/hdd2/sdb7/src/sstefanov/esp-open-sdk/xtensa-lx106-elf/bin/:/media/hdd2/sdb7/opt/google-cloud-sdk/bin:/home/sstefanov/.pyenv/bin:/usr/local/bin/../bin/../bin:/usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games:/usr/lib/jvm/jdk-18/bin:/home/sstefanov/bin:/opt/MounRiver_Studio_Community_Linux_x64_V140/MRS_Community/toolchain/RISC-V Embedded GCC/bin/
new:
export PATH=/media/hdd2/sdb7/src/sstefanov/esp-open-sdk/xtensa-lx106-elf/bin/:/media/hdd2/sdb7/opt/google-cloud-sdk/bin:/home/sstefanov/.pyenv/bin:/usr/local/bin/../bin/../bin:/usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games:/usr/lib/jvm/jdk-18/bin:/home/sstefanov/bin


* Flash with ./flash.sh, not use make flash

* Flash original esp-link from release:
esptool --port $PORT --baud 460800 write_flash -fs 4MB -ff 80m 0x00000 ../esp_iot_sdk_v2.1.0/bin/boot_v1.7.bin 0x1000 esp-link-v3.0.14-g963ffbb/user1.bin   0x3FC000 ../esp_iot_sdk_v2.1.0/bin//esp_init_data_default.bin 0x3FE000 ../esp_iot_sdk_v2.1.0/bin/blank.bin
- easier way is:
make wiflash

Not needed:
* Python must be set as 2.7 in first line of esptool:
#!/usr/bin/env python2.7

* DEBUG setup:

- connect second serial adapter to uart1 (gpio2)
- set debug log to uart1
- use print on debug port:
char prbuff[256];
int prlen;
prlen = os_sprintf(prbuff, "Hello, world! %s", integervalue );
os_printf(prbuff,prlen);
