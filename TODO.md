# Xteink X4 Question Display - Development Plan

## Project Vision

An interactive e-ink question display device that shows random questions from different categories. Users can navigate through questions using physical buttons, with a clean UI featuring custom typography (Lexend font) and category icons.

## MVP Features (Phase 1)

### Core Functionality
- [ ] **Question Database System**
  - [ ] Create JSON/structured data format for 320 questions
  - [ ] Organize questions into categories (with names and icons)
  - [ ] Store question data in SPIFFS or SD card
  - [ ] Implement question loader/parser

- [ ] **Navigation System**
  - [ ] Next question (Right button)
  - [ ] Previous question (Left button)
  - [ ] Random question (Confirm button)
  - [ ] Next category (Volume Up button)
  - [ ] Previous category (Volume Down button)
  - [ ] Return to main menu (Back button)

- [ ] **Display & UI**
  - [ ] Design UI layout matching physical card design
  - [ ] Set display orientation to landscape mode (800×480, MVP only)
  - [ ] Integrate Lexend font (convert to GFX format)
  - [ ] Draw rounded rectangle border around question area
  - [ ] Create bird icon (category mascot, bottom left)
  - [ ] Create category icons (door/symbol, bottom right)
  - [ ] Draw category name banner (rounded filled rectangle)
  - [ ] Display question text centered with proper word wrapping
  - [ ] Display card number vertically on left edge (e.g., "002.350")
  - [ ] Add battery indicator at bottom center
  - [ ] Add visual feedback for button presses (optional)

- [ ] **State Management**
  - [ ] Track current question index
  - [ ] Track current category
  - [ ] Remember last viewed question (persist in SPIFFS)
  - [ ] Handle category boundaries (loop or stop)

### Technical Requirements
- [ ] **Memory Optimization**
  - [ ] Store fonts in PROGMEM
  - [ ] Store icons/graphics in PROGMEM
  - [ ] Lazy-load questions to avoid RAM issues (400KB limit!)
  - [ ] Profile memory usage during development

- [ ] **Power Management**
  - [ ] Implement deep sleep after inactivity timeout
  - [ ] Wake on button press
  - [ ] Battery level indicator on screen
  - [ ] Low battery warning

## Phase 2 - Enhanced Features

### Settings Menu
- [ ] Create settings UI
- [ ] Category whitelist/blacklist system
- [ ] Adjust text size
- [ ] Configure inactivity timeout
- [ ] Display brightness/contrast settings
- [ ] Reset progress/history

### Data Management
- [ ] Import questions from SD card
- [ ] Export viewed questions log
- [ ] Question history tracking
- [ ] Favorites/bookmarks system

### Visual Enhancements
- [ ] Add transitions between questions
- [ ] Category color themes
- [ ] Progress bar for category completion
- [ ] Achievement/milestone graphics

## Phase 3 - Multiplayer/Social Features

### Multi-Player Mode
- [ ] Turn-based question game
- [ ] Player score tracking
- [ ] Timer for answering
- [ ] Question reveal mechanics

### Connectivity (Future)
- [ ] WiFi sync for new question packs
- [ ] Cloud backup of progress
- [ ] Share favorite questions
- [ ] Leaderboards (if applicable)

## Data Structure Draft

### Question Format (JSON)
```json
{
  "categories": [
    {
      "id": 1,
      "name": "Philosophy",
      "icon": "philosophy_icon.h",
      "questions": [
        {
          "id": 1,
          "text": "What would you do if you knew you couldn't fail?",
          "difficulty": "easy"
        }
      ]
    }
  ]
}
```

## Font Integration

### Lexend Font Setup
- [ ] Download Lexend font from Google Fonts
- [ ] Convert TTF to Adafruit GFX format using fontconvert
- [ ] Generate multiple sizes (16pt, 24pt, 32pt)
- [ ] Store in header files (e.g., `LexendRegular16pt.h`)
- [ ] Test font rendering on e-ink display

Command reference:
```bash
# Example font conversion
fontconvert Lexend-Regular.ttf 24 32 127 > LexendRegular24pt.h
```

## UI/UX Design Considerations

### Screen Layout (Based on Physical Card Design)

