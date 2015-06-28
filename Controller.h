#pragma pack(push, 0)
typedef struct Controller_t {
	uint8_t console_message[3];
	uint16_t button_states;
	uint8_t joy_x;
	uint8_t joy_y;
	uint8_t c_x;
	uint8_t c_y;
	uint8_t analog_l;
	uint8_t analog_r;
} Controller;
#pragma pack(pop)

#define CONTROLLER_BUTTON(C, B) (((C).button_states & (1 << (B))) != 0)
typedef enum Button_t {
	Button_a = 0, Button_b, Button_x, Button_y, Button_start,
	Button_d_left = 8, Button_d_right, Button_d_down, Button_d_up, Button_z, Button_r, Button_l
} Button;

#define CONTROLLER_START(C) (CONTROLLER_BUTTON((C), Button_start))
#define CONTROLLER_Y(C) (CONTROLLER_BUTTON((C), Button_y))
#define CONTROLLER_X(C) (CONTROLLER_BUTTON((C), Button_x))
#define CONTROLLER_B(C) (CONTROLLER_BUTTON((C), Button_b))
#define CONTROLLER_A(C) (CONTROLLER_BUTTON((C), Button_a))
#define CONTROLLER_L(C) (CONTROLLER_BUTTON((C), Button_l))
#define CONTROLLER_R(C) (CONTROLLER_BUTTON((C), Button_r))
#define CONTROLLER_Z(C) (CONTROLLER_BUTTON((C), Button_z))
#define CONTROLLER_D_UP(C) (CONTROLLER_BUTTON((C), Button_d_up))
#define CONTROLLER_D_DOWN(C) (CONTROLLER_BUTTON((C), Button_d_down))
#define CONTROLLER_D_RIGHT(C) (CONTROLLER_BUTTON((C), Button_d_right))
#define CONTROLLER_D_LEFT(C) (CONTROLLER_BUTTON((C), Button_d_left))
