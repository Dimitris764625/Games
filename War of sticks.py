import pygame
import sys
import pickle
import random
import math
from pygame import mixer

# Initialize Pygame
pygame.init()
mixer.init()

# Constants
WIDTH, HEIGHT = 800, 600
FPS = 60
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (200, 50, 50)
BLUE = (0, 100, 200)
GREEN = (0, 255, 0)
GREY = (200, 200, 200)
YELLOW = (255, 255, 0)

# Setup screen
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("War of Sticks")
clock = pygame.time.Clock()

# Fonts
font = pygame.font.SysFont(None, 30)
big_font = pygame.font.SysFont(None, 60)

# Load sounds
try:
    # Combat sounds
    sword_sound = mixer.Sound('sword.wav')  # Placeholder - create or find these files
    arrow_sound = mixer.Sound('arrow.wav')
    spawn_sound = mixer.Sound('spawn.wav')
    victory_sound = mixer.Sound('victory.wav')
    defeat_sound = mixer.Sound('defeat.wav')
    
    # Background music
    mixer.music.load('background.mp3')
    mixer.music.set_volume(0.5)
    mixer.music.play(-1)  # Loop indefinitely
except:
    print("Warning: Sound files not found. Continuing without sound.")

class Base:
    def __init__(self, x, color):
        self.rect = pygame.Rect(x, HEIGHT//2 - 50, 60, 100)
        self.color = color
        self.health = 1000
        self.max_health = 1000
        self.shake_timer = 0
        self.shake_intensity = 0

    def draw(self):
        # Apply screen shake if active
        shake_offset = 0
        if self.shake_timer > 0:
            self.shake_timer -= 1
            shake_offset = random.randint(-self.shake_intensity, self.shake_intensity)
        
        pygame.draw.rect(screen, self.color, (self.rect.x + shake_offset, self.rect.y, self.rect.width, self.rect.height))
        # Health bar background
        pygame.draw.rect(screen, RED, (self.rect.x + shake_offset, self.rect.y - 20, self.rect.width, 10))
        # Health bar foreground
        health_width = max(0, self.rect.width * (self.health / self.max_health))
        pygame.draw.rect(screen, GREEN, (self.rect.x + shake_offset, self.rect.y - 20, health_width, 10))

    def take_damage(self, amount):
        self.health -= amount
        self.shake_timer = 10
        self.shake_intensity = 5

class Miner:
    def __init__(self, base_x):
        self.x = base_x + 70
        self.y = HEIGHT//2 + 30
        self.size = 10
        self.is_worker = True
        self.animation_timer = 0

    def draw(self):
        # Add simple animation
        self.animation_timer += 0.1
        anim_offset = math.sin(self.animation_timer) * 2
        pygame.draw.circle(screen, (255, 215, 0), (self.x, self.y + anim_offset), self.size)

class Arrow:
    def __init__(self, x, y, direction, owner):
        self.x = x
        self.y = y
        self.speed = 4 * direction
        self.owner = owner
        self.width = 8
        self.height = 3
        self.active = True
        self.animation_frame = 0

    def move(self):
        self.x += self.speed
        if self.x < 0 or self.x > WIDTH:
            self.active = False

    def draw(self):
        self.animation_frame += 1
        # Add a simple trail effect
        for i in range(1, 4):
            alpha = 255 - (i * 60)
            if alpha > 0:
                trail_color = (150, 75, 0, alpha)
                trail_rect = pygame.Rect(self.x - (i * 3 * (self.speed/abs(self.speed)), self.y, self.width, self.height))
                pygame.draw.rect(screen, trail_color, trail_rect)
        
        pygame.draw.rect(screen, (150, 75, 0), (self.x, self.y, self.width, self.height))

class DamageText:
    def __init__(self, x, y, amount):
        self.x = x
        self.y = y
        self.amount = amount
        self.lifetime = 30
        self.color = RED
        self.alpha = 255
        
    def update(self):
        self.lifetime -= 1
        self.y -= 1
        self.alpha = max(0, self.alpha - 8)
        
    def draw(self):
        text = font.render(f"-{self.amount}", True, self.color)
        text.set_alpha(self.alpha)
        screen.blit(text, (self.x, self.y))

class Particle:
    def __init__(self, x, y, color):
        self.x = x
        self.y = y
        self.color = color
        self.size = random.randint(2, 5)
        self.speed = [random.uniform(-2, 2), random.uniform(-2, 2)]
        self.lifetime = random.randint(10, 20)
        
    def update(self):
        self.x += self.speed[0]
        self.y += self.speed[1]
        self.lifetime -= 1
        self.size = max(0, self.size - 0.1)
        
    def draw(self):
        pygame.draw.circle(screen, self.color, (int(self.x), int(self.y)), int(self.size))

class Soldier:
    def __init__(self, x, y, direction, is_enemy=False, unit_type="swordsman"):
        self.x = x
        self.y = y
        self.direction = direction
        self.width = 10
        self.height = 20
        self.is_enemy = is_enemy
        self.alive = True
        self.target = None
        self.type = unit_type
        self.is_worker = False
        self.attack_cooldown = 0
        self.animation_frame = 0
        
        # Set stats based on unit type
        if unit_type == "swordsman":
            self.color = (0, 0, 0) if not is_enemy else (255, 0, 0)
            self.health = 30
            self.max_health = 30
            self.attack = 2
            self.range = 5
            self.speed = 1
            self.attack_speed = 30
        elif unit_type == "archer":
            self.color = (0, 100, 0) if not is_enemy else (255, 100, 100)
            self.health = 20
            self.max_health = 20
            self.attack = 1
            self.range = 100
            self.speed = 1.2
            self.attack_speed = 60
        elif unit_type == "tank":
            self.color = (100, 50, 0) if not is_enemy else (150, 0, 0)
            self.health = 80
            self.max_health = 80
            self.attack = 1
            self.range = 10
            self.speed = 0.5
            self.attack_speed = 45

    def move(self):
        if not self.target and self.alive:
            self.x += self.speed * self.direction
            self.animation_frame += 0.1

    def find_target(self, enemies):
        for enemy in enemies:
            if not enemy.alive:
                continue
            dist = abs(self.x - enemy.x)
            if dist <= self.range:
                self.target = enemy
                enemy.target = self
                return

    def attack_target(self):
        if self.attack_cooldown > 0:
            self.attack_cooldown -= 1
            
        if self.type == "archer" and self.alive and self.target and self.target.alive:
            if self.attack_cooldown <= 0:
                arrow = Arrow(self.x + (10 * self.direction), self.y + 5, self.direction, self)
                arrows.append(arrow)
                self.attack_cooldown = self.attack_speed
                try:
                    arrow_sound.play()
                except:
                    pass
        elif self.target and self.alive and self.target.alive and self.attack_cooldown <= 0:
            self.target.health -= self.attack
            damage_texts.append(DamageText(self.target.x, self.target.y - 20, self.attack))
            
            # Add attack animation
            self.animation_frame = 10
            
            # Add particles
            for _ in range(5):
                particles.append(Particle(self.target.x + random.randint(-5, 5), 
                                        self.target.y + random.randint(-5, 5), 
                                        RED))
            
            try:
                sword_sound.play()
            except:
                pass
                
            if self.target.health <= 0:
                self.target.alive = False
                self.target = None
            self.attack_cooldown = self.attack_speed

    def draw(self):
        if self.alive:
            # Attack animation
            anim_offset = 0
            if self.animation_frame > 0:
                anim_offset = math.sin(self.animation_frame) * 5
                self.animation_frame -= 1
                
            pygame.draw.rect(screen, self.color, (self.x, self.y + anim_offset, self.width, self.height))
            # Health bar
            health_width = max(0, self.width * (self.health / self.max_health))
            pygame.draw.rect(screen, RED, (self.x, self.y - 10 + anim_offset, self.width, 5))
            pygame.draw.rect(screen, GREEN, (self.x, self.y - 10 + anim_offset, health_width, 5))

class Button:
    def __init__(self, x, y, w, h, text):
        self.rect = pygame.Rect(x, y, w, h)
        self.text = text
        self.hover = False

    def draw(self, surface):
        color = (150, 150, 150) if self.hover else GREY
        pygame.draw.rect(surface, color, self.rect)
        pygame.draw.rect(surface, BLACK, self.rect, 2)  # Border
        
        txt_surface = font.render(self.text, True, BLACK)
        surface.blit(txt_surface, (self.rect.x + self.rect.w//2 - txt_surface.get_width()//2, 
                                 self.rect.y + self.rect.h//2 - txt_surface.get_height()//2))

    def is_clicked(self, pos):
        self.hover = self.rect.collidepoint(pos)
        return self.hover

# Game state
player_base = Base(50, BLUE)
enemy_base = Base(WIDTH - 110, RED)
player_units = []
enemy_units = []
arrows = []
damage_texts = []
particles = []

# Resources
gold = 100
stone = 50
current_wave = 1
enemies_remaining = 0
wave_in_progress = False

# Upgrade levels
miner_upgrade_level = 1
soldier_attack_upgrade = 0
soldier_health_upgrade = 0

# Costs
miner_cost = 50
soldier_cost = 100
upgrade_cost = 50
wave_reward_gold = 100
wave_reward_stone = 20

# Game timing
gold_per_tick = 1
tick_interval = 1000
last_tick = pygame.time.get_ticks()
enemy_spawn_interval = 5000
last_enemy_spawn = pygame.time.get_ticks()

# Game state
game_over = False
winner = ""

# Buttons
save_button = Button(50, 200, 100, 40, "Save")
load_button = Button(50, 250, 100, 40, "Load")
wave_button = Button(WIDTH - 150, 50, 100, 40, "Start Wave")

# Tooltips
tooltips = {
    "swordsman": "Swordsman: Balanced unit with medium health and attack",
    "archer": "Archer: Long-range but fragile",
    "tank": "Tank: High health but slow",
    "miner": "Miner: Generates gold over time",
    "upgrade_miner": "Increases gold generation per miner",
    "upgrade_attack": "Increases soldier attack power",
    "upgrade_health": "Increases soldier health"
}

def save_game(filename="savegame.pkl"):
    save_data = {
        'gold': gold,
        'stone': stone,
        'current_wave': current_wave,
        'player_units': [{'x': u.x, 'y': u.y, 'health': u.health, 'type': u.type} for u in player_units if isinstance(u, Soldier)],
        'miners': [{'x': m.x, 'y': m.y} for m in player_units if isinstance(m, Miner)],
        'enemy_units': [{'x': u.x, 'y': u.y, 'health': u.health, 'type': u.type} for u in enemy_units],
        'player_base_health': player_base.health,
        'enemy_base_health': enemy_base.health,
        'upgrades': {
            'miner': miner_upgrade_level,
            'attack': soldier_attack_upgrade,
            'health': soldier_health_upgrade
        }
    }
    with open(filename, 'wb') as f:
        pickle.dump(save_data, f)
    print("Game Saved!")

def load_game(filename="savegame.pkl"):
    global gold, stone, current_wave, player_units, enemy_units, miner_upgrade_level, soldier_attack_upgrade, soldier_health_upgrade
    
    try:
        with open(filename, 'rb') as f:
            save_data = pickle.load(f)
        
        gold = save_data['gold']
        stone = save_data['stone']
        current_wave = save_data['current_wave']
        
        player_units.clear()
        for data in save_data.get('miners', []):
            miner = Miner(player_base.rect.x)
            miner.x = data['x']
            miner.y = data['y']
            player_units.append(miner)
            
        for data in save_data.get('player_units', []):
            soldier = Soldier(data['x'], data['y'], 1, False, data['type'])
            soldier.health = data['health']
            player_units.append(soldier)
            
        enemy_units.clear()
        for data in save_data.get('enemy_units', []):
            soldier = Soldier(data['x'], data['y'], -1, True, data['type'])
            soldier.health = data['health']
            enemy_units.append(soldier)
            
        player_base.health = save_data['player_base_health']
        enemy_base.health = save_data['enemy_base_health']
        
        upgrades = save_data.get('upgrades', {})
        miner_upgrade_level = upgrades.get('miner', 1)
        soldier_attack_upgrade = upgrades.get('attack', 0)
        soldier_health_upgrade = upgrades.get('health', 0)
        
        print("Game Loaded!")
    except FileNotFoundError:
        print("No save file found.")
    except Exception as e:
        print(f"Error loading game: {e}")

def draw_tooltip(text, pos):
    text_surface = font.render(text, True, WHITE)
    bg_rect = pygame.Rect(pos[0], pos[1], text_surface.get_width() + 10, text_surface.get_height() + 10)
    pygame.draw.rect(screen, (50, 50, 50), bg_rect)
    pygame.draw.rect(screen, BLACK, bg_rect, 2)
    screen.blit(text_surface, (pos[0] + 5, pos[1] + 5))

def draw_window():
    screen.fill(WHITE)

    # Draw bases
    player_base.draw()
    enemy_base.draw()

    # Draw units
    for unit in player_units + enemy_units:
        unit.draw()
        
    # Draw arrows
    for arrow in arrows:
        arrow.draw()
        
    # Draw damage texts
    for text in damage_texts[:]:
        text.update()
        if text.lifetime <= 0:
            damage_texts.remove(text)
        else:
            text.draw()
            
    # Draw particles
    for particle in particles[:]:
        particle.update()
        if particle.lifetime <= 0:
            particles.remove(particle)
        else:
            particle.draw()

    # Draw buttons
    save_button.draw(screen)
    load_button.draw(screen)
    wave_button.draw(screen)

    # Draw resources
    screen.blit(font.render(f"Gold: {gold}", True, BLACK), (50, 50))
    screen.blit(font.render(f"Stone: {stone}", True, BLACK), (50, 80))
    screen.blit(font.render(f"Wave: {current_wave}", True, BLACK), (50, 110))
    screen.blit(font.render(f"Enemies: {enemies_remaining}", True, BLACK), (50, 140))
    
    # Draw upgrades info
    screen.blit(font.render(f"Miner Lvl: {miner_upgrade_level}  (1)", True, (0, 0, 100)), (20, 180))
    screen.blit(font.render(f"Atk Upgrade: {soldier_attack_upgrade}  (2)", True, (100, 0, 0)), (20, 210))
    screen.blit(font.render(f"HP Upgrade: {soldier_health_upgrade}  (3)", True, (0, 100, 0)), (20, 240))
    screen.blit(font.render(f"[1][2][3] = Use stone to upgrade", True, BLACK), (20, 270))

    # Draw unit counts
    miners_count = len([u for u in player_units if isinstance(u, Miner)])
    soldiers_count = len([u for u in player_units if isinstance(u, Soldier) and u.alive])
    screen.blit(font.render(f"Miners: {miners_count}", True, BLACK), (20, 330))
    screen.blit(font.render(f"Soldiers: {soldiers_count}", True, BLACK), (20, 360))

    # Instructions
    screen.blit(font.render("Press [M] to train miner (50 gold)", True, (50, 50, 50)), (20, HEIGHT - 60))
    screen.blit(font.render("Press [S] to train soldier (100 gold)", True, (50, 50, 50)), (20, HEIGHT - 30))

    # Tooltips
    mouse_pos = pygame.mouse.get_pos()
    if wave_button.rect.collidepoint(mouse_pos):
        draw_tooltip("Start the next wave of enemies", (mouse_pos[0], mouse_pos[1] - 30))
    elif save_button.rect.collidepoint(mouse_pos):
        draw_tooltip("Save your game progress", (mouse_pos[0], mouse_pos[1] - 30))
    elif load_button.rect.collidepoint(mouse_pos):
        draw_tooltip("Load a saved game", (mouse_pos[0], mouse_pos[1] - 30))

    # End screen
    if game_over:
        # Dark overlay
        s = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
        s.fill((0, 0, 0, 180))
        screen.blit(s, (0, 0))
        
        text = big_font.render(f"{winner} Wins!", True, YELLOW)
        screen.blit(text, (WIDTH // 2 - text.get_width() // 2, HEIGHT // 2 - 40))
        
        restart_text = font.render("Press R to restart", True, WHITE)
        screen.blit(restart_text, (WIDTH // 2 - restart_text.get_width() // 2, HEIGHT // 2 + 20))

    pygame.display.flip()

def handle_input():
    global gold, stone, miner_upgrade_level, soldier_attack_upgrade, soldier_health_upgrade, game_over, current_wave, wave_in_progress
    
    keys = pygame.key.get_pressed()
    if game_over:
        if keys[pygame.K_r]:
            # Reset game
            global player_base, enemy_base, player_units, enemy_units, arrows, damage_texts, particles
            player_base = Base(50, BLUE)
            enemy_base = Base(WIDTH - 110, RED)
            player_units = []
            enemy_units = []
            arrows = []
            damage_texts = []
            particles = []
            gold = 100
            stone = 50
            current_wave = 1
            miner_upgrade_level = 1
            soldier_attack_upgrade = 0
            soldier_health_upgrade = 0
            game_over = False
            wave_in_progress = False
            try:
                mixer.music.play(-1)
            except:
                pass
        return

    # Train units
    if keys[pygame.K_m] and gold >= miner_cost:
        player_units.append(Miner(player_base.rect.x))
        gold -= miner_cost
        pygame.time.wait(200)
        try:
            spawn_sound.play()
        except:
            pass

    if keys[pygame.K_s] and gold >= soldier_cost:
        soldier = Soldier(player_base.rect.x + 80, player_base.rect.y + 30, 1, False, "swordsman")
        player_units.append(soldier)
        gold -= soldier_cost
        pygame.time.wait(200)
        try:
            spawn_sound.play()
        except:
            pass

    # Upgrades
    if keys[pygame.K_1] and stone >= upgrade_cost:
        miner_upgrade_level += 1
        stone -= upgrade_cost
        pygame.time.wait(200)

    if keys[pygame.K_2] and stone >= upgrade_cost:
        soldier_attack_upgrade += 1
        # Update existing soldiers
        for unit in player_units:
            if isinstance(unit, Soldier):
                unit.attack += 1
        stone -= upgrade_cost
        pygame.time.wait(200)

    if keys[pygame.K_3] and stone >= upgrade_cost:
        soldier_health_upgrade += 1
        # Update existing soldiers
        for unit in player_units:
            if isinstance(unit, Soldier):
                unit.max_health += 5
                unit.health += 5
        stone -= upgrade_cost
        pygame.time.wait(200)

def collect_gold():
    global gold, last_tick
    now = pygame.time.get_ticks()
    if now - last_tick >= tick_interval:
        miners_count = len([u for u in player_units if isinstance(u, Miner)])
        gold += (gold_per_tick * miners_count) * miner_upgrade_level
        last_tick = now

def spawn_enemy():
    global last_enemy_spawn, current_wave, enemies_remaining, wave_in_progress
    
    if not wave_in_progress:
        return
        
    now = pygame.time.get_ticks()
    if now - last_enemy_spawn >= enemy_spawn_interval and enemies_remaining > 0:
        # Increase difficulty with waves
        unit_type = random.choice(["swordsman", "archer", "tank"])
        # Make enemies stronger each wave
        enemy = Soldier(enemy_base.rect.x - 20, enemy_base.rect.y + 30, -1, True, unit_type)
        
        # Scale enemy stats with wave number
        enemy.health += current_wave * 2
        enemy.max_health = enemy.health
        enemy.attack += current_wave // 3
        
        enemy_units.append(enemy)
        enemies_remaining -= 1
        last_enemy_spawn = now
        
        try:
            spawn_sound.play()
        except:
            pass
            
        # Check if wave is complete
        if enemies_remaining == 0 and len([u for u in enemy_units if u.alive]) == 0:
            wave_complete()

def wave_complete():
    global gold, stone, current_wave, wave_in_progress
    gold += wave_reward_gold
    stone += wave_reward_stone
    current_wave += 1
    wave_in_progress = False
    
    try:
        victory_sound.play()
    except:
        pass

def start_wave():
    global enemies_remaining, wave_in_progress
    if not wave_in_progress:
        enemies_remaining = current_wave * 3 + 2  # Scale enemies with wave
        wave_in_progress = True

def move_units():
    for unit in player_units + enemy_units:
        if isinstance(unit, Soldier):
            unit.move()
    
    for arrow in arrows:
        arrow.move()

def handle_combat():
    global game_over, winner
    
    # Targeting
    player_soldiers = [u for u in player_units if isinstance(u, Soldier) and u.alive]
    enemy_soldiers = [u for u in enemy_units if isinstance(u, Soldier) and u.alive]
    
    for soldier in player_soldiers:
        if soldier.alive and not soldier.target:
            soldier.find_target(enemy_soldiers)

    for enemy in enemy_soldiers:
        if enemy.alive and not enemy.target:
            enemy.find_target(player_soldiers)

    # Combat
    for soldier in player_soldiers + enemy_soldiers:
        soldier.attack_target()
        
    # Arrow collisions
    for arrow in arrows[:]:
        if not arrow.active:
            arrows.remove(arrow)
            continue
            
        targets = enemy_soldiers if not arrow.owner.is_enemy else player_soldiers
        for target in targets:
            if (abs(arrow.x - target.x) < 20 and 
                abs(arrow.y - target.y) < 20):
                target.health -= arrow.owner.attack
                damage_texts.append(DamageText(target.x, target.y - 20, arrow.owner.attack))
                arrow.active = False
                
                # Add hit particles
                for _ in range(5):
                    particles.append(Particle(target.x + random.randint(-5, 5), 
                                            target.y + random.randint(-5, 5), 
                                            RED))
                break

    # Unit vs base combat
    for soldier in player_soldiers:
        if soldier.alive and soldier.x >= enemy_base.rect.x - 5:
            enemy_base.take_damage(soldier.attack)
            soldier.alive = False
            
            # Add explosion particles
            for _ in range(10):
                particles.append(Particle(enemy_base.rect.x + random.randint(0, enemy_base.rect.width), 
                                enemy_base.rect.y + random.randint(0, enemy_base.rect.height), 
                                YELLOW))

    for enemy in enemy_soldiers:
        if enemy.alive and enemy.x <= player_base.rect.x + player_base.rect.width:
            player_base.take_damage(enemy.attack)
            enemy.alive = False
            
            # Add explosion particles
            for _ in range(10):
                particles.append(Particle(player_base.rect.x + random.randint(0, player_base.rect.width), 
                                player_base.rect.y + random.randint(0, player_base.rect.height), 
                                YELLOW))

    # Check game over
    if player_base.health <= 0 and not game_over:
        game_over = True
        winner = "Enemy"
        try:
            defeat_sound.play()
            mixer.music.stop()
        except:
            pass

    if enemy_base.health <= 0 and not game_over:
        game_over = True
        winner = "Player"
        try:
            victory_sound.play()
            mixer.music.stop()
        except:
            pass

    # Clean up dead units
    for unit in player_units[:]:
        if isinstance(unit, Soldier) and not unit.alive:
            player_units.remove(unit)
            
    for unit in enemy_units[:]:
        if isinstance(unit, Soldier) and not unit.alive:
            enemy_units.remove(unit)

def main():
    running = True
    while running:
        clock.tick(FPS)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            if event.type == pygame.MOUSEBUTTONDOWN:
                pos = pygame.mouse.get_pos()
                if save_button.is_clicked(pos):
                    save_game()
                if load_button.is_clicked(pos):
                    load_game()
                if wave_button.is_clicked(pos) and not wave_in_progress and not game_over:
                    start_wave()

        if not game_over:
            handle_input()
            collect_gold()
            spawn_enemy()
            move_units()
            handle_combat()

        draw_window()

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
