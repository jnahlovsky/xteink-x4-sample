#ifndef UTF8_GFX_HELPER_H
#define UTF8_GFX_HELPER_H

#include <Adafruit_GFX.h>

/**
 * UTF-8 aware text printing for Adafruit GFX with extended character fonts.
 * Decodes UTF-8 multi-byte sequences into Unicode codepoints and draws
 * them using the font's glyph table.
 *
 * Requires fonts generated with extended range (e.g., 32-383 for Czech).
 */

/**
 * Decode a single UTF-8 character and return the Unicode codepoint.
 *
 * @param str Pointer to UTF-8 string
 * @param codepoint Output: decoded Unicode codepoint
 * @return Number of bytes consumed (1-4), or 1 if invalid
 */
inline uint8_t decodeUtf8Char(const char *str, uint16_t &codepoint)
{
  uint8_t c = (uint8_t)*str;

  // Single byte (ASCII): 0xxxxxxx
  if ((c & 0x80) == 0)
  {
    codepoint = c;
    return 1;
  }

  // Two bytes: 110xxxxx 10xxxxxx (covers Latin Extended: 0x80-0x7FF)
  if ((c & 0xE0) == 0xC0)
  {
    uint8_t c2 = (uint8_t)str[1];
    if ((c2 & 0xC0) == 0x80)
    {
      codepoint = ((c & 0x1F) << 6) | (c2 & 0x3F);
      return 2;
    }
  }

  // Three bytes: 1110xxxx 10xxxxxx 10xxxxxx (covers 0x800-0xFFFF)
  if ((c & 0xF0) == 0xE0)
  {
    uint8_t c2 = (uint8_t)str[1];
    uint8_t c3 = (uint8_t)str[2];
    if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80)
    {
      codepoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
      return 3;
    }
  }

  // Invalid or unsupported UTF-8, return replacement char
  codepoint = '?';
  return 1;
}

/**
 * Draw a UTF-8 encoded string character by character.
 * This function properly handles Czech diacritics and other Unicode characters.
 *
 * @param gfx The Adafruit_GFX display object
 * @param font The GFXfont to use (must include the required Unicode range)
 * @param str UTF-8 encoded null-terminated string
 * @param x Starting X position
 * @param y Starting Y position (baseline)
 * @param color Text color
 * @param lineHeight Line height for newlines (0 = use font yAdvance)
 */
inline void drawUtf8String(Adafruit_GFX &gfx, const GFXfont *font, const char *str,
                           int16_t x, int16_t y, uint16_t color, uint16_t lineHeight = 0)
{
  if (!font || !str)
    return;

  gfx.setFont(font);
  gfx.setTextColor(color);

  int16_t cursorX = x;
  int16_t cursorY = y;
  int16_t startX = x;

  // Get font metrics
  uint16_t fontFirst = pgm_read_word(&font->first);
  uint16_t fontLast = pgm_read_word(&font->last);
  uint8_t yAdvance = pgm_read_byte(&font->yAdvance);

  if (lineHeight == 0)
  {
    lineHeight = yAdvance;
  }

  while (*str)
  {
    uint16_t codepoint;
    uint8_t bytes = decodeUtf8Char(str, codepoint);
    str += bytes;

    // Handle newline
    if (codepoint == '\n')
    {
      cursorX = startX;
      cursorY += lineHeight;
      continue;
    }

    // Skip carriage return
    if (codepoint == '\r')
    {
      continue;
    }

    // Check if character is in font range
    if (codepoint >= fontFirst && codepoint <= fontLast)
    {
      // Get glyph data
      uint16_t glyphIndex = codepoint - fontFirst;
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_ptr(&font->glyph))[glyphIndex]);

      uint8_t w = pgm_read_byte(&glyph->width);
      uint8_t h = pgm_read_byte(&glyph->height);
      int8_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
      int8_t yo = (int8_t)pgm_read_byte(&glyph->yOffset);
      uint8_t xa = pgm_read_byte(&glyph->xAdvance);

      if (w > 0 && h > 0)
      {
        // Draw the character bitmap
        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t *bitmap = (uint8_t *)pgm_read_ptr(&font->bitmap);

        uint8_t bit = 0;
        uint8_t bits = 0;

        for (uint8_t yy = 0; yy < h; yy++)
        {
          for (uint8_t xx = 0; xx < w; xx++)
          {
            if (!(bit++ & 7))
            {
              bits = pgm_read_byte(&bitmap[bo++]);
            }
            if (bits & 0x80)
            {
              gfx.drawPixel(cursorX + xo + xx, cursorY + yo + yy, color);
            }
            bits <<= 1;
          }
        }
      }

      // Advance cursor
      cursorX += xa;
    }
    else
    {
      // Character not in font, draw a placeholder or skip
      // Skip for now
    }
  }
}

/**
 * Get the pixel width of a UTF-8 encoded string.
 * Useful for centering text.
 */
