#include "oled.h"
#include "oled_font.h"
#include "ssd1306.h"

// 屏幕缓冲区，用于跟踪屏幕状态
static uint8_t oled_buffer[8][128] = {0};

/**
 * @brief 计算幂值（m 的 n 次方）
 * @param m 底数
 * @param n 指数
 * @return uint32_t 计算结果
 * @note 实现原理：使用循环累乘的方式计算幂值
 * @note 此函数用于 OLED 显示数字时的位权计算
 */
static uint32_t OLED_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--)
        result *= m;

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
    i2c_bus_init();
    ssd1306_init();
    OLED_Clear();
}

/**
 * @brief 清空 OLED 屏幕显示
 * @note 此函数用于清空整个 OLED 屏幕的显示内容
 * @note 实现原理：
 *       1. 循环遍历 OLED 的 8 个页面（page 0-7）
 *       2. 对于每个页面，设置光标到起始位置（列 0）
 *       3. 向该页面的所有 128 列写入 0x00（空数据）
 *       4. 同时清空屏幕缓冲区
 * @return 无
 */
void OLED_Clear(void)
{
    for (uint8_t page = 0; page < 8; page++)
    {
        ssd1306_set_cursor(page, 0);

        for (uint8_t col = 0; col < 128; col++)
        {
            ssd1306_write_data(0x00);
            oled_buffer[page][col] = 0;  // 清空缓冲区
        }
    }
}

void OLED_PowerDown(void)
{
    ssd1306_write_cmd(0xAE); 
}

void OLED_PowerOn(void)
{
    ssd1306_write_cmd(0xAF); 
}
/**
 * @brief 开启 OLED 显示
 * @return 无
 */
void OLED_ON(void)
{
    ssd1306_write_cmd(0xAF); // 发送开启显示命令
}

/**
 * @brief 关闭 OLED 显示
 * @return 无
 */
void OLED_OFF(void)
{
    ssd1306_write_cmd(0xAE); // 发送关闭显示命令
}

/**
 * @brief 在 OLED 屏幕上显示单个字符（支持不同字号）
 * @param x 字符的列坐标（0-127）
 * @param y 字符的页坐标（0-7）
 * @param chr 要显示的字符（ASCII 码）
 * @param size 字体大小（8 或 16）
 * @note 实现原理：
 *       1. 计算字符在字库中的索引：chr - ' '（空格的 ASCII 码）
 *       2. 根据字体大小选择不同的字库
 *       3. 设置光标位置并写入字符数据
 * @note size = 8: 使用 6x8 字体，每个字符占 6 列
 * @note size = 16: 使用 8x16 字体，每个字符占 8 列，2 页高度
 */
void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size)
{
    uint8_t c = chr - ' ';
    
    if (size == 8)
    {
        // 使用 6x8 字体
        ssd1306_set_cursor(y, x);
        for (uint8_t i = 0; i < 6; i++)
        {
            ssd1306_write_data(OLED_F6x8[c][i]);
        }
    }
    else if (size == 16)
    {
        // 使用 8x16 字体
        ssd1306_set_cursor(y, x);
        for (uint8_t i = 0; i < 8; i++)
        {
            ssd1306_write_data(OLED_F8x16[c][i]);
        }
        ssd1306_set_cursor(y + 1, x);
        for (uint8_t i = 8; i < 16; i++)
        {
            ssd1306_write_data(OLED_F8x16[c][i]);
        }
    }
}

/**
 * @brief 在 OLED 屏幕上显示字符串（支持不同字号）
 * @param x 字符串的起始列坐标（0-127）
 * @param y 字符串的起始页坐标（0-7）
 * @param str 要显示的字符串指针
 * @param size 字体大小（8 或 16）
 * @note 实现原理：
 *       1. 遍历字符串中的每个字符，直到遇到结束符 '\0'
 *       2. 调用 OLED_ShowChar 函数显示当前字符
 *       3. 根据字体大小调整字符间距和换行逻辑
 * @note size = 8: 每个字符占 6 列，1 页高度
 * @note size = 16: 每个字符占 8 列，2 页高度
 */
void OLED_ShowString(uint8_t x, uint8_t y, char *str, uint8_t size)
{
    uint8_t char_width = (size == 8) ? 6 : 8;
    uint8_t char_height = (size == 8) ? 1 : 2;
    
    while (*str)
    {
        OLED_ShowChar(x, y, *str, size);
        
        x += char_width;
        
        if (x > 128 - char_width)
        {
            x = 0;
            y += char_height;
        }
        
        str++;
    }
}



