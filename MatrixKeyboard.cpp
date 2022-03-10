/**
 * @file MatrixKeyboard.cpp
 * @author Evandro Teixeira
 * @brief 
 * @version 0.1
 * @date 06-03-2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "MatrixKeyboard.hpp"

typedef struct
{
    uint8_t Line;
    uint8_t Column;
}KeyPin_t;

/**
 * @brief integer key values
 * @note
 *      [0x0001][0x0002][0x0004][0x0008]
 *      [0x0010][0x0020][0x0040][0x0080]
 *      [0x0100][0x0200][0x0400][0x0800]
 *      [0x1000][0x2000][0x4000][0x8000]
 */
const uint16_t IntegerKeyValues[KeyMAX][KeyMAX] = 
{
    {0x0001,0x0002,0x0004,0x0008},
    {0x0010,0x0020,0x0040,0x0080},
    {0x0100,0x0200,0x0400,0x0800},
    {0x1000,0x2000,0x4000,0x8000}
};

/**
 * @brief char key values
 * @note 
 *      [1][2][3][A]    
 *      [4][5][6][B]
 *      [7][8][9][C]
 *      [*][0][#][D]
 */
const char CharKeyValues[KeyMAX][KeyMAX] = 
{
   {'1','4','7','*'},
   {'2','5','8','0'},
   {'3','6','9','#'},
   {'A','B','C','D'},
};

/**
 * @brief 
 */
const KeyPin_t ParamType[KeyMAX] = {{3,3},{4,3},{3,4},{4,4}};

/**
 * @brief 
 * 
 */
MatrixKeyboard::MatrixKeyboard(uint8_t type) : KeyboardType(type)
{
}

/**
 * @brief Destroy the Matrix Keyboard:: Matrix Keyboard object
 * 
 */
MatrixKeyboard::~MatrixKeyboard()
{
}

/**
 * @brief 
 * 
 */
void MatrixKeyboard::Init(KeyboardPin_t Pins)
{
    uint8_t i = 0;

    LinePins[0] = Pins.line1;
    LinePins[1] = Pins.line2;
    LinePins[2] = Pins.line3;
    ColumnPins[0] = Pins.column1;
    ColumnPins[1] = Pins.column2;
    ColumnPins[2] = Pins.column3;

    switch (KeyboardType)
    {
        case Key4x3:
            LinePins[3] = Pins.line4;
        case Key3x4:
            ColumnPins[3] = Pins.column4;
        case Key4x4:
            LinePins[3] = Pins.line4;
            ColumnPins[3] = Pins.column4;
        break;
        case Key3x3:
        default:
        break;
    }

    for(i=0;i<ParamType[KeyboardType].Line;i++)
    {
        pinMode(LinePins[i], INPUT_PULLDOWN);
    }

    for(i=0;i<ParamType[KeyboardType].Column;i++)
    {
        pinMode(ColumnPins[i], OUTPUT);
    }

    Key.data = 0;
}

/**
 * @brief 
 * 
 */
void MatrixKeyboard::Run(void)
{
    uint8_t i = 0;
    static uint8_t indexColumn = 0;
    static uint8_t value = 0x01;

    value = 1 << indexColumn;

    digitalWrite(ColumnPins[0], (value & 0x01) >> 0);
    digitalWrite(ColumnPins[1], (value & 0x02) >> 1);
    digitalWrite(ColumnPins[2], (value & 0x04) >> 2);
    if(KeyboardType == Key3x4 || KeyboardType == Key4x4)
    {
        digitalWrite(ColumnPins[3], (value & 0x08) >> 3);
    }

    for(i=0;i<ParamType[KeyboardType].Line;i++)
    {
        delayMicroseconds(500);

        if(digitalRead(LinePins[i]) == HIGH)
        {
            Key.data |= IntegerKeyValues[indexColumn][i];
            chKey = CharKeyValues[indexColumn][i];
        }
        else
        {
            Key.data &=~ IntegerKeyValues[indexColumn][i];
        }
    }

    indexColumn++;
    if(indexColumn > ParamType[KeyboardType].Column) 
    {
        indexColumn = 0;
        value = 0x01;
    }
}

/**
 * @brief 
 * 
 * @return key_t 
 */
Key_t MatrixKeyboard::Read(void)
{
    return Key;
}

/**
 * @brief 
 * 
 * @return char 
 */
char MatrixKeyboard::GetCharKey(void)
{
    return chKey;
}