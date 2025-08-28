import csv
import os
import tkinter as tk
from tkinter import messagebox, filedialog, simpledialog
import math
import configparser

# ===== Configuration =====
# Load configuration from config.ini
def load_config():
    config = configparser.ConfigParser()
    config_path = os.path.join(os.path.dirname(__file__), "..", "config.ini")
    
    if not os.path.exists(config_path):
        # Fallback values if config.ini doesn't exist
        return {
            'map_width_multiplier': 10,
            'map_height_multiplier': 3,
            'block_size': 50,
            'buffer_width': 1280,
            'buffer_height': 720
        }
    
    config.read(config_path)
    
    # Get buffer dimensions for calculations
    buffer_width = config.getint('Buffer', 'buffer_width', fallback=1280)
    buffer_height = config.getint('Buffer', 'buffer_height', fallback=720)
    
    # Get map multipliers from config
    map_width_multiplier = config.getint('Map', 'map_width_multiplier', fallback=10)
    map_height_multiplier = config.getint('Map', 'map_height_multiplier', fallback=3)
    block_size = config.getint('Map', 'block_size', fallback=50)
    
    # Calculate actual map dimensions
    map_width = buffer_width * map_width_multiplier
    map_height = buffer_height * map_height_multiplier
    
    return {
        'map_width': map_width,
        'map_height': map_height,
        'map_width_multiplier': map_width_multiplier,
        'map_height_multiplier': map_height_multiplier,
        'block_size': block_size,
        'buffer_width': buffer_width,
        'buffer_height': buffer_height
    }

# Load map editor specific configuration
def load_map_editor_config():
    config = configparser.ConfigParser()
    config_path = os.path.join(os.path.dirname(__file__), "..", "config.ini")
    
    if not os.path.exists(config_path):
        # Fallback values if config.ini doesn't exist
        return {
            'canvas_scale': 5,
            'canvas_width': 1200,
            'canvas_height': 600
        }
    
    config.read(config_path)
    
    canvas_scale = config.getint('MapEditor', 'canvas_scale', fallback=5)
    canvas_width = config.getint('MapEditor', 'canvas_width', fallback=1200)
    canvas_height = config.getint('MapEditor', 'canvas_height', fallback=600)
    
    return {
        'canvas_scale': canvas_scale,
        'canvas_width': canvas_width,
        'canvas_height': canvas_height
    }

# Load configuration
CONFIG = load_config()
MAP_WIDTH = CONFIG['map_width']
MAP_HEIGHT = CONFIG['map_height']
BLOCK_SIZE = CONFIG['block_size']

# Load map editor configuration
MAP_EDITOR_CONFIG = load_map_editor_config()
CANVAS_SCALE = MAP_EDITOR_CONFIG['canvas_scale']
CANVAS_WIDTH = MAP_WIDTH // CANVAS_SCALE
CANVAS_HEIGHT = MAP_HEIGHT // CANVAS_SCALE
GRID_SIZE = BLOCK_SIZE // CANVAS_SCALE  # Grid size

# Get the directory where this script is located
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
# Set output directory relative to script location
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "..", "resources", "stages")
os.makedirs(OUTPUT_DIR, exist_ok=True)

class Block:
    def __init__(self, x, y, block_type="ground"):
        self.x = x
        self.y = y
        self.block_type = block_type
        self.width = BLOCK_SIZE
        self.height = BLOCK_SIZE
    
    def to_csv_row(self):
        """Convert block to CSV row format"""
        return (self.block_type, self.x, self.y, self.x + self.width, self.y + self.height)
    
    def get_color(self):
        """Return color according to block type"""
        if self.block_type == "ground":
            return "brown"
        elif self.block_type == "grass":
            return "green"
        else:
            return "gray"  # Default color