/**
 * @brief 在 OLED 屏幕上显示整数（支持不同字号）
 * @param x 数字的起始列坐标（0-127）
 * @param y 数字的起始页坐标（0-7）
 * @param num 要显示的整数（0-4294967295）
 * @param len 数字的位数（1-10）
 * @param size 字体大小（8 或 16）
 * @note 实现原理：
 *       1. 根据字体大小计算字符宽度
 *       2. 循环 len 次，每次处理一位数字
 *       3. 计算当前位的数字值：num / 10^(len-i-1) % 10
 *       4. 将数字值转换为字符：+ '0'
 *       5. 调用 OLED_ShowChar 函数显示该字符
 * @note size = 8: 每个数字占 6 列，1 页高度
 * @note size = 16: 每个数字占 8 列，2 页高度
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    uint8_t char_width = (size == 8) ? 6 : 8;
    
    for (uint8_t i = 0; i < len; i++)
    {
        OLED_ShowChar(
            x + i * char_width,
            y,
            num / OLED_Pow(10, len - i - 1) % 10 + '0',
            size
        );
    }
}

/**
 * @brief 在 OLED 屏幕上显示有符号整数（支持不同字号）
 * @param x 数字的起始列坐标（0-127）
 * @param y 数字的起始页坐标（0-7）
 * @param num 要显示的有符号整数
 * @param len 数字的位数（1-10）
 * @param size 字体大小（8 或 16）
 * @note 实现原理：
 *       1. 根据字体大小计算字符宽度
 *       2. 判断数字是否为负数
 *       3. 如果是负数，显示负号并取绝对值
 *       4. 调用 OLED_ShowNum 函数显示绝对值
 * @note size = 8: 每个数字占 6 列，1 页高度
 * @note size = 16: 每个数字占 8 列，2 页高度
 */
void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size)
{
    uint8_t char_width = (size == 8) ? 6 : 8;
    
    if (num < 0)
    {
        OLED_ShowChar(x, y, '-', size);
        num = -num;
        x += char_width;
        len--;
    }
    OLED_ShowNum(x, y, (uint32_t)num, len, size);
}

/**
 * @brief 在 OLED 屏幕上显示浮点数（支持不同字号）
 * @param x 数字的起始列坐标（0-127）
 * @param y 数字的起始页坐标（0-7）
 * @param num 要显示的浮点数
 * @param int_len 整数部分的位数（1-9）
 * @param dec_len 小数部分的位数（1-9）
 * @param size 字体大小（8 或 16）
 * @note 实现原理：
 *       1. 根据字体大小计算字符宽度
 *       2. 处理整数部分，调用 OLED_ShowNum 函数显示
 *       3. 显示小数点
 *       4. 处理小数部分，通过乘以10^dec_len转换为整数后显示
 * @note size = 8: 每个数字占 6 列，1 页高度
 * @note size = 16: 每个数字占 8 列，2 页高度
 */
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t int_len, uint8_t dec_len, uint8_t size)
{
    uint8_t char_width = (size == 8) ? 6 : 8;
    uint32_t int_part = (uint32_t)num;
    uint32_t dec_part = (uint32_t)((num - int_part) * OLED_Pow(10, dec_len));
    
    // 显示整数部分
    OLED_ShowNum(x, y, int_part, int_len, size);
    
    // 显示小数点
    OLED_ShowChar(x + int_len * char_width, y, '.', size);
    
    // 显示小数部分
    OLED_ShowNum(x + (int_len + 1) * char_width, y, dec_part, dec_len, size);
}


/**
 * @brief 在 OLED 屏幕上显示中文字符
 * @param x 字符的起始列坐标（0-127）
 * @param y 字符的起始页坐标（0-7）
 * @param ch 要显示的中文字符（UTF-8 或 GB2312 编码）
 * @param size 字体大小（暂时只支持16x16）
 * @note 实现原理：
 *       1. 查找中文字符在字库中的索引
 *       2. 设置光标位置
 *       3. 写入字模数据
 */
