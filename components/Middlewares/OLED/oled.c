#include "oled.h"

/**
 * @brief 计算幂值（m 的 n 次方）
 * @param m 底数
 * @param n 指数
 * @return uint32_t 计算结果
 * @note 实现原理：使用循环累乘的方式计算幂值
 * @note 此函数用于 OLED 显示数字时的位权计算
 */
static uint32_t OLED_Pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;

    while(n--)
        result*=m;

    return result;
}

/**
 * @brief 初始化 OLED 屏幕
 * @note 此函数用于初始化 OLED 屏幕，调用底层的 ssd1306_init() 函数
 * @note 实现原理：
 *       - 调用 ssd1306_init() 函数完成 SSD1306 OLED 控制器的初始化
 *       - 包括设置显示模式、对比度、扫描方向等参数
 * @return 无
 */
void OLED_Init(void)
{
    ssd1306_init();
}

/**
 * @brief 清空 OLED 屏幕显示
 * @note 此函数用于清空整个 OLED 屏幕的显示内容
 * @note 实现原理：
 *       1. 循环遍历 OLED 的 8 个页面（page 0-7）
 *       2. 对于每个页面，设置光标到起始位置（列 0）
 *       3. 向该页面的所有 128 列写入 0x00（空数据）
 * @return 无
 */
void OLED_Clear(void)
{
    for(uint8_t page=0;page<8;page++)
    {
        ssd1306_set_cursor(page,0);

        for(uint8_t col=0;col<128;col++)
        {
            ssd1306_write_data(0x00);
        }
    }
}

/**
 * @brief 在 OLED 屏幕上显示单个字符
 * @param x 字符的列坐标（0-127）
 * @param y 字符的页坐标（0-7）
 * @param chr 要显示的字符（ASCII 码）
 * @note 实现原理：
 *       1. 计算字符在字库中的索引：chr - ' '（空格的 ASCII 码）
 *       2. 设置光标位置到指定的页和列
 *       3. 从 Font6x8 字库中读取 6 个字节的字符数据并写入 OLED
 * @note 使用 6x8 大小的字体，每个字符占 6 列
 */
void OLED_ShowChar(uint8_t x,uint8_t y,char chr)
{
    uint8_t c=chr-' ';

    ssd1306_set_cursor(y,x);

    for(uint8_t i=0;i<6;i++)
    {
        ssd1306_write_data(OLED_F6x8[c][i]);
    }
}

/**
 * @brief 在 OLED 屏幕上显示字符串
 * @param x 字符串的起始列坐标（0-127）
 * @param y 字符串的起始页坐标（0-7）
 * @param str 要显示的字符串指针
 * @note 实现原理：
 *       1. 遍历字符串中的每个字符，直到遇到结束符 '\0'
 *       2. 调用 OLED_ShowChar 函数显示当前字符
 *       3. 每显示一个字符，x 坐标增加 6（因为每个字符占 6 列）
 *       4. 如果 x 坐标超过 122（接近屏幕右边界），则换行显示：
 *          - x 重置为 0
 *          - y 增加 1（切换到下一页）
 * @note 使用 6x8 大小的字体，每个字符占 6 列
 * @note 字符串过长时会自动换行显示
 */
void OLED_ShowString(uint8_t x,uint8_t y,char *str)
{
    while(*str)
    {
        OLED_ShowChar(x,y,*str);

        x+=6;

        if(x>122)
        {
            x=0;
            y++;
        }

        str++;
    }
}

/**
 * @brief 在 OLED 屏幕上显示数字
 * @param x 数字的起始列坐标（0-127）
 * @param y 数字的起始页坐标（0-7）
 * @param num 要显示的数字（0-4294967295）
 * @param len 数字的位数（1-10）
 * @note 实现原理：
 *       1. 循环 len 次，每次处理一位数字
 *       2. 计算当前位的数字值：num / 10^(len-i-1) % 10
 *       3. 将数字值转换为字符：+ '0'
 *       4. 调用 OLED_ShowChar 函数显示该字符，位置为 (x+i*6, y)
 * @note 使用 6x8 大小的字体，每个数字占 6 列
 * @note 数字位数不足时，会在前面显示空格
 */
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len)
{
    for(uint8_t i=0;i<len;i++)
    {
        OLED_ShowChar(
            x+i*6,
            y,
            num/OLED_Pow(10,len-i-1)%10+'0'
        );
    }
}