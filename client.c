#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

// Replace with your own API endpoint.
#define API_ENDPOINT "http://192.168.8.102/api.php"

struct hw_info {
    char name[255];
    char info[8192];
};

void send_hardware_info(struct hw_info *hw)
{
    CURL *ch;
    char *esc;
    size_t len;
    CURLcode res;
    char *post_buffer = NULL;
    size_t capacity = 1024UL * 1024UL * 16UL;
    curl_global_init(CURL_GLOBAL_ALL);

    ch = curl_easy_init();
    if (!ch) {
        printf("Curl error!\n");
        return;
    }

    post_buffer = malloc(capacity);
    if (!post_buffer)
        goto out;

    esc = curl_easy_escape(ch, hw->name, strlen(hw->name));
    if (!esc)
        goto out;
    len = (size_t) snprintf(post_buffer, capacity, "hw_name=%s&hw_info=", esc);
    curl_free(esc);

    esc = curl_easy_escape(ch, hw->info, strlen(hw->info));
    if (!esc)
        goto out;
    snprintf(post_buffer + len, capacity - len, "%s", esc);
    curl_free(esc);

    curl_easy_setopt(ch, CURLOPT_URL, API_ENDPOINT);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, post_buffer);
    res = curl_easy_perform(ch);

    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
out:
    if (ch) {
        curl_easy_cleanup(ch);
    }
    curl_global_cleanup();
    free(post_buffer);
    putchar('\n');
}

void get_hardware_info(struct hw_info *hw)
{
    char *bp_hwinfo = hw->info;
    char *bp_hwinfo_end = &hw->info[sizeof(hw->info)] - 1;
    FILE *handle;
    size_t ret;

    handle = popen("whoami", "r");
    if (!handle)
        return;
    ret = fread(hw->name, sizeof(char), sizeof(hw->name), handle);
    hw->name[sizeof(hw->name) - 1] = '\0';
    hw->name[ret - 1] = '\0';
    pclose(handle);

    bp_hwinfo += (size_t) snprintf(bp_hwinfo, sizeof(hw->info),
                                   "Generated by Lolicon ----\n");

    handle = popen("lshw 2>&1", "r");
    if (!handle)
        return;
    ret = fread(bp_hwinfo, sizeof(char), bp_hwinfo_end - bp_hwinfo, handle);
    hw->info[sizeof(hw->info) - 1] = '\0';
    hw->info[ret - 1] = '\0';
    pclose(handle);
}

int main(void)
{
    struct hw_info hw;
    get_hardware_info(&hw);
    send_hardware_info(&hw);
    return 0;
}