void OLED_ShowChinese(uint8_t x, uint8_t y, const char *ch, uint8_t size)
{
    if (size != 16) return;  // 暂时只支持16x16字体
    
    // 查找中文字符
    uint8_t i = 0;
    while (OLED_CF16x16[i].Index[0] != 0)
    {
#ifdef OLED_CHARSET_UTF8
        if (OLED_CF16x16[i].Index[0] == ch[0] &&
            OLED_CF16x16[i].Index[1] == ch[1] &&
            OLED_CF16x16[i].Index[2] == ch[2])
        {
            break;
        }
#endif
#ifdef OLED_CHARSET_GB2312
        if (OLED_CF16x16[i].Index[0] == ch[0] &&
            OLED_CF16x16[i].Index[1] == ch[1])
        {
            break;
        }
#endif
        i++;
    }
    
    if (OLED_CF16x16[i].Index[0] == 0) return;  // 未找到字符
    
    // 显示中文字符（16x16）
    for (uint8_t page = 0; page < 2; page++)
    {
        ssd1306_set_cursor(y + page, x);
        for (uint8_t col = 0; col < 16; col++)
        {
            ssd1306_write_data(OLED_CF16x16[i].Data[page * 16 + col]);
        }
    }
}

/**
 * @brief 在 OLED 屏幕上显示图片
 * @param x 图片的起始列坐标（0-127）
 * @param y 图片的起始页坐标（0-7）
 * @param width 图片的宽度（1-128）
 * @param height 图片的高度（1-64）
 * @param image 图片数据指针，格式为按页存储的字节数组
 * @note 实现原理：
 *       1. 遍历图片的每个页面（y到y+height/8）
 *       2. 对于每个页面，设置光标位置到指定的页和列
 *       3. 遍历图片的每一列，将对应的数据写入 OLED
 * @note 图片数据格式：
 *       - 每个字节表示垂直方向的8个像素
 *       - 高位在上，低位在下
 *       - 数据按页面顺序存储，每个页面包含width个字节
 */
void OLED_ShowImage(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *image)
{
    // 计算需要显示的页面数
    uint8_t pages = (height + 7) / 8;  // 向上取整
    
    // 遍历每个页面
    for (uint8_t page = 0; page < pages; page++)
    {
        // 计算当前页面在OLED上的位置
        uint8_t current_page = y + page;
        if (current_page >= 8) break;  // 超出OLED范围
        
        // 设置光标位置
        ssd1306_set_cursor(current_page, x);
        
        // 遍历当前页面的每一列
        for (uint8_t col = 0; col < width; col++)
        {
            if (x + col >= 128) break;  // 超出OLED范围
            
            // 计算当前数据在数组中的索引
            uint16_t index = page * width + col;
            
            // 写入数据
            ssd1306_write_data(image[index]);
        }
    }
}

/**
 * @brief 在 OLED 屏幕上绘制一个点
 * @param x 点的列坐标（0-127）
 * @param y 点的行坐标（0-63）
 * @note 实现原理：
 *       1. 计算点所在的页面和偏移量
 *       2. 更新屏幕缓冲区
 *       3. 设置光标位置
 *       4. 写入更新后的数据
 */
void OLED_DrawPoint(uint8_t x, uint8_t y)
{
    if (x >= 128 || y >= 64) return;  // 超出范围
    
    uint8_t page = y / 8;
    uint8_t offset = y % 8;
    
    // 更新屏幕缓冲区
    oled_buffer[page][x] |= (1 << offset);
    
    // 设置光标位置
    ssd1306_set_cursor(page, x);
    
    // 写入更新后的数据
    ssd1306_write_data(oled_buffer[page][x]);
}

/**
 * @brief 在 OLED 屏幕上绘制一条直线
 * @param x1 起点的列坐标（0-127）
 * @param y1 起点的行坐标（0-63）
 * @param x2 终点的列坐标（0-127）
 * @param y2 终点的行坐标（0-63）
 * @note 实现原理：使用 Bresenham 直线算法
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    int16_t sx = (dx > 0) ? 1 : -1;
    int16_t sy = (dy > 0) ? 1 : -1;
    dx = dx > 0 ? dx : -dx;
    dy = dy > 0 ? dy : -dy;
    
    int16_t err = dx - dy;
    int16_t e2;
    
    uint8_t x = x1;
    uint8_t y = y1;
    
    while (1)
    {
        OLED_DrawPoint(x, y);
        
        if (x == x2 && y == y2) break;
        
        e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

/**
 * @brief 在 OLED 屏幕上绘制一个圆
 * @param x0 圆心的列坐标（0-127）
 * @param y0 圆心的行坐标（0-63）
 * @param radius 圆的半径
 * @note 实现原理：使用 Bresenham 圆算法
 */
