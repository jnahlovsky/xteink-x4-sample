import csv
import os

# Configuration
CSV_FILE = 'cards.csv'
OUTPUT_FILE = 'src/Questions.h'

def convert():
    if not os.path.exists(CSV_FILE):
        print(f"Error: {CSV_FILE} not found!")
        return

    categories_data = []
    questions_data = []

    cat_map = {
        "Emotions": "EMOCE",
        "Fantasy": "FANTAZIE",
        "Relationships": "VZTAHY",
        "MyBody": "MOJE TĚLO",
        "School": "ŠKOLA / PRÁCE",
        "Nature": "PŘÍRODA",
        "WhatIf": "CO KDYBY",
        "Secrets": "TAJEMSTVÍ"
    }

    cat_enum_names = list(cat_map.keys())

    with open(CSV_FILE, 'r', encoding='utf-8') as f:
        # Read the whole file to handle potential issues with newlines inside quoted fields
        reader = csv.reader(f)
        next(reader) # skip header

        for row in reader:
            if not row or len(row) < 2: continue

            # Parse category (e.g., "Category=Emotions")
            cat_raw = row[0]
            if '=' in cat_raw:
                cat_id = cat_raw.split('=')[1].strip()
            else:
                cat_id = cat_raw.strip()

            # Clean up text
            text = row[1]

            # 1. Handle non-breakable spaces (\xa0) -> convert to regular space
            text = text.replace('\xa0', ' ')

            # 2. Handle extra line breaks -> convert to \n
            # CSV reader handles quoted newlines, so we just replace them
            text = text.replace('\r\n', '\n').replace('\r', '\n')
            text = text.replace('\n', '\\n')

            # 3. Escape double quotes for C++
            text = text.replace('"', '\\"')

            # 4. Trim extra spaces
            text = text.strip()

            if cat_id in cat_map:
                questions_data.append((text, cat_id))
            else:
                print(f"Warning: Unknown category '{cat_id}' for question: {text[:30]}...")

    # Generate header file
    header = """#ifndef QUESTIONS_H
#define QUESTIONS_H

#include <Arduino.h>

// Category enum for memory efficiency
enum Category {
"""
    for cat in cat_enum_names:
        header += f"    CAT_{cat.upper()},\n"
    header += """    CAT_COUNT
};

// Category display names mapping
const char* const CATEGORY_NAMES[] PROGMEM = {
"""
    for cat in cat_enum_names:
        header += f'    "{cat_map[cat]}",\n'
    header += """};

// Question data structure
struct Question
{
  const char *text;
  Category category;
};

// =============================================================================
// QUESTION DATABASE
// =============================================================================
const Question QUESTIONS[] PROGMEM = {
"""

    questions_list = []
    for text, cat_id in questions_data:
        questions_list.append(f'    {{"{text}", CAT_{cat_id.upper()}}}')

    footer = """
const int QUESTION_COUNT = sizeof(QUESTIONS) / sizeof(QUESTIONS[0]);

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

// Get question text by index
inline const char *getQuestionText(int index)
{
  if (index < 0 || index >= QUESTION_COUNT)
    return "";
  return (const char*)pgm_read_ptr(&QUESTIONS[index].text);
}

// Get category name by index
inline const char *getQuestionCategory(int index)
{
  if (index < 0 || index >= QUESTION_COUNT)
    return "";
  Category cat = (Category)pgm_read_word(&QUESTIONS[index].category);
  return (const char*)pgm_read_ptr(&CATEGORY_NAMES[cat]);
}

// Get total number of questions
inline int getQuestionCount()
{
  return QUESTION_COUNT;
}

#endif // QUESTIONS_H
"""

    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        f.write(header)
        f.write(',\n'.join(questions_list))
        f.write('\n};')
        f.write(footer)

    print(f"Successfully converted {len(questions_data)} questions to {OUTPUT_FILE}")

if __name__ == "__main__":
    convert()