**Display Orientation**: Landscape mode only (800×480px) for MVP.

The UI follows the physical card game design with:
- Rounded rectangle border around the question area
- Centered question text in the upper portion
- Bird icon at bottom left corner
- Category name in a banner/button style at the bottom
- Category icon/door symbol on the right side of banner
- Card number displayed vertically on the left edge

```
╔═════════════════════════════════════════════════════╗
║   ╭─────────────────────────────────────────────╮   ║
║   │                                             │   ║  ← Rounded border
║   │              LONG QUESTION HERE ?           │   ║  ← Question centered
║   │                                             │   ║
║   │   (Question text in                         │   ║
║   │    Lexend font)                             │   ║
║   │                                             │   ║
║   ╰─────────────────────────────────────────────╯   ║
║   ┌─────────────────────────────────────────────┐   ║  ← Bird + Banner + Door
║   │  LOGO                      CATEGORY NAME    │   ║  ← Category banner
║   └─────────────────────────────────────────────┘   ║
╚═════════════════════════════════════════════════════╝
         [Battery indicator]
```

### Visual Elements
- **Rounded border**: Draw rounded rectangle around question using GFX library
- **Bird icon**: Category mascot icon (bottom left, ~32-48px)
- **Category banner**: Filled rounded rectangle with category name
- **Door/icon**: Category symbol (bottom right, ~32-48px)
- **Card number**: Vertical text on left margin (e.g., "002.350")
- **Question text**: Centered, multi-line, Lexend font
- **Battery**: Small indicator at very bottom center

### Button Mapping
- **Back**: Main menu / Exit
- **Confirm**: Random question
- **Left**: Previous question
- **Right**: Next question
- **Volume Down**: Previous category
- **Volume Up**: Next category
- **Power (long press)**: Sleep/Wake

## Development Priority

1. ✅ Hardware abstraction (BatteryMonitor, ButtonHandler, DisplayManager, PowerManager) - DONE
2. 🎯 Question data structure and storage
3. 🎯 Font integration (Lexend)
4. 🎯 Basic question display UI
5. 🎯 Navigation implementation
6. 🎯 State persistence
7. Category system
8. Settings menu
9. Power optimization
10. Polish and refinements

## Memory Budget Planning

| Component | Estimated RAM | Notes |
|-----------|---------------|-------|
| Display buffer | ~48KB | GxEPD2 internal buffer |
| Question text | ~2KB | Current question only |
| Font data | ~20-40KB | PROGMEM (not RAM) |
| Icons/graphics | ~10KB | PROGMEM (not RAM) |
| State/variables | ~5KB | Navigation state |
| Stack reserve | ~50KB | Safety margin |
| **Total usable** | **~400KB** | ESP32-C3 limit |

## Testing Checklist

- [ ] Test all button combinations
- [ ] Test category navigation boundaries
- [ ] Test random question generation (no duplicates in sequence)
- [ ] Test deep sleep and wake
- [ ] Test low battery scenarios
- [ ] Test question text wrapping with long text
- [ ] Test SPIFFS read/write operations
- [ ] Memory leak testing (long-term operation)
- [ ] E-ink ghosting prevention

## Resources Needed

- [ ] Lexend font files (Google Fonts)
- [ ] Bird icon graphic (32-48px, black & white)
- [ ] Category door/icon graphics (32-48px per category, black & white)
- [ ] 320 questions organized by category (Czech language)
- [ ] Category names and translations
- [ ] Adafruit GFX fontconvert tool
- [ ] Reference card designs for each category

## Known Constraints

- **Memory**: 400KB RAM, no PSRAM - keep heap usage minimal
- **Display refresh**: ~2 seconds per update - minimize refreshes
- **E-ink lifespan**: Prefer full refreshes, limit partial updates
- **Font size**: Balance readability vs. screen space
- **Button latency**: ADC read delays ~50ms for debouncing

## Future Ideas (Backlog)

- Multiple language support
- Question difficulty levels
- Daily question notification
- Question of the day mode
- Social sharing (QR code generation)
- Custom question creation via web interface
- Statistics dashboard (most viewed categories, etc.)