void OLED_DrawCircle(uint8_t x0, uint8_t y0, uint8_t radius)
{
    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;
    
    while (x >= y)
    {
        // 绘制8个对称点
        if (x0 + x < 128)
        {
            if (y0 + y < 64) OLED_DrawPoint(x0 + x, y0 + y);
            if (y0 - y >= 0) OLED_DrawPoint(x0 + x, y0 - y);
        }
        if (x0 - x >= 0)
        {
            if (y0 + y < 64) OLED_DrawPoint(x0 - x, y0 + y);
            if (y0 - y >= 0) OLED_DrawPoint(x0 - x, y0 - y);
        }
        if (x0 + y < 128)
        {
            if (y0 + x < 64) OLED_DrawPoint(x0 + y, y0 + x);
            if (y0 - x >= 0) OLED_DrawPoint(x0 + y, y0 - x);
        }
        if (x0 - y >= 0)
        {
            if (y0 + x < 64) OLED_DrawPoint(x0 - y, y0 + x);
            if (y0 - x >= 0) OLED_DrawPoint(x0 - y, y0 - x);
        }
        
        if (err <= 0)
        {
            y++;
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x--;
            err -= 2 * x + 1;
        }
    }
}

/**
 * @brief 擦除 OLED 屏幕上的指定区域
 * @param x 区域的起始列坐标（0-127）
 * @param y 区域的起始页坐标（0-7）
 * @param width 区域的宽度（1-128）
 * @param height 区域的高度（1-8，单位：页）
 * @note 实现原理：
 *       1. 遍历指定区域的每个页面
 *       2. 对于每个页面，设置光标位置
 *       3. 向该页面的指定列范围写入 0x00（空数据）
 */
void OLED_ClearArea(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    // 遍历每个页面
    for (uint8_t page = 0; page < height; page++)
    {
        uint8_t current_page = y + page;
        if (current_page >= 8) break;  // 超出OLED范围
        
        // 设置光标位置
        ssd1306_set_cursor(current_page, x);
        
        // 写入空数据
        for (uint8_t col = 0; col < width; col++)
        {
            if (x + col >= 128) break;  // 超出OLED范围
            ssd1306_write_data(0x00);
        }
    }
}

/**
 * @brief 擦除 OLED 屏幕上的指定行
 * @param y 行的页坐标（0-7）
 * @note 实现原理：
 *       1. 设置光标到指定页的起始列
 *       2. 向该页的所有 128 列写入 0x00（空数据）
 */
void OLED_ClearLine(uint8_t y)
{
    if (y >= 8) return;  // 超出范围
    
    // 设置光标位置
    ssd1306_set_cursor(y, 0);
    
    // 写入空数据
    for (uint8_t col = 0; col < 128; col++)
    {
        ssd1306_write_data(0x00);
    }
}

/**
 * @brief 擦除 OLED 屏幕上的指定列的指定范围
 * @param x 列的坐标（0-127）
 * @param start_y 起始行坐标（0-63）
 * @param end_y 结束行坐标（0-63）
 * @note 实现原理：
 *       1. 计算起始和结束的页面
 *       2. 遍历每个页面
 *       3. 对于每个页面，设置光标位置到指定列
 *       4. 写入 0x00（空数据）
 */
void OLED_ClearColumn(uint8_t x, uint8_t start_y, uint8_t end_y)
{
    if (x >= 128) return;  // 超出范围
    if (start_y > end_y) return;  // 起始行大于结束行
    if (end_y >= 64) end_y = 63;  // 限制结束行
    
    uint8_t start_page = start_y / 8;
    uint8_t end_page = end_y / 8;
    
    // 遍历每个页面
    for (uint8_t page = start_page; page <= end_page; page++)
    {
        if (page >= 8) break;  // 超出OLED范围
        
        // 设置光标位置
        ssd1306_set_cursor(page, x);
        
        // 写入空数据
        ssd1306_write_data(0x00);
    }
}
// 显示方框符号
void OLED_ShowBox(uint8_t x, uint8_t y)
{
    // 方框符号的点阵数据
    const uint8_t box_bitmap[] = {
        0x18, 0x24, 0x24, 0x18, 0x18, 0x24, 0x24, 0x18
    };
    OLED_ShowImage(x, y, 8, 8, box_bitmap);
}

// 显示打钩符号
void OLED_ShowCheck(uint8_t x, uint8_t y)
{
    // 打钩符号的点阵数据
    const uint8_t check_bitmap[] = {
        0x00, 0x10, 0x18, 0x0C, 0x06, 0x3E, 0x1E, 0x00
    };
    OLED_ShowImage(x, y, 8, 8, check_bitmap);
}