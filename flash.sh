PORT=/dev/ttyUSB0
esptool --port $PORT --baud 460800 write_flash -fs 4MB -ff 80m 0x00000 ../esp_iot_sdk_v2.1.0/bin/boot_v1.7.bin 0x1000 firmware/user1.bin     0x3FC000 ../esp_iot_sdk_v2.1.0/bin//esp_init_data_default.bin 0x3FE000 ../esp_iot_sdk_v2.1.0/bin/blank.bin

stty -F $PORT cs8 -cstopb -parenb 115200