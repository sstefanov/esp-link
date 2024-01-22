# OTA Flash (from wiflash file)
# download curl for windows
# check build number
# get next firmware name (user1.bin in example)
curl -m 10 -s http://%HOSTNAME%/flash/next
# upload next firmware
curl -XPOST --data-binary @firmware/user1.bin http://%HOSTNAME%/flash/upload
# reboot
curl -m 10 -s http://%HOSTNAME%/flash/reboot
# wait 5 s
# and get next firmware, must be different (user2.bin in example)
curl -m 10 -s http://%HOSTNAME%/flash/next
# check new build number