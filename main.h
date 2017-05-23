/**
 * @file main.h
 * @brief Includes and definitions for dynamic timezone updating
 *
 * @author Alex O'Neill <me@aoneill.me>
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <systemd/sd-bus.h>

#include <smem.h>

/** @brief The URL to connect to to give the timezone information */
#define TZ_URL "http://geoip.ubuntu.com/lookup"

/** @brief The XPath expression describing the location of timezone info */
#define XPATH_EXPR "//Response/TimeZone/text()"

// Methods
int main(int argc, char **argv);

#endif /* __MAIN_H__ */
