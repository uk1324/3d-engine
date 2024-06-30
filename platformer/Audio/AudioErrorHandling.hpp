#pragma once

bool checkAlError();
void handleAudioError();

#define AL_TRY(voidExpression) { voidExpression; if (checkAlError()) { handleAudioError(); } }