class Enemy:
    def __init__(self, x, y, enemy_type="basic", difficulty=1):
        self.x = x
        self.y = y
        self.enemy_type = enemy_type  # "basic", "fast", "tank"
        self.difficulty = difficulty  # 1=easy, 2=normal, 3=hard
    
    def to_csv_row(self):
        """Convert enemy to CSV row format"""
        return (self.x, self.y, self.enemy_type, self.difficulty)
    
    def get_color(self):
        """Return color according to enemy type and difficulty"""
        if self.enemy_type == "tank":
            return "purple" if self.difficulty == 1 else "darkviolet" if self.difficulty == 2 else "indigo"
        elif self.enemy_type == "helicopter":
            return "orange" if self.difficulty == 1 else "darkorange" if self.difficulty == 2 else "saddlebrown"
        else:
            return "gray"
    
    def get_display_text(self):
        """Get text to display on enemy"""
        return f"{self.enemy_type[0].upper()}{self.difficulty}"

class SpawnPoint:
    def __init__(self, x, y, spawn_type="tank"):
        self.x = x
        self.y = y
        self.spawn_type = spawn_type  # "tank" for main tank spawn
    
    def to_csv_row(self):
        """Convert spawn point to CSV row format"""
        return (self.x, self.y, self.spawn_type)
    
    def get_color(self):
        """Return color according to spawn type"""
        if self.spawn_type == "tank":
            return "lime"  # Bright green for tank spawn
        else:
            return "yellow"
    
    def get_display_text(self):
        """Get text to display on spawn point"""
        return "T" if self.spawn_type == "tank" else "S"

