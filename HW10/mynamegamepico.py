"""
Crossy Road 2D - Slalom Edition
Run with:  pgzrun crossy_road.py

Controls:
  Left / Right Arrow - move frog left / right (hold to slide)
  Up Arrow / W       - hop forward
  Down Arrow / S     - hop backward
  Space              - restart
"""

import pgzrun
import random
import serial
ser = serial.Serial('/dev/tty.usbmodem14401', 115200)

WIDTH = 600
HEIGHT = 800

TITLE = "Mini Crossy Road"

# -----------------------------
# Player
# -----------------------------
GRID = 50

player = Rect((275, 700), (GRID, GRID))

MOVE_DISTANCE = GRID
# -----------------------------
# Game state
# -----------------------------
score = 0
game_over = False


# -----------------------------
# Car settings
# -----------------------------
cars = []

LANES = [150, 250, 350, 450, 550, 650]

CAR_SPEEDS = [4, 5, 6, 7]

speed_multiplier = 1.0
frame_count = 0

# -----------------------------
# Sounds
# -----------------------------
# Add your own .wav or .ogg files later:
#
# sounds/jump.wav
# sounds/hit.wav
#
# Then uncomment the sound lines below.
# -----------------------------


def create_car():
    lane = random.choice(LANES)

    # Random direction
    direction = random.choice([-1, 1])

    if direction == 1:
        x = -100
    else:
        x = WIDTH + 100

    car = {
        "rect": Rect((x, lane), (100, GRID)),
        "speed": random.choice(CAR_SPEEDS) * direction,
        "color": random.choice([
            (255, 0, 0),
            (0, 200, 255),
            (255, 255, 0),
            (255, 100, 0),
            (200, 0, 255)
        ])
    }

    cars.append(car)


# Create initial cars
for _ in range(10):
    create_car()


def reset_game():
    global score, game_over, cars
    global speed_multiplier
    global frame_count

    speed_multiplier = 1.0
    frame_count = 0

    score = 0
    game_over = False

    player.x = 275
    player.y = 700

    cars = []

    for _ in range(10):
        create_car()


def update():
    global game_over
    global frame_count
    global speed_multiplier

    frame_count += 1

    # Increase difficulty every few seconds
    if frame_count % 300 == 0:
        speed_multiplier += 0.1

    if ser.in_waiting:

        command = ser.readline().decode().strip()

        handle_input(command)

    if game_over:
        return

    # Move cars
    for car in cars:
        car["rect"].x += car["speed"] * speed_multiplier

        # Wrap around screen
        if car["speed"] > 0 and car["rect"].x > WIDTH + 120:
            car["rect"].x = -120

        if car["speed"] < 0 and car["rect"].x < -120:
            car["rect"].x = WIDTH + 120

        # Collision
        if player.colliderect(car["rect"]):
            game_over = True

            # sounds.hit.play()

def handle_input(command):
    global score

    if command == "S":
        reset_game()
        return

    if game_over:
        return

    if command == "U":

        player.y -= MOVE_DISTANCE

        player.y = max(0, player.y)

        score += 1

    elif command == "D":

        player.y += MOVE_DISTANCE

        player.y = min(HEIGHT - player.height, player.y)

    # Reached top
    if player.y <= 0:

        score += 10

        player.x = 275
        player.y = 725

def on_key_down(key):
    global score

    if key == keys.SPACE:
        reset_game()
        return

    if game_over:
        return

    moved = False

    if key == keys.UP:
        player.y -= MOVE_DISTANCE
        moved = True

    elif key == keys.DOWN:
        player.y += MOVE_DISTANCE
        moved = True

    elif key == keys.LEFT:
        player.x -= MOVE_DISTANCE
        moved = True

    elif key == keys.RIGHT:
        player.x += MOVE_DISTANCE
        moved = True

    # Keep player on screen
    player.x = max(0, min(player.x, WIDTH - player.width))
    player.y = max(0, min(player.y, HEIGHT - player.height))

    # Score when moving upward
    if moved:
        # sounds.jump.play()

        if key == keys.UP:
            score += 1

    # Reached top
    if player.y <= 0:
        score += 10

        player.x = 280
        player.y = 740


def draw():
    screen.clear()
    
    screen.draw.text(
        f"Speed: {speed_multiplier:.1f}x",
        (20, 60),
        fontsize=35,
        color="white"
    )
    # Grass background
    screen.fill((30, 160, 30))

    # Road lanes
    for lane in LANES:
        screen.draw.filled_rect(
            Rect((0, lane), (WIDTH, GRID)),
            (50, 50, 50)
        )

    # Draw cars
    for car in cars:
        screen.draw.filled_rect(car["rect"], car["color"])

    # Draw player
    screen.draw.filled_rect(player, (255, 255, 255))

    # Score
    screen.draw.text(
        f"Score: {score}",
        (20, 20),
        fontsize=40,
        color="white"
    )

    if game_over:
        screen.draw.text(
            "GAME OVER",
            center=(WIDTH // 2, HEIGHT // 2 - 40),
            fontsize=70,
            color="red"
        )

        screen.draw.text(
            "Press SPACE to restart",
            center=(WIDTH // 2, HEIGHT // 2 + 30),
            fontsize=40,
            color="white"
        )
pgzrun.go()