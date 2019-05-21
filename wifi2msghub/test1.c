#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>

enum{
    RESET,
    SCAN,
    GPS,
    PARSE,
    SEND,
}state = SCAN;

/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;        
};

#define MAX 100000

CURL *curl_easy_init( );
char *strtok_r(char *str, const char *delim, char **saveptr);
FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);


//////////////////////////////////CREATE JSON OBJECT////////////////////////////////
void create_json_object(json_object* json, char* wifi, char* gps){
    char *saveptr = NULL;
    char* garbage = NULL;
    char* qual = NULL;
    char* siglvl = NULL;
    char* essid = NULL;
    char* lat = "30.0000"; //NULL;
    char* lon = "-120.000"; //NULL;
    char* elev = "40.000"; //NULL;
    
    //GET LATITUDE
    garbage = strtok_r(gps, ":", &saveptr);
    lat = strtok_r(NULL, ",", &saveptr);

    //GET LONGITUDE
    garbage = strtok_r(NULL, ":", &saveptr);
    lon = strtok_r(NULL, ",", &saveptr);

    //GET ELEVATION
    garbage = strtok_r(NULL, ":", &saveptr);
    elev = strtok_r(NULL, ",", &saveptr);
    


    //GET NETWORK QUALITY
    garbage = strtok_r(wifi, "=", &saveptr);
    qual = strtok_r(NULL, " ", &saveptr);
    
    //GET SIGNAL LEVEL
    garbage = strtok_r(NULL, "=", &saveptr);
    siglvl = strtok_r(NULL, "ESSID", &saveptr);
    
    //GET ESSID
    garbage = strtok_r(NULL, "\"", &saveptr);
    essid = strtok_r(NULL, "\"", &saveptr);

    //ADD WIFI AND GPS INFO TO JSON STRUCTURE
    json_object *jquality = json_object_new_string(qual);
    json_object *jlevel = json_object_new_string(siglvl);
    json_object *jessid = json_object_new_string(essid);
    json_object *jlat = json_object_new_string(lat); //json_object_new_double(atoi(lat);
    json_object *jlon = json_object_new_string(lon); //json_object_new_double(atoi(lon));
    json_object *jelev = json_object_new_string(elev); //json_object_new_double(atoi(elev));

    json_object_object_add(json, "\"quality\"", jquality);
    json_object_object_add(json, "\"rssi\"", jlevel);
    json_object_object_add(json, "\"essid\"", jessid);

    json_object_object_add(json, "\"latitude\"", jlat);
    json_object_object_add(json, "\"longitude\"", jlon);
    json_object_object_add(json, "\"elevation\"", jelev);
}

/////////////////////////////SAVES CURL RETURNED INFO INTO BUFFER//////////////////////////////
size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp) {

    size_t realsize = size * nmemb;                             /* calculate buffer size */
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp;   /* cast pointer to fetch struct */

    /* expand buffer */
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);

    /* check buffer */
    if (p->payload == NULL) {
        /* this isn't good */
        fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
        /* free buffer */
        free(p->payload);
        /* return */
        return -1;
                                          
    }

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
                                
}

