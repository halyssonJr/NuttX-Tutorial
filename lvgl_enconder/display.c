/****************************************************************************
 * apps/examples/lvgl_enconder/display.c
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
#include "enconder.h"

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


static pid_t display_pid;
uint8_t main_arc_value = 0;

void main_screen_cb(lv_event_t *event)
{
    /* Arc events */
}

void create_main_screen(lv_obj_t *parent)
{
   
    lv_obj_t *main_title = lv_label_create(parent);
    lv_label_set_text(main_title, MAIN_TITLE);
    lv_obj_set_align(main_title, LV_ALIGN_TOP_MID);
    lv_obj_align(main_title, LV_ALIGN_TOP_MID, MAIN_TITLE_OFF_SET_X, MAIN_TITLE_OFF_SET_Y);

    lv_obj_t *main_arc = lv_arc_create(parent);
    lv_obj_set_size(main_arc, ARC_WIDTH, ARC_HEIGTH);
    lv_arc_set_value(main_arc, 0);
    lv_obj_align(main_arc, LV_ALIGN_CENTER, ARC_OFF_SET_X, ARC_OFF_SET_Y);

    lv_obj_t *main_arc_lablel = lv_label_create(main_arc);
    lv_label_set_text_fmt(main_arc_lablel, "%d %%", main_arc_value);
    lv_obj_set_align(main_arc_lablel, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(main_arc, main_screen_cb, LV_EVENT_VALUE_CHANGED, main_arc_lablel);

}

static int display_task(int argc, char *argv[])
{
    uint8_t pos = 0;
    int ret;

    struct mq_attr attr = 
    {    
        .mq_maxmsg  = MQ_MAX_MSG,
        .mq_msgsize = MQ_MSG_SIZE,
        .mq_flags   = 0
    };

    mqd_t mq_display = mq_open(MQ_NAME, O_RDONLY | O_CREAT, 0666, NULL);

    if (mq_display == (mqd_t)-1)
    {
        printf("[ERROR] Message Queue Failed \n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        // TODO : Wait a queue and update label value and arc
        printf("Running ...\n");

        ret = mq_receive(mq_display, (char *)&pos, sizeof(pos), 0);
        if(ret > 0)
        {
            printf("Pos = %d\n", pos);
        }

        sleep(1);
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

    return 0;
}