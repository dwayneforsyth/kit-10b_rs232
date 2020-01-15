
void next_pattern(void);
void back_pattern(void);
extern uint8_t p_table;
extern uint8_t demo_mode;
extern bool run;
extern uint8_t user_msg_size;
extern uint8_t user_id;
extern uint8_t plockout[4];

void debugOut(char* debugString);
void debugPackedOut(uint16_t address, uint8_t size);
void doMenu(void);
void initPatternZero(void);

//#define DEBUG