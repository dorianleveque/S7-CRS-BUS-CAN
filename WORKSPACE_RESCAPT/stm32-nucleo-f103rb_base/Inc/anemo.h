
#ifndef INC_ANEMO_H_
#define INC_ANEMO_H_

#include "main.h"


void anemo_Timer1Init(void);
int anemo_GetCount(void);
int anemo_ResetCount(void);
float anemo_GetSpeed(int prescaler);

#endif /* INC_ANEMO_H_ */
