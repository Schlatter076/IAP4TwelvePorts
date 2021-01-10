/*
 * base64.h
 *
 *  Created on: 2021年1月5日
 *      Author: HHS007
 */

#ifndef UTIL_BASE64_H_
#define UTIL_BASE64_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int base64_encode(const unsigned char *sourcedata, char *base64);
int num_strchr(const char *str, char c);
int base64_decode(const char *base64, unsigned char *dedata);

#endif /* UTIL_BASE64_H_ */
