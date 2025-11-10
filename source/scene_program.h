
#ifndef SCENE_PROGRAM_H
#define SCENE_PROGRAM_H
// static SwkbdState swkbd;
// static char mybuf[256];
// static char mybuf2[256];
// static SwkbdStatusData swkbdStatus;
// static SwkbdLearningData swkbdLearning;
// SwkbdButton button = SWKBD_BUTTON_NONE;
// bool didit = false;
// bool swkbdTriggered = false;
void sceneProgramInit(void);
void sceneProgramUpdate(uint32_t kDown, uint32_t kHeld);
void sceneProgramRender(void);
void sceneProgramExit(void);

#endif
