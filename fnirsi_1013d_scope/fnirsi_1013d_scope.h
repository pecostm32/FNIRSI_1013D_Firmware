//----------------------------------------------------------------------------------------------------------------------------------

#ifndef FNIRSI_1013D_SCOPE_H
#define FNIRSI_1013D_SCOPE_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   480

#define SCREEN_SIZE     (SCREEN_WIDTH * SCREEN_HEIGHT)

//----------------------------------------------------------------------------------------------------------------------------------

#define CHANNEL1_COLOR         0x00FFFF00
#define CHANNEL2_COLOR         0x0000FFFF
#define TRIGGER_COLOR          0x0000FF00

#define CHANNEL1_TRIG_COLOR    0x00CCCC00
#define CHANNEL2_TRIG_COLOR    0x0000CCCC

#define XYMODE_COLOR           0x00FF00FF

#define CURSORS_COLOR          0x0000AA11

#define ITEM_ACTIVE_COLOR      0x00EF9311

//----------------------------------------------------------------------------------------------------------------------------------

#define TOUCH_STATE_INACTIVE                 0x00
#define TOUCH_STATE_HAVE_DISPLACEMENT        0x01
#define TOUCH_STATE_X_Y_MODE                 0x02
#define TOUCH_STATE_MOVE_CHANNEL_1           0x03
#define TOUCH_STATE_MOVE_CHANNEL_2           0x04
#define TOUCH_STATE_MOVE_TRIGGER_LEVEL       0x05
#define TOUCH_STATE_MOVE_TIME_CURSOR_LEFT    0x06
#define TOUCH_STATE_MOVE_TIME_CURSOR_RIGHT   0x07
#define TOUCH_STATE_MOVE_VOLT_CURSOR_TOP     0x08
#define TOUCH_STATE_MOVE_VOLT_CURSOR_BOTTOM  0x09

#define TOUCH_STATE_MASK                     0x0F

#define TOUCH_STATE_MOVE_TRIGGER_POINT       0x80

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* FNIRSI_1013D_SCOPE_H */

