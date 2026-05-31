#include <raylib.h>
#include <wiiuse/wpad.h>

int main(int argc, char** argv) {
    InitWindow(640, 480, "Wii Raylib"); // calls InitPlatform internally

    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);

    while (!WindowShouldClose()) {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;

        WPADData* data = WPAD_Data(0);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello Wii!", 10, 10, 20, DARKGRAY);
        if (data->data_present) {
            DrawText(TextFormat("IR: %.0f, %.0f", data->ir.x, data->ir.y), 10, 30, 20, DARKGRAY);
            DrawText(TextFormat("Gyro: %.0f, %.0f, %.0f", data->orient.pitch, data->orient.roll, data->orient.yaw), 10, 50, 20, DARKGRAY);
            DrawRectangle(data->ir.x - 10, data->ir.y - 10, 20, 20, (Color) { (unsigned char)data->orient.pitch, (unsigned char)data->orient.roll, (unsigned char)data->orient.yaw, 255 });
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}