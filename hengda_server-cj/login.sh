#!/usr/bin/expect
spawn /usr/bin/ssh admin@192.168.2.111 -p 22
expect "admin@192.168.2.111's password:"
send "Hengda123456\r"

expect "IES:/->"
send "develop\r"

expect "Password"
send "Huawei@SYS3\r"

expect efo
interact