class MapEditor:
    def __init__(self, root):
        self.root = root
        self.root.title("Tank Boy Map Editor")
        self.root.geometry("1400x800")
        
        # Store block data
        self.blocks = []
        self.current_stage = 1
        
        # Store enemy data
        self.enemies = []
        
        # Store spawn point data
        self.spawn_points = []
        
        # Current drawing mode ('draw', 'erase', 'enemy', or 'spawn')
        self.draw_mode = 'draw'
        
        # Current block type ('ground' or 'grass')
        self.current_block_type = 'ground'
        
        # Current enemy type and difficulty
        self.current_enemy_type = 'tank'
        self.current_enemy_difficulty = 1
        
        # Current spawn type
        self.current_spawn_type = 'tank'
        
        # Brush size (number of blocks to paint at once)
        self.brush_size = 1
        

        
        self.setup_ui()
        self.setup_canvas()
        
        # Create default ground blocks
        self.create_default_ground()
        
        # Initial canvas drawing
        self.redraw_canvas()
        
    def setup_ui(self):
        # Top toolbar
        toolbar = tk.Frame(self.root)
        toolbar.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)
        
        # Stage selection
        tk.Label(toolbar, text="Stage:").pack(side=tk.LEFT)
        self.stage_var = tk.StringVar(value="1")
        stage_entry = tk.Entry(toolbar, textvariable=self.stage_var, width=5)
        stage_entry.pack(side=tk.LEFT, padx=5)
        
        # Buttons
        tk.Button(toolbar, text="Load", command=self.load_map).pack(side=tk.LEFT, padx=5)
        tk.Button(toolbar, text="Save", command=self.save_map).pack(side=tk.LEFT, padx=5)
        
        # Separator
        tk.Frame(toolbar, width=2, bg="gray").pack(side=tk.LEFT, fill=tk.Y, padx=10)
        
        # Drawing mode selection
        self.mode_var = tk.StringVar(value="draw")
        tk.Radiobutton(toolbar, text="Draw", variable=self.mode_var, value="draw", 
                      command=self.change_mode).pack(side=tk.LEFT, padx=5)
        tk.Radiobutton(toolbar, text="Erase", variable=self.mode_var, value="erase", 
                      command=self.change_mode).pack(side=tk.LEFT, padx=5)
        tk.Radiobutton(toolbar, text="Enemy", variable=self.mode_var, value="enemy", 
                      command=self.change_mode).pack(side=tk.LEFT, padx=5)
        tk.Radiobutton(toolbar, text="Spawn", variable=self.mode_var, value="spawn", 
                      command=self.change_mode).pack(side=tk.LEFT, padx=5)
        
        # Separator
        tk.Frame(toolbar, width=2, bg="gray").pack(side=tk.LEFT, fill=tk.Y, padx=10)
        
        # Block type selection
        tk.Label(toolbar, text="Block Type:").pack(side=tk.LEFT)
        self.block_type_var = tk.StringVar(value="ground")
        tk.Radiobutton(toolbar, text="Ground", variable=self.block_type_var, value="ground", 
                      command=self.change_block_type, bg="brown", activebackground="brown").pack(side=tk.LEFT, padx=2)
        tk.Radiobutton(toolbar, text="Grass", variable=self.block_type_var, value="grass", 
                      command=self.change_block_type, bg="green", activebackground="green").pack(side=tk.LEFT, padx=2)
        
        # Separator
        tk.Frame(toolbar, width=2, bg="gray").pack(side=tk.LEFT, fill=tk.Y, padx=10)
        
        # Enemy type selection (only visible when enemy mode is selected)
        self.enemy_frame = tk.Frame(self.root)
        tk.Label(self.enemy_frame, text="Enemy Type:").pack(side=tk.LEFT)
        self.enemy_type_var = tk.StringVar(value="tank")
        enemy_types = ["tank", "helicopter"]
        enemy_combo = tk.OptionMenu(self.enemy_frame, self.enemy_type_var, *enemy_types, command=self.change_enemy_type)
        enemy_combo.pack(side=tk.LEFT, padx=5)
        
        # Enemy difficulty selection
        tk.Label(self.enemy_frame, text="Difficulty:").pack(side=tk.LEFT)
        self.enemy_difficulty_var = tk.StringVar(value="1")
        difficulty_combo = tk.OptionMenu(self.enemy_frame, self.enemy_difficulty_var, "1", "2", "3", command=self.change_enemy_difficulty)
        difficulty_combo.pack(side=tk.LEFT, padx=5)
        
        # Initially hide enemy frame (it will be shown when enemy mode is selected)
        self.enemy_frame.pack_forget()
        
        # Spawn point settings frame (only visible when spawn mode is selected)
        self.spawn_frame = tk.Frame(self.root)
        tk.Label(self.spawn_frame, text="Spawn Type:").pack(side=tk.LEFT)
        self.spawn_type_var = tk.StringVar(value="tank")
        spawn_types = ["tank"]  # Can be extended for other spawn types in the future
        spawn_combo = tk.OptionMenu(self.spawn_frame, self.spawn_type_var, *spawn_types, command=self.change_spawn_type)
        spawn_combo.pack(side=tk.LEFT, padx=5)
        
        # Add note for spawn point
        tk.Label(self.spawn_frame, text="(Only one tank spawn allowed per stage)", fg="gray").pack(side=tk.LEFT, padx=10)
        
        # Initially hide spawn frame
        self.spawn_frame.pack_forget()
        
        # Separator
        tk.Frame(toolbar, width=2, bg="gray").pack(side=tk.LEFT, fill=tk.Y, padx=10)
        
        # Brush size selection
        tk.Label(toolbar, text="Brush Size:").pack(side=tk.LEFT)
        self.brush_var = tk.StringVar(value="1")
        brush_sizes = ["1", "2", "3", "4", "5"]
        brush_combo = tk.OptionMenu(toolbar, self.brush_var, *brush_sizes, command=self.change_brush_size)
        brush_combo.pack(side=tk.LEFT, padx=5)
        
        # Information display
        self.info_label = tk.Label(toolbar, text=f"Map: {MAP_WIDTH}x{MAP_HEIGHT} | Block: {BLOCK_SIZE}px | Scale: 1/{CANVAS_SCALE} | Buffer: {CONFIG['buffer_width']}x{CONFIG['buffer_height']}")
        self.info_label.pack(side=tk.RIGHT, padx=10)
        
        # Coordinate display
        self.coord_label = tk.Label(toolbar, text="Click to see coordinates", fg="blue")
        self.coord_label.pack(side=tk.RIGHT, padx=10)
        
        # Refresh config button
        tk.Button(toolbar, text="🔄 Refresh Config", command=self.refresh_config, bg="lightblue").pack(side=tk.RIGHT, padx=5)
        

        
    def setup_canvas(self):
        # Canvas frame
        canvas_frame = tk.Frame(self.root)
        canvas_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Scrollbars
        h_scrollbar = tk.Scrollbar(canvas_frame, orient=tk.HORIZONTAL)
        v_scrollbar = tk.Scrollbar(canvas_frame, orient=tk.VERTICAL)
        
        # Canvas - use actual map dimensions for scroll region
        map_canvas_width = MAP_WIDTH // CANVAS_SCALE
        map_canvas_height = MAP_HEIGHT // CANVAS_SCALE
        
        self.canvas = tk.Canvas(canvas_frame, 
                               width=min(map_canvas_width, MAP_EDITOR_CONFIG['canvas_width']), 
                               height=min(map_canvas_height, MAP_EDITOR_CONFIG['canvas_height']),
                               scrollregion=(0, 0, map_canvas_width, map_canvas_height),
                               xscrollcommand=h_scrollbar.set,
                               yscrollcommand=v_scrollbar.set,
                               bg="lightblue")
        
        h_scrollbar.config(command=self.canvas.xview)
        v_scrollbar.config(command=self.canvas.yview)
        
        # Layout
        self.canvas.grid(row=0, column=0, sticky="nsew")
        h_scrollbar.grid(row=1, column=0, sticky="ew")
        v_scrollbar.grid(row=0, column=1, sticky="ns")
        
        canvas_frame.grid_rowconfigure(0, weight=1)
        canvas_frame.grid_columnconfigure(0, weight=1)
        
        # Draw grid and boundaries
        self.draw_grid()
        self.draw_boundaries()
        
        # Event binding
        self.canvas.bind("<Button-1>", self.on_canvas_click)
        self.canvas.bind("<B1-Motion>", self.on_canvas_drag)
        self.canvas.bind("<MouseWheel>", self.on_mouse_wheel)
        
        # Focus for keyboard events
        self.canvas.focus_set()
        
    def draw_grid(self):
        """Draw grid"""
        # Use actual map dimensions
        map_canvas_width = MAP_WIDTH // CANVAS_SCALE
        map_canvas_height = MAP_HEIGHT // CANVAS_SCALE
        
        # Vertical lines
        for x in range(0, map_canvas_width + 1, GRID_SIZE):
            self.canvas.create_line(x, 0, x, map_canvas_height, fill="gray", width=1, tags="grid")
        
        # Horizontal lines
        for y in range(0, map_canvas_height + 1, GRID_SIZE):
            self.canvas.create_line(0, y, map_canvas_width, y, fill="gray", width=1, tags="grid")
            
    def draw_boundaries(self):
        """Draw map boundaries"""
        # Calculate actual map boundaries on canvas (considering zoom will be applied later)
        map_bottom_canvas = MAP_HEIGHT // CANVAS_SCALE
        map_right_canvas = MAP_WIDTH // CANVAS_SCALE
        
        # Bottom map boundary (red solid line)
        self.canvas.create_line(0, map_bottom_canvas, map_right_canvas, map_bottom_canvas, 
                              fill="red", width=3, tags="boundary")
        
        # Right map boundary (red solid line) 
        self.canvas.create_line(map_right_canvas, 0, map_right_canvas, map_bottom_canvas, 
                              fill="red", width=3, tags="boundary")
        
        # Top map boundary (red solid line)
        self.canvas.create_line(0, 0, map_right_canvas, 0, fill="red", width=3, tags="boundary")
        
        # Left map boundary (red solid line)
        self.canvas.create_line(0, 0, 0, map_bottom_canvas, fill="red", width=3, tags="boundary")
        
        # Boundary text
        self.canvas.create_text(map_right_canvas - 50, map_bottom_canvas - 10, text="Map Boundary", 
                              fill="red", font=("Arial", 8), tags="boundary")
            
    def change_mode(self):
        """Change drawing mode"""
        self.draw_mode = self.mode_var.get()
        
        # Hide all mode-specific frames first
        self.enemy_frame.pack_forget()
        self.spawn_frame.pack_forget()
        
        # Show appropriate frame based on mode
        if self.draw_mode == 'enemy':
            # Pack enemy frame below toolbar
            self.enemy_frame.pack(side=tk.TOP, fill=tk.X, padx=5, pady=2)
        elif self.draw_mode == 'spawn':
            # Pack spawn frame below toolbar
            self.spawn_frame.pack(side=tk.TOP, fill=tk.X, padx=5, pady=2)
        
    def change_block_type(self):
        """Change block type"""
        self.current_block_type = self.block_type_var.get()
        
    def change_brush_size(self, value):
        """Change brush size"""
        self.brush_size = int(value)
        
    def change_enemy_type(self, value):
        """Change enemy type"""
        self.current_enemy_type = value
        
    def change_enemy_difficulty(self, value):
        """Change enemy difficulty"""
        self.current_enemy_difficulty = int(value)
        
    def change_spawn_type(self, value):
        """Change spawn type"""
        self.current_spawn_type = value
        
    def on_mouse_wheel(self, event):
        """Handle mouse wheel events for scroll"""
        # Check modifier keys
        shift_pressed = event.state & 0x1  # Shift key
        
        if shift_pressed:
            # Shift + Scroll: Horizontal scroll
            self.handle_horizontal_scroll(event)
        else:
            # Normal scroll: Vertical scroll
            self.handle_vertical_scroll(event)
            

            

            
    def handle_horizontal_scroll(self, event):
        """Handle horizontal scroll with Shift+Scroll"""
        if event.delta > 0 or event.num == 4:
            self.canvas.xview_scroll(-1, "units")
        else:
            self.canvas.xview_scroll(1, "units")
            
    def handle_vertical_scroll(self, event):
        """Handle normal vertical scroll"""
        if event.delta > 0 or event.num == 4:
            self.canvas.yview_scroll(-1, "units")
        else:
            self.canvas.yview_scroll(1, "units")
            

        
    def canvas_to_map_coords(self, canvas_x, canvas_y):
        """Convert canvas coordinates to map coordinates"""
        # Consider scroll offset
        canvas_x = self.canvas.canvasx(canvas_x)
        canvas_y = self.canvas.canvasy(canvas_y)
        
        # Convert to actual map coordinates first
        actual_map_x = canvas_x * CANVAS_SCALE
        actual_map_y = canvas_y * CANVAS_SCALE
        
        # Align to BLOCK_SIZE boundaries in map coordinates
        # Use round() for more precise alignment
        map_x = round(actual_map_x / BLOCK_SIZE) * BLOCK_SIZE
        map_y = round(actual_map_y / BLOCK_SIZE) * BLOCK_SIZE
        
        # Ensure coordinates are within map bounds
        map_x = max(0, min(map_x, MAP_WIDTH - BLOCK_SIZE))
        map_y = max(0, min(map_y, MAP_HEIGHT - BLOCK_SIZE))
        
        # Convert back to grid coordinates for display
        grid_x = map_x // CANVAS_SCALE
        grid_y = map_y // CANVAS_SCALE
        
        return map_x, map_y, grid_x, grid_y
        
    def on_canvas_click(self, event):
        """Canvas click event"""
        self.handle_canvas_action(event.x, event.y)
        
    def on_canvas_drag(self, event):
        """Canvas drag event"""
        self.handle_canvas_action(event.x, event.y)
        
    def handle_canvas_action(self, canvas_x, canvas_y):
        """Handle drawing/erasing action on canvas"""
        center_map_x, center_map_y, center_grid_x, center_grid_y = self.canvas_to_map_coords(canvas_x, canvas_y)
        
        # Update coordinate display
        self.coord_label.config(text=f"Map: ({center_map_x}, {center_map_y}) | Grid: ({center_grid_x}, {center_grid_y})")
        
        # Process multiple blocks according to brush size
        brush_range = self.brush_size
        offset = (brush_range - 1) * BLOCK_SIZE // 2
        
        # Calculate brush boundaries
        brush_left = center_map_x - offset
        brush_right = brush_left + (brush_range * BLOCK_SIZE)
        brush_top = center_map_y - offset
        brush_bottom = brush_top + (brush_range * BLOCK_SIZE)
        
        # Check if entire brush area is within map boundaries
        if (brush_left < 0 or brush_right > MAP_WIDTH or 
            brush_top < 0 or brush_bottom > MAP_HEIGHT):
            return  # Don't draw if brush goes outside map
        
        blocks_changed = False
        
        for dx in range(brush_range):
            for dy in range(brush_range):
                # Calculate relative position from brush center
                map_x = center_map_x - offset + dx * BLOCK_SIZE
                map_y = center_map_y - offset + dy * BLOCK_SIZE
                
                # Additional safety check (should not be needed with above check, but just in case)
                if map_x < 0 or map_x >= MAP_WIDTH or map_y < 0 or map_y >= MAP_HEIGHT:
                    continue
                    
                if self.draw_mode == 'draw':
                    # Remove existing block (overwrite)
                    self.blocks = [b for b in self.blocks if not (b.x == map_x and b.y == map_y)]
                    
                    # Ensure perfect alignment
                    aligned_x = (map_x // BLOCK_SIZE) * BLOCK_SIZE
                    aligned_y = (map_y // BLOCK_SIZE) * BLOCK_SIZE
                    
                    # Add new block with aligned coordinates
                    block = Block(aligned_x, aligned_y, self.current_block_type)
                    self.blocks.append(block)
                    blocks_changed = True
                    
                elif self.draw_mode == 'erase':
                    # Remove block, enemy, and spawn point at this position
                    original_blocks_count = len(self.blocks)
                    original_enemies_count = len(self.enemies)
                    original_spawns_count = len(self.spawn_points)
                    
                    # Remove blocks
                    self.blocks = [b for b in self.blocks if not (b.x == map_x and b.y == map_y)]
                    
                    # Remove enemies
                    self.enemies = [e for e in self.enemies if not (e.x == map_x and e.y == map_y)]
                    
                    # Remove spawn points
                    self.spawn_points = [s for s in self.spawn_points if not (s.x == map_x and s.y == map_y)]
                    
                    # Check if anything was removed
                    if (len(self.blocks) != original_blocks_count or 
                        len(self.enemies) != original_enemies_count or
                        len(self.spawn_points) != original_spawns_count):
                        blocks_changed = True
                        
                elif self.draw_mode == 'enemy':
                    # Remove existing enemy at this position
                    self.enemies = [e for e in self.enemies if not (e.x == map_x and e.y == map_y)]
                    
                    # Add new enemy
                    enemy = Enemy(map_x, map_y, self.current_enemy_type, self.current_enemy_difficulty)
                    self.enemies.append(enemy)
                    blocks_changed = True
                    
                elif self.draw_mode == 'spawn':
                    # For tank spawn, only allow one spawn point
                    if self.current_spawn_type == 'tank':
                        # Remove existing tank spawn points
                        self.spawn_points = [s for s in self.spawn_points if s.spawn_type != 'tank']
                    
                    # Remove any existing spawn point at this position
                    self.spawn_points = [s for s in self.spawn_points if not (s.x == map_x and s.y == map_y)]
                    
                    # Add new spawn point
                    spawn = SpawnPoint(map_x, map_y, self.current_spawn_type)
                    self.spawn_points.append(spawn)
                    blocks_changed = True
        
        # Redraw canvas if there are changes
        if blocks_changed:
            self.redraw_canvas()
            
    def draw_block_on_canvas(self, grid_x, grid_y, color):
        """Draw block on canvas"""
        self.canvas.create_rectangle(grid_x, grid_y, 
                                   grid_x + GRID_SIZE, grid_y + GRID_SIZE,
                                   fill=color, outline="black", tags="block")
                                   
    def draw_enemy_on_canvas(self, grid_x, grid_y, enemy):
        """Draw enemy on canvas"""
        # Draw enemy circle
        center_x = grid_x + GRID_SIZE // 2
        center_y = grid_y + GRID_SIZE // 2
        radius = GRID_SIZE // 3
        
        self.canvas.create_oval(center_x - radius, center_y - radius,
                               center_x + radius, center_y + radius,
                               fill=enemy.get_color(), outline="black", tags="enemy")
        
        # Draw enemy text
        self.canvas.create_text(center_x, center_y, text=enemy.get_display_text(),
                               fill="white", font=("Arial", 8, "bold"), tags="enemy")
                               
    def draw_spawn_on_canvas(self, grid_x, grid_y, spawn):
        """Draw spawn point on canvas"""
        # Draw spawn point as a square
        center_x = grid_x + GRID_SIZE // 2
        center_y = grid_y + GRID_SIZE // 2
        size = GRID_SIZE // 2
        
        self.canvas.create_rectangle(center_x - size//2, center_y - size//2,
                                   center_x + size//2, center_y + size//2,
                                   fill=spawn.get_color(), outline="black", width=2, tags="spawn")
        
        # Draw spawn text
        self.canvas.create_text(center_x, center_y, text=spawn.get_display_text(),
                               fill="black", font=("Arial", 10, "bold"), tags="spawn")
                                   
    def redraw_canvas(self):
        """Redraw entire canvas"""
        # Clear only blocks, enemies, and spawn points, keep grid and boundaries
        self.canvas.delete("block")
        self.canvas.delete("enemy")
        self.canvas.delete("spawn")
        
        # Draw blocks
        for block in self.blocks:
            grid_x = block.x // CANVAS_SCALE
            grid_y = block.y // CANVAS_SCALE
            self.draw_block_on_canvas(grid_x, grid_y, block.get_color())
        
        # Draw enemies
        for enemy in self.enemies:
            grid_x = enemy.x // CANVAS_SCALE
            grid_y = enemy.y // CANVAS_SCALE
            self.draw_enemy_on_canvas(grid_x, grid_y, enemy)
            
        # Draw spawn points
        for spawn in self.spawn_points:
            grid_x = spawn.x // CANVAS_SCALE
            grid_y = spawn.y // CANVAS_SCALE
            self.draw_spawn_on_canvas(grid_x, grid_y, spawn)
            

            
    def load_map(self):
        """Load map, enemies, and spawn points"""
        stage_num = self.stage_var.get()
        
        # Load map blocks
        map_filename = f"stage{stage_num}.csv"
        map_filepath = os.path.join(OUTPUT_DIR, map_filename)
        
        # Load enemy data
        enemy_filename = f"enemies{stage_num}.csv"
        enemy_filepath = os.path.join(OUTPUT_DIR, enemy_filename)
        
        # Load spawn point data
        spawn_filename = f"spawns{stage_num}.csv"
        spawn_filepath = os.path.join(OUTPUT_DIR, spawn_filename)
        
        try:
            # Load blocks
            self.blocks = []
            if os.path.exists(map_filepath):
                with open(map_filepath, 'r', newline='') as f:
                    reader = csv.reader(f)
                    header = next(reader)  # Skip header
                    
                    for row in reader:
                        if len(row) >= 5:
                            block_type, start_x, start_y, end_x, end_y = row
                            start_x, start_y = int(float(start_x)), int(float(start_y))
                            
                            # Keep original block type (ground/grass)
                            if block_type in ["ground", "grass"]:
                                block = Block(start_x, start_y, block_type)
                            else:
                                # Treat other types as ground
                                block = Block(start_x, start_y, "ground")
                            self.blocks.append(block)
            
            # Load enemies
            self.enemies = []
            if os.path.exists(enemy_filepath):
                with open(enemy_filepath, 'r', newline='') as f:
                    reader = csv.reader(f)
                    header = next(reader)  # Skip header
                    
                    for row in reader:
                        if len(row) >= 4:
                            x, y, enemy_type, difficulty = row
                            x, y = int(float(x)), int(float(y))
                            difficulty = int(difficulty)
                            
                            enemy = Enemy(x, y, enemy_type, difficulty)
                            self.enemies.append(enemy)
                            
            # Load spawn points
            self.spawn_points = []
            if os.path.exists(spawn_filepath):
                with open(spawn_filepath, 'r', newline='') as f:
                    reader = csv.reader(f)
                    header = next(reader)  # Skip header
                    
                    for row in reader:
                        if len(row) >= 3:
                            x, y, spawn_type = row
                            x, y = int(float(x)), int(float(y))
                            
                            spawn = SpawnPoint(x, y, spawn_type)
                            self.spawn_points.append(spawn)
                            
            self.redraw_canvas()
            messagebox.showinfo("Success", f"Stage {stage_num} loaded successfully!\nBlocks: {len(self.blocks)}, Enemies: {len(self.enemies)}, Spawns: {len(self.spawn_points)}")
            
        except Exception as e:
            messagebox.showerror("Error", f"Error loading files: {e}")
            
    def save_map(self):
        """Save map, enemies, and spawn points"""
        stage_num = self.stage_var.get()
        
        try:
            # Save map blocks
            map_filename = f"stage{stage_num}.csv"
            map_filepath = os.path.join(OUTPUT_DIR, map_filename)
            
            with open(map_filepath, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["type", "start_x", "start_y", "end_x", "end_y"])
                
                # Save all blocks
                for block in self.blocks:
                    writer.writerow(block.to_csv_row())
            
            # Save enemy data
            enemy_filename = f"enemies{stage_num}.csv"
            enemy_filepath = os.path.join(OUTPUT_DIR, enemy_filename)
            
            with open(enemy_filepath, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["x", "y", "enemy_type", "difficulty"])
                
                # Save all enemies
                for enemy in self.enemies:
                    writer.writerow(enemy.to_csv_row())
                    
            # Save spawn point data
            spawn_filename = f"spawns{stage_num}.csv"
            spawn_filepath = os.path.join(OUTPUT_DIR, spawn_filename)
            
            with open(spawn_filepath, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["x", "y", "spawn_type"])
                
                # Save all spawn points
                for spawn in self.spawn_points:
                    writer.writerow(spawn.to_csv_row())
                    
            messagebox.showinfo("Success", f"Stage {stage_num} saved successfully!\nBlocks: {len(self.blocks)}, Enemies: {len(self.enemies)}, Spawns: {len(self.spawn_points)}\nLocation: {OUTPUT_DIR}")
            
        except Exception as e:
            messagebox.showerror("Error", f"Error saving files: {e}")
            
    def refresh_config(self):
        """Refresh configuration from config.ini and update UI"""
        global CONFIG, MAP_EDITOR_CONFIG, MAP_WIDTH, MAP_HEIGHT, BLOCK_SIZE, CANVAS_SCALE, CANVAS_WIDTH, CANVAS_HEIGHT, GRID_SIZE
        
        # Reload configurations
        CONFIG = load_config()
        MAP_EDITOR_CONFIG = load_map_editor_config()
        
        # Update global variables
        MAP_WIDTH = CONFIG['map_width']
        MAP_HEIGHT = CONFIG['map_height']
        BLOCK_SIZE = CONFIG['block_size']
        CANVAS_SCALE = MAP_EDITOR_CONFIG['canvas_scale']
        CANVAS_WIDTH = MAP_WIDTH // CANVAS_SCALE
        CANVAS_HEIGHT = MAP_HEIGHT // CANVAS_SCALE
        GRID_SIZE = BLOCK_SIZE // CANVAS_SCALE
        
        # Update info label
        self.info_label.config(text=f"Map: {MAP_WIDTH}x{MAP_HEIGHT} | Block: {BLOCK_SIZE}px | Scale: 1/{CANVAS_SCALE} | Buffer: {CONFIG['buffer_width']}x{CONFIG['buffer_height']}")
        
        # Remove old canvas frame and recreate
        for widget in self.root.winfo_children():
            if isinstance(widget, tk.Frame) and widget != self.root.winfo_children()[0]:  # Keep toolbar
                widget.destroy()
        
        # Recreate canvas with new dimensions
        self.setup_canvas()
        
        # Recreate default ground
        self.blocks = []
        self.enemies = []
        self.spawn_points = []
        self.create_default_ground()
        self.redraw_canvas()
        
        messagebox.showinfo("Config Refreshed", f"Configuration updated!\nMap: {MAP_WIDTH}x{MAP_HEIGHT}\nCanvas Scale: 1/{CANVAS_SCALE}")
    
    def create_default_ground(self):
        """Create ground blocks at default ground level"""
        # Set ground level (2-3 rows from bottom of map)
        ground_level_start = MAP_HEIGHT - BLOCK_SIZE * 3  # 3 blocks height from bottom
        ground_level_end = MAP_HEIGHT
        
        # Place ground blocks across entire map width
        # Ensure perfect alignment with BLOCK_SIZE boundaries
        for y in range(ground_level_start, ground_level_end, BLOCK_SIZE):
            for x in range(0, MAP_WIDTH, BLOCK_SIZE):
                # Ensure coordinates are perfectly aligned
                aligned_x = (x // BLOCK_SIZE) * BLOCK_SIZE
                aligned_y = (y // BLOCK_SIZE) * BLOCK_SIZE
                
                # Check if there's already a block at the same position
                if not any(b.x == aligned_x and b.y == aligned_y for b in self.blocks):
                    block = Block(aligned_x, aligned_y, "ground")
                    self.blocks.append(block)

def main():
    root = tk.Tk()
    app = MapEditor(root)
    root.mainloop()

if __name__ == "__main__":
    main()