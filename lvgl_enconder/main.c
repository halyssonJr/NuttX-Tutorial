/****************************************************************************
 * apps/examples/lvgl_enconder/main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/boardctl.h>
#include <sys/param.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <lvgl/lvgl.h>
#include <port/lv_port.h>

#include "display.h"
#include "enconder.h"

#define DISPLAY_TASK_NAME  "display"
#define DISPLAY_TASK_STACK_SIZE 4096
#define DISPLAY_TASK_PRIORITY 120

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{ 

  if (init_display() < 0)
  {
    return ERROR;
  }

  init_enconder();

  create_main_screen(lv_scr_act());

  while (1)
  {
    uint32_t idle;
    idle = lv_timer_handler();
    /* Minimum sleep of 1ms */
    idle = idle ? idle : 1;
    usleep(idle * 1000);
  }

  return 0;
}
