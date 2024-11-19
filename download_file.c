/*
 * MIT License
 *
 * Copyright (c) 2024 SplinterGU
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Credits:
 * - Developed by SplinterGU
 * - GitHub: https://github.com/SplinterGU/ESPeccyFlashTool
 * - This project is a tool for flashing firmware to ESP32 devices.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

// Callback para recibir los datos JSON de la API de GitHub y almacenarlos en memoria
size_t write_json(void *ptr, size_t size, size_t nmemb, void *data) {
    size_t real_size = size * nmemb;
    char **response = (char **)data;
    int curr_size = strlen(*response);

    *response = realloc(*response, curr_size + real_size + 1);
    if (*response == NULL) {
        printf("not enough memory!\n");
        return 0;
    }

    strncat(*response, ptr, real_size);
    return real_size;
}

// Función para obtener la URL de la última release de GitHub
int fetch_latest_release_url(const char *repo, char *download_url, const char *asset_name, char *release_tag) {
    CURL *curl;
    CURLcode res;
    char *response = malloc(1);  // Reserva inicial para la respuesta
    response[0] = '\0';  // Asegura que la cadena esté vacía
    char api_url[512];

    // Construir la URL de la API para obtener la última release
    snprintf(api_url, sizeof(api_url), "https://api.github.com/repos/%s/releases/latest", repo);

    // Inicializar libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "comm error!\n");
        free(response);
        return 1;
    }

    // Configuración de curl
    curl_easy_setopt(curl, CURLOPT_URL, api_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");  // Para evitar problemas con la API de GitHub

    // Deshabilitar la verificación del certificado SSL
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // No verificar el certificado del servidor
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);  // No verificar el nombre del host

    // Realizar la solicitud
    res = curl_easy_perform(curl);

    // Comprobar si la solicitud fue exitosa
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (res != CURLE_OK || http_code != 200) {
        fprintf(stderr, "error getting last release %ld (%s)\n", http_code, curl_easy_strerror(res));
        free(response);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Parsear el JSON para obtener la URL de descarga
    json_t *root;
    json_error_t error;
    root = json_loads(response, 0, &error);

    if (!root) {
        fprintf(stderr, "json parser error: %s\n", error.text);
        free(response);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Obtener el tag de la release
    json_t *tag = json_object_get(root, "tag_name");
    if (json_is_string(tag)) {
        snprintf(release_tag, 128, "%s", json_string_value(tag)); // Asume que release_tag tiene suficiente espacio
    } else {
        fprintf(stderr, "Error: tag_name not found in the release data\n");
        json_decref(root);
        free(response);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Buscar el asset que nos interesa (archivo .bin)
    json_t *assets = json_object_get(root, "assets");
    if (!json_is_array(assets)) {
        fprintf(stderr, "Error: no assets for download in this release\n");
        json_decref(root);
        free(response);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Recorrer los assets y buscar el archivo .bin
    size_t index;
    json_t *asset;
    json_array_foreach(assets, index, asset) {
        const char *name = json_string_value(json_object_get(asset, "name"));
        if (strstr(name, asset_name)) {
            const char *url = json_string_value(json_object_get(asset, "browser_download_url"));
            snprintf(download_url, 512, "%s", url);
            break;
        }
    }

    json_decref(root);
    free(response);
    curl_easy_cleanup(curl);

    return 0;
}

// Callback para escribir datos binarios en el archivo
size_t write_data(void *ptr, size_t size, size_t nmemb, void *data) {
    size_t real_size = size * nmemb;
    FILE *fp = (FILE *)data;

    // Escribir los datos binarios directamente al archivo
    size_t written = fwrite(ptr, size, nmemb, fp);

    printf(".");

    return written;
}

// Función para descargar el archivo binario
int download_file(const char *repo, const char *asset_name) {
    char url[512];
    char release_tag[128];

    // Obtener la URL de la última release
    if (fetch_latest_release_url(repo, url, asset_name, release_tag) != 0) {
        fprintf(stderr, "Can't download file\n");
        return 1;
    }

    printf("Downloading %s (%s)", asset_name, release_tag);

    CURL *curl;
    CURLcode res;
    FILE *fp;

    // Abrir el archivo de salida en modo binario
    fp = fopen(asset_name, "wb");
    if (!fp) {
        perror(" error writting file!\n");
        return 1;
    }

    // Inicializar libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, " download error!\n");
        fclose(fp);
        return 1;
    }

    // Configuración de curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // Seguir redirecciones si es necesario
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // Deshabilitar la verificación del certificado SSL
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // No verificar el certificado del servidor
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);  // No verificar el nombre del host

    // Realizar la descarga
    res = curl_easy_perform(curl);

    // Comprobar si la descarga fue exitosa
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (res != CURLE_OK || http_code != 200) {
        fprintf(stderr, " download error %ld (%s)\n", http_code, curl_easy_strerror(res));
        fclose(fp);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Cerrar el archivo y limpiar
    printf(" done!\n");
    fclose(fp);
    curl_easy_cleanup(curl);

    return 0;
}
