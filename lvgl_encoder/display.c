/****************************************************************************
 * apps/examples/lvgl_encoder/display.c
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
#include <mqueue.h>
#include <fcntl.h>

#include <lvgl/lvgl.h>
#include <port/lv_port.h>

#include "display.h"
#include "encoder.h"

#define DISPLAY_TASK_NAME  "display_task"
#define DISPLAY_TASK_STACK_SIZE 4096
#define DISPLAY_TASK_PRIORITY 120

#define ARC_WIDTH   150
#define ARC_HEIGTH  150
#define ARC_OFF_SET_X 0
#define ARC_OFF_SET_Y 20

#define MAIN_TITLE_OFF_SET_X 0
#define MAIN_TITLE_OFF_SET_Y 5
#define MAIN_TITLE "NuttX Demo - LVGL Project"

#define LCDDEV_PATH "/dev/lcd0"
#define DISPLAY_RES_H 320
#define DISPLAY_RES_V 240

static pid_t display_pid;

uint32_t main_arc_value = 0;

lv_obj_t *main_arc;
lv_obj_t *main_arc_label;

void main_screen_cb(lv_event_t *event)
{
    /* Arc events */
    lv_event_code_t event_code = lv_event_get_code(event);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        lv_arc_set_value(main_arc, (uint16_t)main_arc_value);
        lv_label_set_text_fmt(main_arc_label, "%d%%", (uint16_t)main_arc_value);
        printf("Arc value = %d\n", lv_arc_get_value(main_arc));
    }
}

void create_main_screen(lv_obj_t *parent)
{
    lv_obj_t *main_title = lv_label_create(parent);
    lv_label_set_text(main_title, MAIN_TITLE);
    lv_obj_set_align(main_title, LV_ALIGN_TOP_MID);
    lv_obj_align(main_title, LV_ALIGN_TOP_MID, MAIN_TITLE_OFF_SET_X, MAIN_TITLE_OFF_SET_Y);

    main_arc = lv_arc_create(parent);
    lv_obj_set_size(main_arc, ARC_WIDTH, ARC_HEIGTH);
    lv_arc_set_value(main_arc, 0);
    lv_obj_align(main_arc, LV_ALIGN_CENTER, ARC_OFF_SET_X, ARC_OFF_SET_Y);

    main_arc_label = lv_label_create(main_arc);
    lv_label_set_text_fmt(main_arc_label, "%d %%", main_arc_value);
    lv_obj_set_align(main_arc_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(main_arc, main_screen_cb, LV_EVENT_ALL, NULL);

    lv_event_send(main_arc, LV_EVENT_VALUE_CHANGED, NULL);
}

static int display_task(int argc, char *argv[])
{
    int pos = 0, last_pos = -1;
    int ret;

    mqd_t mq_display = mq_open(MQ_NAME, O_RDONLY | O_CREAT, 0666, NULL);

    if (mq_display == (mqd_t)-1)
    {
        printf("[ERROR] Message Queue Failed \n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        ret = mq_receive(mq_display, (char *)&pos, sizeof(pos), 0);
        if(ret > 0)
        {
            if (pos != last_pos)
            {
                main_arc_value = pos/2;
                // printf("Pos = %d\n", main_arc_value);

                if (lv_event_send(main_arc, LV_EVENT_VALUE_CHANGED, NULL) != LV_RES_OK) 
                {
                    printf("[ERROR] Failed to sending the event\n");
                }

                last_pos = pos;
            }
        }

        usleep(1000);
    }

    return EXIT_SUCCESS;
}



int init_display(void)
{

    #ifdef NEED_BOARDINIT
    /* Perform board-specific driver initialization */

    boardctl(BOARDIOC_INIT, 0);

    #ifdef CONFIG_BOARDCTL_FINALINIT
    /* Perform architecture-specific final-initialization (if configured) */

    boardctl(BOARDIOC_FINALINIT, 0);
    #endif
    #endif

    display_pid = task_create(DISPLAY_TASK_NAME,
                              DISPLAY_TASK_PRIORITY,
                              DISPLAY_TASK_STACK_SIZE,
                              display_task,
                              NULL);
    if (display_pid < 0)
    {
        return -1;
    }

    /* LVGL initialization */
    lv_init();

    /* LVGL port initialization */
    lv_port_init();

    create_main_screen(lv_scr_act());

    while (1)
    {
        lv_timer_handler();
        usleep(10000);
    }
    return 0;
}