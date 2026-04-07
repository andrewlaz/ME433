import board
import pwmio
import time

servo = pwmio.PWMOut(board.GP16, frequency=50, variable_frequency=False)

def set_servo(angle):
    duty = (0.05 + (angle/180.0) * 0.05) * 65535
    servo.duty_cycle = int(duty)

while True:
    for angle in range(10, 170):
        set_servo(angle)
        time.sleep(0.01)

    for angle in range(170, 10, -1):
        set_servo(angle)
        time.sleep(0.01)