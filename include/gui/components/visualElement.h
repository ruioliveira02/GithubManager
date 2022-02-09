/**
 * @file visualElement.h
 * 
 * File containing declaration of the #VisualElement type
 */

#ifndef _VISUAL_ELEMENT_H_

/**
 * @brief Include guard
 */
#define _VISUAL_ELEMENT_H_

/**
 * @brief Enum used to describe all possible types of content
 */
typedef enum contentType {
    TEXT,   ///< Basic text
    BUTTON, ///< Button
    TITLE,  ///< Title
} ContentType;

/**
 * @brief A element to render to the screen
 * 
 */
typedef struct visualElement *VisualElement;

void setVEButtonSelected(VisualElement, bool);
void setVEText(VisualElement, char*);
VisualElement createVEWideText(wchar_t*);
VisualElement createVisualElement(ContentType, void*);
VisualElement copyVisualElement(VisualElement);
void renderVisualElement(VisualElement, int*, int*, int, int);
void freeVisualElement(VisualElement);

#endif