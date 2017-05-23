/**
 * @file main.c
 * @brief Dynamically update timezone based on GeoIP
 *
 * @author Alex O'Neill <me@aoneill.me>
 * @bugs No known bugs.
 */

#include "main.h"

/**
 * @brief Callback method for libcurl to save data from a request
 *
 * @param content The bytes produced by libcurl
 * @param len The number of elements read by libcurl
 * @param nmemb The size of each element
 * @param uptr The user-passed pointer, in our case a smem_t*
 *
 * @return The number of bytes handled
 */
static size_t curl_write(void *content, size_t len, size_t nmemb, void *uptr) {
  // Recover sized memory
  assert(uptr);
  smem_t *data = (smem_t *) uptr;

  // Create more space for the bytes
  size_t bytes = len * nmemb;
  if(!(data->mem = realloc(data->mem, data->len + bytes))) {
    fprintf(stderr, "Failed to allocate memory for libcurl\n");
    smem_free(data);

    return- -1;
  }

  // Copy the bytes
  memcpy(data->mem + data->len - 1, (char *) content, bytes);
  data->len += bytes;
  data->mem[data->len - 1] = '\0';

  // Give back how many bytes were copied
  return bytes;
}

/**
 * @brief Given in-memory XML, find the timezone information
 *
 * @param data The in-memory XML document
 * @param ptr A destination pointer to place the null-terminated timezone
 *
 * @return A non-zero value on success
 */
static int parse_tz(smem_t *data, char **ptr) {
  assert(data && ptr);

  // Load the document from memory
  xmlDocPtr doc;
  if (!(doc = xmlParseDoc((unsigned char *) data->mem))) {
    fprintf(stderr, "Failed to parse XML\n");
    return 0;
  }

  // Create a context for XPath
  xmlXPathContextPtr xpathCtx;
  if(!(xpathCtx = xmlXPathNewContext(doc))) {
    fprintf(stderr, "Failed to create new XPath context\n");

    xmlFreeDoc(doc);
    return 0;
  }

  // Find appropriate nodes
  xmlXPathObjectPtr xpathObj =
    xmlXPathEvalExpression((unsigned char *) XPATH_EXPR, xpathCtx);
  assert(xpathObj);

  // Get the timezone in the XML
  int ret = 1;
  xmlNodeSetPtr nodes = xpathObj->nodesetval;
  if(!nodes->nodeNr) {
    fprintf(stderr, "Failed to find timezone information\n");
    ret = 0;
  }

  // Parse the XML
  else {
    char *value = (char *) nodes->nodeTab[0]->content;
    *ptr = malloc(strlen(value));
    strcpy(*ptr, value);

    // Give some information
    printf("Timezone detected: %s\n", value);
  }

  // Cleanup
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(doc);

  return ret;
}

/**
 * @brief Communicate with DBus to set the timezone
 *
 * @see https://www.freedesktop.org/wiki/Software/systemd/timedated
 *
 * @param tz The timezone to set the system to.
 *
 * @return A non-zero value on success
 */
static int set_timezone(char *tz) {
  // Connect to the system bus
  sd_bus *bus = NULL;
  int ret;
  if((ret = sd_bus_open_system(&bus)) < 0) {
    fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-ret));
  }

  // Connection was successful
  else {
    // Change the timezone
    sd_bus_error error = SD_BUS_ERROR_NULL;
    if((ret = sd_bus_call_method(bus,
            // Service, object, interface
            "org.freedesktop.timedate1",
            "/org/freedesktop/timedate1",
            "org.freedesktop.timedate1",

            // Method, error container, return (NULL)
            "SetTimezone", &error, NULL,

            // Arguments (timezone, authentication prompt)
            "sb", tz, 0)) < 0) {
      fprintf(stderr, "Failed to issue SetTimezone(...): %s\n", error.message);
    }

    else {
      // Confirm changes
      printf("Timezone updated!\n");
    }

    // Free the error
    sd_bus_error_free(&error);
  }

  // Cleanup
  sd_bus_unref(bus);
  return (ret == 0);
}

/**
 * @brief Output usage information for the user
 *
 * @param argc The number of command-line arguments
 * @param argv The command-line arguments
 *
 * @return An integer status code
 */
static int usage(int argc, char **argv) {
  printf(
    "usage: %s [timezone]\n\n"
    "Dynamically update the system timezone based on GeoIP\n\n"
    "Args:\n"
    "  timezone: Optional, pass a timezone to set manually\n"
    "    See: https://www.freedesktop.org/wiki/Software/systemd/timedated/\n",
    argv[0]);

  return 1;
}

/**
 * @brief Entry-point for the timezone updater
 *
 * @param argc The number of command-line arguments
 * @param argv The command-line arguments
 *
 * @return An integer status code
 */
int main(int argc, char **argv) {
  if(argc > 2) {
    return usage(argc, argv);
  }
  else if(argc == 2) {
    return set_timezone(argv[1]);
  }

  // Initialize libcurl
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();
  if(!curl) {
    fprintf(stderr, "Failed to initialize libcurl\n");
    return 1;
  }

  // Create a data holder
  smem_t data;
  if(!smem_init(&data, 1)) {
    fprintf(stderr, "Failed to initialize storage\n");
    return 1;
  }

  // Point curl in the proper direction
  curl_easy_setopt(curl, CURLOPT_URL, TZ_URL);

  // Construct callback
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &data);

  // Setup timeout, user-agent, enable redirects, cap redirects
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);

  // Preform the request
  CURLcode res = curl_easy_perform(curl);

  // Check errors
  if(res != CURLE_OK)
    fprintf(stderr, "Failed to contact the internet: %s\n",
            curl_easy_strerror(res));

  // Continue if valid
  else if(smem_valid(&data)) {
    // Get the timezone
    char *tz;
    if(!parse_tz(&data, &tz)) {
      fprintf(stderr, "Failed to parse timezone\n");
    }

    // Update timezone
    else {
      set_timezone(tz);
      free(tz);
    }

    // Cleanup memory
    smem_free(&data);
  }

  // Cleanup
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return !(res == CURLE_OK);
}