inline uint16_t getUtf8StringWidth(const GFXfont *font, const char *str)
{
  if (!font || !str)
    return 0;

  uint16_t fontFirst = pgm_read_word(&font->first);
  uint16_t fontLast = pgm_read_word(&font->last);
  uint16_t width = 0;

  while (*str)
  {
    uint16_t codepoint;
    uint8_t bytes = decodeUtf8Char(str, codepoint);
    str += bytes;

    // Stop at newline for single-line width
    if (codepoint == '\n')
      break;
    if (codepoint == '\r')
      continue;

    if (codepoint >= fontFirst && codepoint <= fontLast)
    {
      uint16_t glyphIndex = codepoint - fontFirst;
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_ptr(&font->glyph))[glyphIndex]);
      width += pgm_read_byte(&glyph->xAdvance);
    }
  }

  return width;
}

/**
 * Draw a centered UTF-8 string.
 */
inline void drawUtf8StringCentered(Adafruit_GFX &gfx, const GFXfont *font, const char *str,
                                   int16_t centerX, int16_t y, uint16_t color)
{
  uint16_t w = getUtf8StringWidth(font, str);
  drawUtf8String(gfx, font, str, centerX - w / 2, y, color);
}

/**
 * Get the pixel width of a single line from a UTF-8 string (stops at \n or \0).
 * Does NOT modify the string.
 */
inline uint16_t getUtf8LineWidth(const GFXfont *font, const char *str, const char **nextLine)
{
  if (!font || !str)
  {
    if (nextLine)
      *nextLine = str;
    return 0;
  }

  uint16_t fontFirst = pgm_read_word(&font->first);
  uint16_t fontLast = pgm_read_word(&font->last);
  uint16_t width = 0;

  const char *p = str;
  while (*p && *p != '\n')
  {
    uint16_t codepoint;
    uint8_t bytes = decodeUtf8Char(p, codepoint);
    p += bytes;

    if (codepoint == '\r')
      continue;

    if (codepoint >= fontFirst && codepoint <= fontLast)
    {
      uint16_t glyphIndex = codepoint - fontFirst;
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_ptr(&font->glyph))[glyphIndex]);
      width += pgm_read_byte(&glyph->xAdvance);
    }
  }

  // Set next line pointer (skip the \n if present)
  if (nextLine)
  {
    *nextLine = (*p == '\n') ? (p + 1) : p;
  }

  return width;
}

/**
 * Draw multi-line UTF-8 text centered both horizontally and vertically within a rectangle.
 * Each line is center-aligned.
 *
 * @param gfx The Adafruit_GFX display object
 * @param font The GFXfont to use
 * @param str UTF-8 encoded text (can contain \n for line breaks)
 * @param centerX Center X coordinate of the rectangle
 * @param centerY Center Y coordinate of the rectangle
 * @param color Text color
 */
inline void drawUtf8MultiLineCentered(Adafruit_GFX &gfx, const GFXfont *font, const char *str,
                                      int16_t centerX, int16_t centerY, uint16_t color)
{
  if (!font || !str || *str == '\0')
    return;

  // Get font metrics
  uint8_t yAdvance = pgm_read_byte(&font->yAdvance);
  uint16_t fontFirst = pgm_read_word(&font->first);
  uint16_t fontLast = pgm_read_word(&font->last);

  // Count lines
  int lineCount = 1;
  for (const char *p = str; *p; p++)
  {
    if (*p == '\n')
      lineCount++;
  }

  // Calculate vertical start position
  int16_t totalHeight = lineCount * yAdvance;
  int16_t currentY = centerY - totalHeight / 2 + yAdvance; // +yAdvance because y is baseline

  // Process each line
  const char *lineStart = str;

  while (*lineStart)
  {
    // Find line width and next line pointer
    const char *nextLine;
    uint16_t lineWidth = getUtf8LineWidth(font, lineStart, &nextLine);

    // Calculate starting X for this line (centered)
    int16_t cursorX = centerX - lineWidth / 2;

    // Draw characters in this line
    const char *p = lineStart;
    while (*p && *p != '\n')
    {
      uint16_t codepoint;
      uint8_t bytes = decodeUtf8Char(p, codepoint);
      p += bytes;

      if (codepoint == '\r')
        continue;

      if (codepoint >= fontFirst && codepoint <= fontLast)
      {
        uint16_t glyphIndex = codepoint - fontFirst;
        GFXglyph *glyph = &(((GFXglyph *)pgm_read_ptr(&font->glyph))[glyphIndex]);

        uint8_t w = pgm_read_byte(&glyph->width);
        uint8_t h = pgm_read_byte(&glyph->height);
        int8_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
        int8_t yo = (int8_t)pgm_read_byte(&glyph->yOffset);
        uint8_t xa = pgm_read_byte(&glyph->xAdvance);

        if (w > 0 && h > 0)
        {
          uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
          uint8_t *bitmap = (uint8_t *)pgm_read_ptr(&font->bitmap);

          uint8_t bit = 0;
          uint8_t bits = 0;

          for (uint8_t yy = 0; yy < h; yy++)
          {
            for (uint8_t xx = 0; xx < w; xx++)
            {
              if (!(bit++ & 7))
              {
                bits = pgm_read_byte(&bitmap[bo++]);
              }
              if (bits & 0x80)
              {
                gfx.drawPixel(cursorX + xo + xx, currentY + yo + yy, color);
              }
              bits <<= 1;
            }
          }
        }

        cursorX += xa;
      }
    }

    // Move to next line
    currentY += yAdvance;
    lineStart = nextLine;
  }
}

#endif // UTF8_GFX_HELPER_H
