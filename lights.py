import RPi.GPIO as GPIO
import time

# GPIO setup
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)

# Define IR sensor pins for each road
road_sensors = {
    "Road 1": [18, 27, 22, 17],
    "Road 2": [4, 23,24,25],
    "Road 3": [12, 5, 6, 13],
    "Road 4": [19, 26, 21, 20]
}

# Setup GPIO pins as input
for sensors in road_sensors.values():
    for pin in sensors:
        GPIO.setup(pin, GPIO.IN)

def get_density(sensors):
    """Calculate density based on the number of detected objects."""
    count = sum(GPIO.input(pin) == 0 for pin in sensors)  # 0 means object detected
    return count

def display_density():
    """Displays the density of each road using hashtags in the terminal."""
    print("\nTraffic Density Monitor")
    print("========================")
    
    for road, sensors in road_sensors.items():
        density = get_density(sensors)
        density_bar = "#" * density
        print(f"{road}: [{density_bar:<4}] ({density}/4)")
    
    print("========================\n")

try:
    while True:
        display_density()
        time.sleep(1)  # Update every second

except KeyboardInterrupt:
    print("\nStopping traffic density monitoring...")

finally:
    GPIO.cleanup()
