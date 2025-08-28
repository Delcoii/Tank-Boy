import csv
import os
import tkinter as tk
from tkinter import messagebox, filedialog, simpledialog
import math

# ===== Configuration =====
MAP_WIDTH = 12800
MAP_HEIGHT = 720*3
BLOCK_SIZE = 50  # Size of one block (pixels)

# GUI settings
CANVAS_SCALE = 5  # Canvas display scale (1/10 of actual size)
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

class MapEditor:
    def __init__(self, root):
        self.root = root
        self.root.title("Tank Boy Map Editor")
        self.root.geometry("1400x800")
        
        # Store block data
        self.blocks = []
        self.current_stage = 1
        
        # Current drawing mode ('draw' or 'erase')
        self.draw_mode = 'draw'
        
        # Current block type ('ground' or 'grass')
        self.current_block_type = 'ground'
        
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
        
        # Brush size selection
        tk.Label(toolbar, text="Brush Size:").pack(side=tk.LEFT)
        self.brush_var = tk.StringVar(value="1")
        brush_sizes = ["1", "2", "3", "4", "5"]
        brush_combo = tk.OptionMenu(toolbar, self.brush_var, *brush_sizes, command=self.change_brush_size)
        brush_combo.pack(side=tk.LEFT, padx=5)
        
        # Information display
        self.info_label = tk.Label(toolbar, text=f"Map: {MAP_WIDTH}x{MAP_HEIGHT} | Block Size: {BLOCK_SIZE}px | Scale: 1/{CANVAS_SCALE}")
        self.info_label.pack(side=tk.RIGHT, padx=10)
        

        
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
                               width=min(map_canvas_width, 1200), 
                               height=min(map_canvas_height, 600),
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
        
    def change_block_type(self):
        """Change block type"""
        self.current_block_type = self.block_type_var.get()
        
    def change_brush_size(self, value):
        """Change brush size"""
        self.brush_size = int(value)
        
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
        
        # Align to grid
        grid_x = int(canvas_x // GRID_SIZE) * GRID_SIZE
        grid_y = int(canvas_y // GRID_SIZE) * GRID_SIZE
        
        # Convert to actual map coordinates
        map_x = grid_x * CANVAS_SCALE
        map_y = grid_y * CANVAS_SCALE
        
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
        
        # Process multiple blocks according to brush size
        brush_range = self.brush_size
        offset = (brush_range - 1) * BLOCK_SIZE // 2
        
        blocks_changed = False
        
        for dx in range(brush_range):
            for dy in range(brush_range):
                # Calculate relative position from brush center
                map_x = center_map_x - offset + dx * BLOCK_SIZE
                map_y = center_map_y - offset + dy * BLOCK_SIZE
                
                # Check map range
                if map_x < 0 or map_x >= MAP_WIDTH or map_y < 0 or map_y >= MAP_HEIGHT:
                    continue
                    
                if self.draw_mode == 'draw':
                    # Remove existing block (overwrite)
                    self.blocks = [b for b in self.blocks if not (b.x == map_x and b.y == map_y)]
                    
                    # Add new block
                    block = Block(map_x, map_y, self.current_block_type)
                    self.blocks.append(block)
                    blocks_changed = True
                    
                elif self.draw_mode == 'erase':
                    # Remove block
                    original_count = len(self.blocks)
                    self.blocks = [b for b in self.blocks if not (b.x == map_x and b.y == map_y)]
                    if len(self.blocks) != original_count:
                        blocks_changed = True
        
        # Redraw canvas if there are changes
        if blocks_changed:
            self.redraw_canvas()
            
    def draw_block_on_canvas(self, grid_x, grid_y, color):
        """Draw block on canvas"""
        self.canvas.create_rectangle(grid_x, grid_y, 
                                   grid_x + GRID_SIZE, grid_y + GRID_SIZE,
                                   fill=color, outline="black", tags="block")
                                   
    def redraw_canvas(self):
        """Redraw entire canvas"""
        # Clear only blocks, keep grid and boundaries
        self.canvas.delete("block")
        for block in self.blocks:
            grid_x = block.x // CANVAS_SCALE
            grid_y = block.y // CANVAS_SCALE
            self.draw_block_on_canvas(grid_x, grid_y, block.get_color())
            

            
    def load_map(self):
        """Load map"""
        stage_num = self.stage_var.get()
        filename = f"stage{stage_num}.csv"
        filepath = os.path.join(OUTPUT_DIR, filename)
        
        if not os.path.exists(filepath):
            messagebox.showerror("Error", f"Stage {stage_num} file does not exist.")
            return

        try:
            self.blocks = []
            with open(filepath, 'r', newline='') as f:
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
                            
            self.redraw_canvas()
            messagebox.showinfo("Success", f"Stage {stage_num} loaded successfully!")
            
        except Exception as e:
            messagebox.showerror("Error", f"Error loading file: {e}")
            
    def save_map(self):
        """Save map"""
        stage_num = self.stage_var.get()
        filename = f"stage{stage_num}.csv"
        filepath = os.path.join(OUTPUT_DIR, filename)
        
        try:
            with open(filepath, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["type", "start_x", "start_y", "end_x", "end_y"])
                
                # Save all blocks
                for block in self.blocks:
                    writer.writerow(block.to_csv_row())
                    
            messagebox.showinfo("Success", f"Stage {stage_num} saved successfully!\nLocation: {filepath}")
            
        except Exception as e:
            messagebox.showerror("Error", f"Error saving file: {e}")
            
    def create_default_ground(self):
        """Create ground blocks at default ground level"""
        # Set ground level (2-3 rows from bottom of map)
        ground_level_start = MAP_HEIGHT - BLOCK_SIZE * 3  # 3 blocks height from bottom
        ground_level_end = MAP_HEIGHT
        
        # Place ground blocks across entire map width
        for y in range(ground_level_start, ground_level_end, BLOCK_SIZE):
            for x in range(0, MAP_WIDTH, BLOCK_SIZE):
                # Check if there's already a block at the same position
                if not any(b.x == x and b.y == y for b in self.blocks):
                    block = Block(x, y, "ground")
                    self.blocks.append(block)

def main():
    root = tk.Tk()
    app = MapEditor(root)
    root.mainloop()

if __name__ == "__main__":
    main()