////////////////////////MAIN CONTROL LOOP FOR WIFI SIGNAL SCAN SAMPLE/////////////////////
int main(int argc, char* argv[]){
    char buffer[MAX] = {'\0'};
    struct curl_fetch_st curl_fetch; 
    struct curl_fetch_st *fetch = &curl_fetch;
    
    struct curl_fetch_st gps_curl_fetch;
    struct curl_fetch_st *gps_fetch = &gps_curl_fetch;
    
    //WIFI CURL OPTS
    CURL *wificurl = curl_easy_init();
    curl_easy_setopt(wificurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(wificurl, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(wificurl, CURLOPT_WRITEDATA, (void *) fetch);
    curl_easy_setopt(wificurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(wificurl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(wificurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(wificurl, CURLOPT_MAXREDIRS, 1);
    
    //GPS CURL OPTS
    CURL *gpscurl = curl_easy_init();
    curl_easy_setopt(gpscurl, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(gpscurl, CURLOPT_WRITEDATA, (void *) gps_fetch);
    curl_easy_setopt(gpscurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(gpscurl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(gpscurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(gpscurl, CURLOPT_MAXREDIRS, 1);
    
    //BUFFERS AND WIFI ERROR CHECKING
    char* wifiERROR = "-en {}\n";
    char* wifi = NULL;
    char* gps = NULL;
    char kafkaCommand[1000]; //{"echo \"$json\" | kafkacat -P -b
    char MSGHUB_PASSWORD[1000] = {"b5s6FSlt4I3BKF56yYEGbhflX1g8mom"};
    char MSGHUB_USERNAME[1000] = {"LcFjdtwZ9rGklOR2z"};
    char MSGHUB_BROKER_URL[1000] = {"kafka02-prod02.messagehub.services.us-south.bluemix.net:9093,kafka01-prod02.messagehub.services.us-south.bluemix.net:9093,kafka04-prod02.messagehub.services.us-south.bluemix.net:9093,kafka05-prod02.messagehub.services.us-south.bluemix.net:9093,kafka03-prod02.messagehub.services.us-south.bluemix.net:9093"};
    char MSGHUB_TOPIC[1000] = {"troy.fine_ibm.com.IBM_cpu2msghub"};


    //INSTANTIATE JSON OBJECT
    json_object* json = json_object_new_object(); 
    FILE* output;

    while(1){
        switch(state){
            case(RESET):
                printf("in the RESET state\n");
                // RESET BUFFERS THEN GO TO SCAN STATE
                
                sleep(5);
                state = SCAN;
                break;

            case(SCAN):
                printf("SCANning for WiFi networks\n");

                // SCAN FOR WIRELESS NETWORKS AND WRITE TO BUFFER
                ///////// CURL EXAMPLE /////////
                CURLcode wifires;

                if(wificurl) {
                    curl_easy_setopt(wificurl, CURLOPT_URL, "http://localhost:34567/v1/wifisignalscan");
                    wifires = curl_easy_perform(wificurl);
                    wifi = fetch->payload;
                    //printf("WIFI: %s\n\n", wifi);
                }
                curl_fetch.payload = NULL;
                curl_fetch.size = 0;
                
                //CHECK IF WIFI SCAN SUCCEEDED
                if(wifi == NULL){
                    printf("\nWIFI SCAN FAILED! SCAN AGAIN NEXT ITERATION.\n\n");
                    state = RESET;
                }
                else if(strncmp(wifi, wifiERROR, 6) == 0){
                    printf("\nWIFI SCAN FAILED! SCAN AGAIN NEXT ITERATION.\n\n");
                    state = RESET;
                } 
                else state = GPS;
                
                break;
            
            case(GPS):
                printf("in GSP state\n");
                
                // GET GPS LOCATION
                CURLcode gpsres;
                
                if(gpscurl) {
                    curl_easy_setopt(gpscurl, CURLOPT_URL, "http://172.17.0.2:31779/v1/gps/location");
                    gpsres = curl_easy_perform(gpscurl);
                    gps = gps_fetch->payload;
                    //printf("GPS: %s\n\n", gps);
                }

                gps_curl_fetch.payload = NULL;
                gps_curl_fetch.size = 0;
                
                if(gps == NULL){
                    printf("\nGPS SERVICE FAILURE! WILL TRY AGAIN NEXT ITERATION.\n\n");
                    state = RESET;
                }
                else state = PARSE;
                break;

            case(PARSE):
                printf("PARSE found networks\n");
                
                // CREATE JSON OBJECT
                create_json_object(json, wifi, gps);
                printf ("\nThe json object created: %s\n\n",json_object_to_json_string(json));
                
                state = SEND;
                break; 

            case(SEND): ;
                // SEND JSON OBJECT TO EVENT STREAM VIA POPEN
                sprintf(kafkaCommand, "echo \"%s\" | kafkacat -P -b %s -X api.version.request=true -X security.protocol=sasl_ssl -X sasl.mechanisms=PLAIN -X sasl.username=%s -X sasl.password=%s -t %s", json_object_to_json_string(json), MSGHUB_BROKER_URL, MSGHUB_USERNAME, MSGHUB_PASSWORD, MSGHUB_TOPIC);
                
                printf("\n\nKafka command: %s\n\n", kafkaCommand);
                output = popen(kafkaCommand, "r");
                
                state = RESET;
                break;
        }
    }

    curl_easy_cleanup(wificurl);
    curl_easy_cleanup(gpscurl);
    curl_global_cleanup();
    free(fetch->payload);
    free(gps_fetch->payload);
    json_object_put(json);
    pclose(output);

    return (0);
}
