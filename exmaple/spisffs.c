#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "spiffs.h"

static const char *TAG = "SPIFFS";

void spiffs_example()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    // 挂在注册
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    // 磁盘信息
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    
    // // 创建文件实例
    // ESP_LOGI(TAG, "Opening file");
    // FILE *f = fopen("/spiffs/hello.txt", "w");
    // if (f == NULL)
    // {
    //     ESP_LOGE(TAG, "Failed to open file for writing");
    //     return;
    // }
    // fprintf(f, "Hello World!\n");
    // fclose(f);
    // ESP_LOGI(TAG, "File written");

    // // 删除文件
    // ESP_LOGI(TAG, "File delete");
    // struct stat st;
    // if (stat("/spiffs/foo.txt", &st) == 0)
    // {
    //     // Delete it if it exists
    //     unlink("/spiffs/foo.txt");
    // }

    // // 重命名文件
    // ESP_LOGI(TAG, "Renaming file");
    // if (rename("/spiffs/hello.txt", "/spiffs/foo.txt") != 0)
    // {
    //     ESP_LOGE(TAG, "Rename failed");
    //     return;
    // }

    // //读取文件
    // ESP_LOGI(TAG, "Reading file");
    // f = fopen("/spiffs/foo.txt", "r");
    // if (f == NULL) {
    //     ESP_LOGE(TAG, "Failed to open file for reading");
    //     return;
    // }
    // char line[64];
    // fgets(line, sizeof(line), f);
    // fclose(f);

    // //获取一句话
    // char* pos = strchr(line, '\n');
    // if (pos) {
    //     *pos = '\0';
    // }
    // ESP_LOGI(TAG, "Read from file: '%s'", line);

    //卸载磁盘
    // esp_vfs_spiffs_unregister(NULL);
    // ESP_LOGI(TAG, "SPIFFS unmounted");
}
