import pygame
import math
import sys
from urllib.request import urlopen
from io import BytesIO

class SteeringWheelDisplay:
    def __init__(self, width=800, height=600, image_url=None, transparent=False):
        pygame.init()
        self.width = width
        self.height = height
        self.transparent = transparent
        
        if transparent:
            # Create display with per-pixel alpha for transparency
            self.screen = pygame.display.set_mode((width, height), pygame.SRCALPHA)
        else:
            self.screen = pygame.display.set_mode((width, height))
        
        pygame.display.set_caption("Pothole Assist - Steering Display")
        
        # Colors
        self.BLACK = (0, 0, 0)
        self.WHITE = (255, 255, 255)
        self.GRAY = (100, 100, 100)
        self.BLUE = (0, 120, 215)
        self.RED = (220, 50, 50)
        
        # Display colors
        if transparent:
            self.BG_COLOR = (0, 0, 0, 0)  # Fully transparent
        else:
            self.BG_COLOR = self.WHITE
        self.TEXT_COLOR = self.BLACK
        
        # Steering wheel properties
        self.center_x = width // 2
        self.center_y = height // 2
        self.current_angle = 0  # Current steering angle in degrees
        
        # Load steering wheel image
        self.load_steering_image(image_url)
        
        # Font
        self.font_large = pygame.font.Font(None, 48)
        self.font_small = pygame.font.Font(None, 32)
        
        self.clock = pygame.time.Clock()
        
    def load_steering_image(self, image_url):
        """Load steering wheel image from URL or local file"""
        try:
            if image_url:
                if image_url.startswith('http'):
                    # Load from URL
                    image_data = urlopen(image_url).read()
                    image_file = BytesIO(image_data)
                    self.steering_image_original = pygame.image.load(image_file)
                else:
                    # Load from local file
                    self.steering_image_original = pygame.image.load(image_url)
                
                # Get original dimensions
                original_width = self.steering_image_original.get_width()
                original_height = self.steering_image_original.get_height()
                
                # Scale to target size while maintaining aspect ratio
                target_size = 300
                aspect_ratio = original_width / original_height
                
                if aspect_ratio > 1:  # Wider than tall
                    new_width = target_size
                    new_height = int(target_size / aspect_ratio)
                else:  # Taller than wide or square
                    new_height = target_size
                    new_width = int(target_size * aspect_ratio)
                
                self.steering_image_original = pygame.transform.scale(
                    self.steering_image_original, (new_width, new_height))
                
                print(f"Steering wheel image loaded successfully! ({new_width}x{new_height})")
            else:
                # Create a default drawn steering wheel if no image provided
                self.steering_image_original = self.create_default_wheel()
                
        except Exception as e:
            print(f"Error loading image: {e}")
            print("Using default drawn steering wheel instead.")
            self.steering_image_original = self.create_default_wheel()
    
    def create_default_wheel(self):
        """Create a default steering wheel surface"""
        size = 300
        surface = pygame.Surface((size, size), pygame.SRCALPHA)
        center = size // 2
        radius = size // 2 - 10
        
        # Draw outer circle (wheel rim)
        pygame.draw.circle(surface, self.GRAY, (center, center), radius, 8)
        
        # Draw inner circle
        pygame.draw.circle(surface, self.WHITE, (center, center), radius - 20, 2)
        
        # Draw spokes (3 spokes at 120 degrees apart)
        for i in range(3):
            spoke_angle = math.radians(i * 120 - 90)
            end_x = center + int(radius * 0.9 * math.cos(spoke_angle))
            end_y = center + int(radius * 0.9 * math.sin(spoke_angle))
            pygame.draw.line(surface, self.GRAY, (center, center), (end_x, end_y), 6)
        
        # Draw center hub
        pygame.draw.circle(surface, self.BLUE, (center, center), 25)
        
        # Draw top indicator
        indicator_y = center - radius + 30
        pygame.draw.circle(surface, self.RED, (center, indicator_y), 15)
        
        return surface
        
    def draw_steering_wheel(self, angle):
        """Draw the steering wheel rotated by the given angle"""
        # Rotate the image
        rotated_image = pygame.transform.rotate(self.steering_image_original, angle)
        
        # Get rect and center it
        rect = rotated_image.get_rect(center=(self.center_x, self.center_y))
        
        # Draw the rotated image
        self.screen.blit(rotated_image, rect)
        
    def draw_angle_display(self, angle):
        """Display the current steering angle as text"""
        # Angle value
        angle_text = self.font_large.render(f"{angle:.1f}°", True, self.TEXT_COLOR)
        angle_rect = angle_text.get_rect(center=(self.center_x, self.height - 80))
        self.screen.blit(angle_text, angle_rect)
        
        # Label
        label_text = self.font_small.render("Steering Angle", True, self.TEXT_COLOR)
        label_rect = label_text.get_rect(center=(self.center_x, self.height - 40))
        self.screen.blit(label_text, label_rect)
        
        # Direction indicator
        if angle > 5:
            direction = "LEFT"
            color = self.TEXT_COLOR
        elif angle < -5:
            direction = "RIGHT"
            color = self.TEXT_COLOR
        else:
            direction = "CENTER"
            color = self.TEXT_COLOR
            
        dir_text = self.font_small.render(direction, True, color)
        dir_rect = dir_text.get_rect(center=(self.center_x, 50))
        self.screen.blit(dir_text, dir_rect)
        
    def update_angle(self, new_angle):
        """Update the steering angle (to be called from OBD reader)"""
        self.current_angle = new_angle
        
    def run(self):
        """Main display loop"""
        running = True
        
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                # Keyboard input for testing
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_LEFT:
                        self.current_angle += 10
                    elif event.key == pygame.K_RIGHT:
                        self.current_angle -= 10
                    elif event.key == pygame.K_SPACE:
                        self.current_angle = 0
            
            # Clear screen
            self.screen.fill(self.BG_COLOR)
            
            # Draw everything
            self.draw_steering_wheel(self.current_angle)
            self.draw_angle_display(self.current_angle)
            
            # Title
            title = self.font_small.render("Pothole Assist", True, self.TEXT_COLOR)
            self.screen.blit(title, (20, 20))
            
            # Instructions (for testing)
            inst_text = self.font_small.render("Arrow Keys: Manual | Space: Center", 
                                              True, self.TEXT_COLOR)
            self.screen.blit(inst_text, (20, self.height - 30))
            
            pygame.display.flip()
            self.clock.tick(60)  # 60 FPS
            
        pygame.quit()
        sys.exit()

if __name__ == "__main__":

    image_url = "steering_wheel.jpg"

    # Set transparent=True for overlay mode (camera feed shows through)
    # Set transparent=False for standalone display with white background
    display = SteeringWheelDisplay(image_url=image_url, transparent=False)
    display.run()