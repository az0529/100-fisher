#include "ssd1306.h"

#define OLED_ADDR 0x3C

/**
 * @brief 向 OLED 写入命令
 * @param cmd 要写入的命令字节
 * @note 命令模式下，第一个字节为 0x00 表示后续为命令数据
 * @note 通过 I2C 总线向 OLED 设备（地址 0x3C）发送命令
 */
void ssd1306_write_cmd(uint8_t cmd)
{
    uint8_t buf[2]={0x00,cmd};

    i2c_bus_write(OLED_ADDR,buf,2);
}

/**
 * @brief 向 OLED 写入数据
 * @param data 要写入的数据字节
 * @note 数据模式下，第一个字节为 0x40 表示后续为图像数据
 * @note 通过 I2C 总线向 OLED 设备（地址 0x3C）发送数据
 */
void ssd1306_write_data(uint8_t data)
{
    uint8_t buf[2]={0x40,data};

    i2c_bus_write(OLED_ADDR,buf,2);
}

/**
 * @brief 设置 OLED 光标位置
 * @param page 页地址（0-7）
 * @param column 列地址（0-127）
 * @note 实现原理：
 *       1. 发送页地址命令：0xB0 + page（设置当前页）
 *       2. 发送列地址低 4 位：column & 0x0F
 *       3. 发送列地址高 4 位：0x10 | (column >> 4)
 * @note 此函数用于指定后续数据写入的起始位置
 */
void ssd1306_set_cursor(uint8_t page,uint8_t column)
{
    ssd1306_write_cmd(0xB0+page);
    ssd1306_write_cmd(column&0x0F);
    ssd1306_write_cmd(0x10|(column>>4));
}


/**
 * @brief 初始化 OLED 屏幕
 * @note 此函数通过发送一系列命令初始化 SSD1306 OLED 控制器
 * @note 初始化流程说明：
 *       1. 关闭显示（0xAE）
 *       2. 设置时钟分频和振荡器频率（0xD5, 0x80）
 *       3. 设置多路复用率为 64（0xA8, 0x3F）
 *       4. 设置显示偏移为 0（0xD3, 0x00）
 *       5. 设置显示起始行为 0（0x40）
 *       6. 开启电荷泵（0x8D, 0x14）
 *       7. 设置内存寻址模式为页寻址（0x20, 0x02）
 *       8. 设置 SEG 重映射为反转（0xA1）
 *       9. 设置 COM 扫描方向为反转（0xC8）
 *       10. 设置 COM 引脚硬件配置（0xDA, 0x12）
 *       11. 设置对比度（0x81, 0xCF）
 *       12. 设置预充电周期（0xD9, 0xF1）
 *       13. 设置 VCOMH 电压倍率（0xDB, 0x30）
 *       14. 开启正常显示模式（0xA4）
 *       15. 设置显示模式为正常显示（0xA6）
 *       16. 开启显示（0xAF）
 */
void ssd1306_init(void)
{
    ssd1306_write_cmd(0xAE); // display off

    ssd1306_write_cmd(0xD5); // clock divide
    ssd1306_write_cmd(0x80);

    ssd1306_write_cmd(0xA8); // multiplex
    ssd1306_write_cmd(0x3F);

    ssd1306_write_cmd(0xD3); // display offset
    ssd1306_write_cmd(0x00);

    ssd1306_write_cmd(0x40); // start line

    ssd1306_write_cmd(0x8D); // charge pump
    ssd1306_write_cmd(0x14);

    ssd1306_write_cmd(0x20); // memory mode
    ssd1306_write_cmd(0x02); // page mode

    ssd1306_write_cmd(0xA1); // segment remap
    ssd1306_write_cmd(0xC8); // COM scan

    ssd1306_write_cmd(0xDA); // com pins
    ssd1306_write_cmd(0x12);

    ssd1306_write_cmd(0x81); // contrast
    ssd1306_write_cmd(0xCF);

    ssd1306_write_cmd(0xD9); // precharge
    ssd1306_write_cmd(0xF1);

    ssd1306_write_cmd(0xDB); // vcomh
    ssd1306_write_cmd(0x30);

    ssd1306_write_cmd(0xA4); // resume
    ssd1306_write_cmd(0xA6); // normal display

    ssd1306_write_cmd(0xAF); // display on
}