#include <genesis.h>
#include "serial.h"
#include "resources.h"

int cursor_x = 0;
int cursor_y = 5;
u8 last_state = 0xFF;
bool connected = FALSE;
bool awaiting_ack = FALSE;
char latest_body[64];
int retry_timer = 0;
const int max_retries = 3;

void draw_status(const char* msg) {
    VDP_setTextPalette(PAL0);
    VDP_drawText(msg, 2, 1);
}

void Reset_XPort() {
    draw_status(" Sending Reset Command  ");
    Serial_Write_Msg("CMD:RESET\n");
    for (int i = 0; i < 120; i++) SYS_doVBlankProcess(); // Wait ~2 seconds
}

bool open_connection() {
    char buffer[128];
    sprintf(buffer, "Cmegaserve.kurnath.com:4000\n");
    Serial_Write_Msg(buffer);

    int wait = 120;
    while (wait-- > 0) {
        if (Data_Available() && Serial_Read() == 'C') {
            draw_status(" Connected             ");
            connected = TRUE;
            return TRUE;
        }
        SYS_doVBlankProcess();
    }

    draw_status(" Connect Timeout        ");
    connected = FALSE;
    return FALSE;
}

void close_connection() {
    Serial_Write_Msg("QU\n");
    int wait = 30;
    while (wait-- > 0) {
        if (Data_Available() && Serial_Read() == '>') break;
        SYS_doVBlankProcess();
    }
    connected = FALSE;
    draw_status(" Disconnected           ");
}

void send_latest_state() {
    if (!connected) {
        if (!open_connection()) return;
    }

    Serial_Write_Msg(latest_body);
    awaiting_ack = TRUE;
    retry_timer = 0;
}

void send_controller_state(u8 state) {
    sprintf(latest_body, "PAD: %02X\n", state);
    PAL_setColor(47, RGB24_TO_VDPCOLOR(0xffffff));
    VDP_setTextPalette(PAL2);
    VDP_drawText(latest_body, 0, cursor_y++);
    if (cursor_y >= 28) {
        VDP_clearTextAreaBG(BG_B, 0, 4, 40, 28);
        cursor_y = 5;
    }
    if (!awaiting_ack) {
        send_latest_state();
    }
}

int main() {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_setTextPlane(BG_A);

    SPR_init();
    Init_Serial();
    JOY_init();

    VDP_drawImageEx(BG_B, &headerbar, TILE_ATTR_FULL(PAL1, 1, 0, 0, TILE_USER_INDEX), 0, 0, TRUE, TRUE);
    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x000000));
    PAL_setColor(15, RGB24_TO_VDPCOLOR(0x00ff00));
    PAL_setColor(63, RGB24_TO_VDPCOLOR(0xff0000));
    VDP_setTextPalette(PAL3);
    VDP_drawText("--==:Controller Monitor:==--", 5, 4);

    Reset_XPort();
    open_connection();

    while (1) {
        u8 current_state = JOY_readJoypad(JOY_1);
        if (current_state != last_state) {
            send_controller_state(current_state);
            last_state = current_state;
        }

        if (awaiting_ack) {
            if (Data_Available()) {
                char ch = Serial_Read();
                if (ch == 'O') {
                    awaiting_ack = FALSE;
                    draw_status(" Message ACK'd         ");
                }
            } else {
                retry_timer++;
                if (retry_timer > max_retries * 60) {
                    draw_status(" Retry failed          ");
                    awaiting_ack = FALSE;
                    close_connection();
                    open_connection();
                    send_latest_state();
                }
            }
        }

        SYS_doVBlankProcess();
    }

    return 0;
}
