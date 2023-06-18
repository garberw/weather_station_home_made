#!/usr/bin/env python


import RPi.GPIO as GPIO
import subprocess

# SHUTDOWN_PIN = 3
SHUTDOWN_PIN = 12   # bcm

print("garberw ==== note two warnings are harmless")
print("garberw ==== warning channel already in use (it always says this)")
print("garberw ==== physical pull up resistor already fitted on this channel (SCL does have one)")
GPIO.setmode(GPIO.BCM)
GPIO.setup(SHUTDOWN_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.wait_for_edge(SHUTDOWN_PIN, GPIO.FALLING)

subprocess.call(['shutdown', '-h', 'now'], shell=False)
