/*
 * backlight.h
 *
 *  Created on: 2021年1月7日
 *      Author: HHS007
 */

#ifndef BACKLIGHT_BACKLIGHT_H_
#define BACKLIGHT_BACKLIGHT_H_

#include "sys.h"

#define BACK_LIGHT_STA  (PAout(7))

void Backlight_Init(void);

#endif /* BACKLIGHT_BACKLIGHT_H_ */
