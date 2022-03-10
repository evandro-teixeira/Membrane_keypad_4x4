/**
 * @file MatrixKeyboard.hpp
 * @author Evandro Teixeira
 * @brief 
 * @version 0.1
 * @date 06-03-2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>

/**
 * @brief 
 */
enum
{
    Key3x3 = 0,
    Key4x3,
    Key3x4,
    Key4x4,
    KeyMAX
};

/**
 * @brief 
 */
typedef struct 
{
    uint8_t line1;
    uint8_t line2;
    uint8_t line3;
    uint8_t line4;
    uint8_t column1;
    uint8_t column2;
    uint8_t column3;
    uint8_t column4;
}KeyboardPin_t;

/**
 * @brief 
 */
typedef union 
{
    uint16_t data;
    struct 
    {
        unsigned key1 : 1;
        unsigned key2 : 1;
        unsigned key3 : 1;
        unsigned key4 : 1;
        unsigned key5 : 1;
        unsigned key6 : 1;
        unsigned key7 : 1;
        unsigned key8 : 1;
        unsigned key9 : 1;
        unsigned key10 : 1;
        unsigned key11 : 1;
        unsigned key12 : 1;
        unsigned key13 : 1;
        unsigned key14 : 1;
        unsigned key15 : 1;
        unsigned key16 : 1;
    }byte;
}Key_t;


/**
 * @brief 
 */
class MatrixKeyboard
{
private:
    uint8_t KeyboardType;
    Key_t Key;
    uint8_t LinePins[4];
    uint8_t ColumnPins[4];
    char chKey;
public:
    MatrixKeyboard(uint8_t type);
    ~MatrixKeyboard();
    void Init(KeyboardPin_t Pins);
    void Run(void);
    Key_t Read(void);
    char GetCharKey(void);
};

