/*
 *---------------------------------------------------------------
 *                        Lvgl Font Tool
 *
 * 注:使用unicode编码
 * 注:本字体文件由Lvgl Font Tool V0.4 生成
 * 作者:阿里(qq:617622104)
 *---------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_spiffs.h"

#define TAG "MY_FONT"
typedef struct
{
    uint16_t min;
    uint16_t max;
    uint8_t bpp;
    uint8_t reserved[3];
} x_header_t;
typedef struct
{
    uint32_t pos;
} x_table_t;
typedef struct
{
    uint8_t adv_w;
    uint8_t box_w;
    uint8_t box_h;
    int8_t ofs_x;
    int8_t ofs_y;
    uint8_t r;
} glyph_dsc_t;

static x_header_t __g_xbf_hd = {
    .min = 0x000a,
    .max = 0x9fa0,
    .bpp = 1,
};

static uint8_t __g_font_buf[87]; // 如bin文件存在SPI FLASH可使用此buff
static uint8_t flag = 0;
FILE *f;
static uint8_t *user_font_getdata(int offset, int size)
{
    // 建议单独分配一个磁盘，加快读写，不卸载
    if (flag == 0)
    {
        flag = 1;
        esp_vfs_spiffs_conf_t conf = {
            .base_path = "/spiffs",
            .partition_label = NULL,
            .max_files = 5,
            .format_if_mount_failed = false};

        // 挂载注册
        esp_err_t ret = esp_vfs_spiffs_register(&conf);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "磁盘挂载失败");
            return NULL;
        }
        // 读取
        f = fopen("/spiffs/font/myFont.bin", "r");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for reading");
            return NULL;
        }
    }

    fseek(f, offset, SEEK_SET);
    fread(__g_font_buf, 1, size, f);
    // fclose(f);

    // 卸载磁盘
    // esp_vfs_spiffs_unregister(NULL);
    // // ESP_LOGI(TAG, "SPIFFS unmounted");
    return __g_font_buf;
}

static uint32_t user_font_get_glyph_dsc(uint32_t unicode_letter, glyph_dsc_t *gdsc)
{
    if (unicode_letter > __g_xbf_hd.max || unicode_letter < __g_xbf_hd.min)
    {
        return 0;
    }
    // 根据字符unicode码查找索引数据
    uint32_t unicode_offset = sizeof(x_header_t) + (unicode_letter - __g_xbf_hd.min) * 4;
    uint32_t *p_pos = (uint32_t *)user_font_getdata(unicode_offset, 4);
    if (p_pos[0] != 0)
    {
        uint32_t pos = p_pos[0];
        glyph_dsc_t *mdsc = (glyph_dsc_t *)user_font_getdata(pos, sizeof(glyph_dsc_t));
        memcpy(gdsc, mdsc, sizeof(glyph_dsc_t));

        return pos;
    }
    return 0;
}

static const uint8_t *user_font_get_bitmap(uint32_t pos, int size)
{
    return user_font_getdata(pos + sizeof(glyph_dsc_t), size);
}

// 获取map数组
int lvgl_get_bitmap(uint32_t letter, uint8_t *bitmap_buf, uint8_t *box_w, uint8_t *box_h, int8_t *offset_x, int8_t *offset_y)
{
    glyph_dsc_t gdsc;
    uint32_t pos = user_font_get_glyph_dsc(letter, &gdsc);
    if (pos != 0)
    {
        int size = gdsc.box_w * gdsc.box_h * __g_xbf_hd.bpp / 8;
        *box_w = gdsc.box_w;
        *box_h = gdsc.box_h;
        *offset_x = gdsc.ofs_x;
        *offset_y = gdsc.ofs_y;
        user_font_get_bitmap(pos, size);
        memcpy(bitmap_buf, __g_font_buf, size);

        return gdsc.adv_w;
    }
    return 0;
}
