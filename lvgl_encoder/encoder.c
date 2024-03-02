/****************************************************************************
 * apps/examples/lvgl_encoder/encoder.c
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

#include <sys/types.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <inttypes.h>
#include <errno.h>
#include <debug.h>

#include <mqueue.h>

#include <nuttx/sensors/qencoder.h>
#include "encoder.h"

#define ENCODER_TASK_NAME  "encoder_task"
#define ENCODER_TASK_STACK_SIZE 2048
#define ENCODER_TASK_PRIORITY 120
#define ENCODER_PATH "/dev/qe0"

#define ENCONDER_MAX_POSITION (100 * 2) + 1
#define ENCONDER_MIN_POSITION 0

static int encoder_task(int argc, char *argv[])
{
    int fd;
    int ret;
    int position = 0;

    mqd_t mq_enconder = mq_open(MQ_NAME, O_WRONLY | O_CREAT | O_NONBLOCK, 0666, NULL);

    if (mq_enconder == (mqd_t)-1)
    {
        printf("[ERROR] Message Queue Failed \n");
        return EXIT_FAILURE;
    }

    fd = open(ENCODER_PATH, O_RDONLY);
    if(fd < 0)
    {
        printf("[ERROR] - Failed to open %s, ret = %d\n", ENCODER_PATH, errno);
        goto error;
    }

    while(1)
    {
        if (ioctl (fd, QEIOC_POSITION, (long int)&position) < 0)
        {
            break;
        }
        else
        {
            if (position < ENCONDER_MIN_POSITION)
            {
                ret = ioctl(fd, QEIOC_RESET, 0);
                if (ret < 0)
                {
                    printf("[ERROR] ioctl QEIOC_RESET failed: %d", ret);
                }
            }

            else if (position > ENCONDER_MAX_POSITION)
            {
                /* Workaround to avoid sending values above to 100 */
                continue;
            }

            else
            {
                ret = mq_send(mq_enconder, (FAR char *)&position, sizeof(position), 0);
                if (ret < 0)
                {
                    printf("[ERROR] Failed to send \n");
                }
            }
        }

        usleep(1000);
    }

    return EXIT_SUCCESS;
    
    error:
    mq_close(mq_enconder);
    close(fd);
    return EXIT_FAILURE;

}


int init_encoder(void)
{

    pid_t enconder_pid = task_create(ENCODER_TASK_NAME,
                              ENCODER_TASK_PRIORITY,
                              ENCODER_TASK_STACK_SIZE,
                              encoder_task,
                              NULL);
    if (enconder_pid < 0)
    {
        return -EXIT_FAILURE;
    }

    return enconder_pid;
}