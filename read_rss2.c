#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <libxml/xmlreader.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void parseXML(const char* filename) {
    xmlTextReaderPtr reader;
    int ret;

    reader = xmlReaderForFile(filename, NULL, 0);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT) {
                // Check if the element is "title"
                if (xmlStrEqual(xmlTextReaderConstName(reader), (const xmlChar*)"title")) {
                    // Get the value of the "title" element
                    xmlChar *value = xmlTextReaderReadString(reader);
                    printf("%s\n",(const char*)value);
                    xmlFree(value);
                }
		}
            // Check if the current node is an element
            if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT) {
                // Check if the element is "description"
                if (xmlStrEqual(xmlTextReaderConstName(reader), (const xmlChar*)"description")) {
                    // Get the value of the "description" element
                    xmlChar *value = xmlTextReaderReadString(reader);
                    printf("%s\n",(const char*)value);
                    xmlFree(value);
                }
            }
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            printf("Error parsing XML\n");
        }
    } else {
        printf("Unable to open %s\n", filename);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s RSS_URL\n", argv[0]);
        return 1;
    }

    CURL *curl;
    FILE *fp;
    CURLcode res;
    char *url = argv[1];
    char outfilename[FILENAME_MAX] = "rss.xml";

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outfilename,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    } else {
        printf("Error initializing cURL\n");
    }

    parseXML(outfilename);

    return 0;
}
